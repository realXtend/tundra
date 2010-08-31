/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef MessageConnection_h
#define MessageConnection_h

#include <vector>
#include <map>
#include <utility>

#include "LocklessQueue.h"
#include "Lockable.h"
#include "Socket.h"
#include "IMessageHandler.h"
#include "BasicSerializedDataTypes.h"
#include "Datagram.h"
#include "FragmentedTransferManager.h"

#include "clb/Container/MaxHeap.h"
#include "clb/Time/Clock.h"
#include "clb/Time/PolledTimer.h"
/*
UDP packet format: 3 bytes if InOrder=false. 5-6 bytes if InOrder=true.
1bit   - InOrder packet.   This packet contains InOrder messages.
1bit   - Reliable packet.  This packet is expected to be Acked by the receiver.
6 bits - The six lowest bits of the PacketID.
u16      The 16 next bits of the PacketID. This gives 22 bits of the PacketID in total.
* u8       InOrder array length.
* N x u8-u16   InOrder PacketID delta counter. VLE-encoded 1.7/8 Only present if InOrder=true.
.Message.
.Message.
...
.Message.


Message format: 2 bytes if FRGSTART=FRAGMENT=false.
1bit - FRGSTART. This is the first fragment of the message (offset=0)
1bit - FRAGMENT. This message is a fragment of a bigger message -flag. If FRGSTART=true, this field is not read and is assumed to be true.
1bit - InOrder. This message may be processed by the application only after all the previous InOrder messages have been processed.
* 1bit - InOrder. This message may be processed by the application only after all the previous InOrder messages have been processed.
* 1bit - InOrder. This message may be processed by the application only after all the previous InOrder messages have been processed.
(Old: 1bit - Unused.)
(Old: 1bit - Unused.)
11 bits Content length (includes both the length of MessageID and Content Data fields)
* u8 InOrder array index.                                         Only present if InOrder=7 (111 in base 2).
u8-u32 # of Fragments in whole message  VLE-encoded 1.7/1.7/16    Only present if FRGSTART=true.
u8 Fragment Transfer ID                                           Only present if FRAGMENT=true or FRGSTART=true.
* New: u8-u32 Fragment number           VLE-encoded 1.7/1.7/16    Only present if FRAGMENT=true and FRGSTART=false.
(Old: u8-u16 Fragment number            VLE-encoded 1.7/8         Only present if FRAGMENT=true.)
.Content.

Content format:
u8-u32 MessageID			      VLE-encoded 1.7/1.7/16
N bytes Content data

*/

/*
TCP stream format:
.Message.
.Message.
...
.Message.

Message format:
u8-u32 Whole Message length (including MessageID + ContentData, but not this field itself)  VLE-encoded 1.7/1.7/16
u8-u32 MessageID			      VLE-encoded 1.7/1.7/16
N bytes Content data

*/

class MessageConnection;
class NetworkServer;
class Network;
class FragmentedSendManager;
struct FragmentedSendManager::FragmentedTransfer;

/// The entry point for the Network connection handler thread.
DWORD WINAPI NetworkWorkerMain(LPVOID lpParameter);

/// Contains 22 actual bits of data.
typedef unsigned long packet_id_t;

/// Performs modular arithmetic comparison to see if newID refers to a PacketID newer than oldID.
/// @return True if newID is newer than oldID, false otherwise.
inline bool PacketIDIsNewerThan(packet_id_t newID, packet_id_t oldID)
{
	if (newID > oldID)
		return true;
	if (oldID - newID >= (1 << 21))
		return true;
	return false;
}

/// Computes the PacketID for the packet (id + increment).
inline packet_id_t AddPacketID(packet_id_t id, int increment)
{
	return (id + increment) & ((1 << 22) - 1);
}

/// Computes the PacketID for the packet (id - increment).
inline packet_id_t SubPacketID(packet_id_t id, packet_id_t sub)
{
	if (id >= sub)
		return (packet_id_t)(id - sub);
	else
		return (packet_id_t)((1 << 22) - (sub-id));
}

/// NetworkMessage stores the serialized byte data of a single outbound network message, along
/// with fields that specify how it is treated by the network connection.
class NetworkMessage
{
private:
	/// To create a NetworkMessage, call MessageConnection::StartNewMessage();
	NetworkMessage() {}

	void operator=(const NetworkMessage &); ///< Noncopyable, N/I.
	NetworkMessage(const NetworkMessage &); ///< Noncopyable, N/I.

	friend class MessageConnection;
	friend class FragmentedSendManager;
	friend struct FragmentedSendManager::FragmentedTransfer;

	/// A running number that is assigned to each message to distinguish the order
	/// the messages were added to the queue. The network layer manages this numbering,
	/// the application can not control it. This is used to break ties on packets
	/// that have the same priority.
	unsigned long messageNumber;

	/// The number of times this message has been sent and not been acked (reliable messages only).
	unsigned long sendCount;

	/// The index of this fragment, or not used (undefined) if totalNumFragments==0.
	unsigned long fragmentIndex;

	/// If 0, this message is being sent unfragmented. Otherwise, this NetworkMessage is a fragment of the whole
	/// message and transfer points to the data structure that tracks the transfer of a fragmented message.
	FragmentedSendManager::FragmentedTransfer *transfer;

public:
	/// Stores the actual byte data of the message.
	std::vector<char> data;

	/// The send priority of this message with respect to other messages. Priority 0 is the lowest, and 
	/// priority 0xFFFFFFFE is the highest. Priority 0xFFFFFFFF is a special one that means 'don't send this message'.
	unsigned long priority;

	/// The ID of this message. IDs 0 - 5 are reserved for the protocol and may not be used.
	packet_id_t id;

	/// When sending out a message, the application can attach a content ID to the message,
	/// which will effectively replace all the older messages with the same messageID and
	/// contentID.
	unsigned long contentID;

	/// If true, this message should be delivered reliably, possibly resent to guarantee
	/// that the receiving party gets it.
	bool reliable;

	/// If true, this message should be delivered in-order with all the other in-order
	/// messages. The processing order of this message relative to non-in-ordered messages
	/// is not specified and can vary.
	bool inOrder;

	/// If this flag is set, the message will not be sent and will be deleted as soon
	/// as possible. It has been superceded by another message before it had the time
	/// to leave the outbound send queue.
	bool obsolete;

	/// Checks if this message is newer than the other message.
	bool IsNewerThan(const NetworkMessage &rhs) const { return (unsigned long)(messageNumber - rhs.messageNumber) < 0x80000000; }

	/// A message with this priority is obsolete and should not be sent through the connection.
	static const unsigned long cPriorityDontSend = 0xFFFFFFFF;
	/// The maximum priority a message can have.
	static const unsigned long cMaxPriority = 0xFFFFFFFE;

	/// Returns the total number of bytes this datagram will take up space when it is serialized to an UDP channel.
	int GetTotalDatagramPackedSize() const;

	/// Returns the number of this message. The message number identifies the admission order of messages to the outbound queue.
	unsigned long MessageNumber() const { return messageNumber; }
};

/// Stores information about an established MessageConnection.
struct ConnectionStatistics
{
	/// Remembers a ping request that was sent to the other end.
	struct PingTrack
	{
		clb::tick_t pingSentTick;  ///< Timestamp of when the PingRequest was sent.
		clb::tick_t pingReplyTick; ///< If replyReceived==true, contains the timestamp of when PingReply was received as a response.
		unsigned long pingID;      ///< ID of this ping message.
		bool replyReceived;        ///< True of PingReply has already been received for this.
	};
	/// Contains an entry for each recently performed Ping operation, sorted by age (oldest first).
	std::vector<PingTrack> ping;

	/// Remembers both in- and outbound traffic events on the socket.
	struct TrafficTrack
	{
		clb::tick_t tick;          ///< Denotes when this event occurred.
		unsigned long packetsIn;   ///< The number of datagrams in when this event occurred.
		unsigned long packetsOut;  ///< The number of datagrams out when this event occurred.
		unsigned long messagesIn;  ///< The total number of messages the received datagrams contained.
		unsigned long messagesOut; ///< The total number of messages the sent datagrams contained.
		unsigned long bytesIn;     ///< The total number of bytes the received datagrams contained.
		unsigned long bytesOut;    ///< The total number of bytes the sent datagrams contained. 
	};
	/// Contains an entry for each recent traffic event (data in/out) on the connection, sorted by age (oldest first).
	std::vector<TrafficTrack> traffic;

	/// Remembers the send/receive time of a datagram with a certain ID.
	struct DatagramIDTrack
	{
		clb::tick_t tick;
		packet_id_t packetID;
	};
	/// Contains an entry for each recently received packet, sorted by age (oldest first).
	std::vector<DatagramIDTrack> recvPacketIDs;
};

class NetworkMessagePriorityCmp
{
public:
	int operator ()(const NetworkMessage *a, const NetworkMessage *b)
	{
		assert(a && b);
		if (a->priority < b->priority) return -1;
		if (b->priority < a->priority) return 1;

		if (a->MessageNumber() < b->MessageNumber()) return 1;
		if (b->MessageNumber() < a->MessageNumber()) return -1;

		return 0;
	}
};

/// Represents the current state of the connection.
enum ConnectionState
{
	ConnectionPending, ///< Waiting for the other end to send an acknowledgement packet to form the connection.
	ConnectionOK,      ///< The connection is running ok.
	ConnectionDisconnecting, ///< We are closing the connection down. Cannot send any more messages.
	ConnectionClosed    ///< The socket is no longer open.
};

/// Returns a textual representation of a ConnectionState.
std::string ConnectionStateToString(ConnectionState state);

/// Represents a single established network connection. MessageConnection maintains its own worker thread that manages
/// connection control, the scheduling and prioritization of outbound messages, and receiving inbound messages.
class MessageConnection : public clb::RefCountable
{
private:
	/// A queue populated by the main thread to give out messages to the MessageConnection work thread to process.
	SimpleLockfreeQueue<NetworkMessage*> outboundAcceptQueue;

	/// A queue populated by the networking thread to hold all the incoming messages until the application can process them.
	SimpleLockfreeQueue<NetworkMessage*> inboundMessageQueue;

	/// A queue used by the main thread to save all incoming messages.
	SimpleLockfreeQueue<Datagram> inboundInjectedDatagramQueue;

	/// A priority queue to maintain in order all the messages that are going out the pipe.
	clb::MaxHeap<NetworkMessage*, NetworkMessagePriorityCmp> outboundQueue;

	/// Tracks all the message sends that are fragmented. [worker thread]
	Lockable<FragmentedSendManager> fragmentedSends;

	/// Tracks all the receives of fragmented messages and helps reconstruct the original messages from fragments. [worker thread]
	FragmentedReceiveManager fragmentedReceives;

	/// Maintains a byte buffer that contains partial messages. (Used only in TCP mode) [worker thread]
	std::vector<char> inboundData;

	/// The size of the above structure can be anything, but the number of actually
	/// used bytes is given by this variable. [worker thread]
	size_t inboundBytesPending;

	/// Specifies the result of a Socket read activity.
	enum SocketReadResult
	{
		SocketReadOK,        ///< All data was read from the socket and it is empty for now.
		SocketReadError,     ///< An error occurred - probably the connection is dead.
		SocketReadThrottled, ///< There was so much data to read that we need to pause and make room for sends as well.
	};

	/// Reads all the new bytes available in the socket. [used internally by worker thread]
	/// @return The number of bytes successfully read.
	SocketReadResult ReadSocket(size_t &bytesRead);

	/// Reads all available bytes from a stream socket.
	SocketReadResult ReadStreamSocket(size_t &bytesRead);

	/// Reads all available bytes from a datagram socket.
	SocketReadResult ReadDatagramSocket(size_t &bytesRead);

	/// Reads all available bytes from application-injected datagrams. Only applicable if the
	/// MessageConnection operates in UDP mode. (reliable==false)
	SocketReadResult ReadInjectedDatagrams(size_t &totalBytesRead);

	/// Parses the raw inbound byte stream into messages. [used internally by worker thread]
	void StreamExtractMessages();

	/// Parses bytes with have previously been read from the socket to actual application-level messages.
	void DatagramExtractMessages(const char *data, size_t numBytes);

	void HandleInboundMessage(packet_id_t packetID, const char *data, size_t numBytes);

	/// Allocates a new NetworkMessage struct. [both worker and main thread]
	NetworkMessage *AllocateNewMessage();
	/// Frees up a NetworkMessage struct when it is no longer needed. [both worker and main thread]
	void FreeMessage(NetworkMessage *msg);

	// Ping/RTT management operations:
	void SendPingRequestMessage();
	void HandlePingRequestMessage(const char *data, size_t numBytes);
	void HandlePingReplyMessage(const char *data, size_t numBytes);

	// Congestion control and data rate management:
	void PerformFlowControl();
	void HandleFlowControlRequestMessage(const char *data, size_t numBytes);

	// Acknowledging reliable datagrams:
	void PerformPacketAckSends();
	void SendPacketAckMessage();
	void HandlePacketAckMessage(const char *data, size_t numBytes);

	// Closing down the connection:
	void SendDisconnectMessage(bool isInternal);
	void HandleDisconnectMessage();
	void SendDisconnectAckMessage();
	void HandleDisconnectAckMessage();

	/// Checks if the connection has been silent too long and has now timed out.
	void DetectConnectionTimeOut();

	/// Refreshes Packet Loss related statistics.
	void ComputePacketLoss();
	/// Refreshes RTT and other connection related statistics.
	void ComputeStats();
	
	/// Adds a new entry for outbound data statistics.
	void AddOutboundStats(unsigned long numBytes, unsigned long numPackets, unsigned long numMessages);
	/// Adds a new entry for inbound data statistics.
	void AddInboundStats(unsigned long numBytes, unsigned long numPackets, unsigned long numMessages);

	/// Marks that we have received a datagram with the given ID.
	void AddReceivedPacketIDStats(packet_id_t packetID);
	/// @return True if we have received a packet with the given packetID already.
	bool HaveReceivedPacketID(packet_id_t packetID) const;

	/// Pulls in all new messages from the main thread to the worker thread side and admits them to the send priority queue. [worker thread only]
	void AcceptOutboundMessages();

	/// The object that receives notifications of all received data.
	IMessageHandler *inboundMessageHandler;

	/// The underlying socket on top of which this connection operates.
	Socket *socket;

	/// If injectionEnabled == true, we do not read from the socket from this thread, instead we just fetch injected datagrams.
	bool injectionEnabled;

	/// Win32 thread handle for the network worker thread.
	HANDLE workThread;

	/// If true, the whole connection should be torn down.
	volatile bool workThreadErrorOccurred;

	volatile enum
	{
		WorkThreadNotRunning,
		WorkThreadRunning,
		WorkThreadShouldQuit,
	} workThreadState;

	/// Specifies the current connection state.
	volatile ConnectionState connectionState;

	/// If true, all sends to the socket are on hold, until ResumeOutboundSends() is called.
	volatile bool bOutboundSendsPaused;

	/// The main entry point for the network worker thread.
	void NetworkWorkerThreadMain();

	friend DWORD WINAPI NetworkWorkerMain(LPVOID lpParameter);
	friend class NetworkServer;
	friend class Network;

	/// Posted when the winsock socket recv() has some inbound bytes for us, or if a send() might succeed.
	WSAEVENT eventSocketReady;
	/// Posted when the application has pushed us some messages to handle.
	WSAEVENT eventMsgsOutAvailable;

	void operator=(const MessageConnection &); ///< Noncopyable, N/I.
	MessageConnection(const MessageConnection &); ///< Noncopyable, N/I.

	/// Specifies the PacketID of the last received datagram with InOrder flag set.
	packet_id_t lastReceivedInOrderPacketID;

	/// Specifies the PacketID of the last sent datagram with InOrder flag set.
	packet_id_t lastSentInOrderPacketID;

	/// A running index to identify packet datagrams as they are send out to the stream.
	packet_id_t datagramPacketIDCounter;

	// The following are used for statistics purposes:
	float rtt; ///< The currently estimated round-trip time, in milliseconds.	
	clb::tick_t lastHeardTime; ///< The tick since last successful receive from the socket.	
	float packetLossRate; ///< The currently estimated datagram packet loss rate, [0, 1].	
	float packetLossCount; ///< The current packet loss in absolute packets/sec.
	float packetsInPerSec; ///< The average number of datagrams we are receiving/second.
	float packetsOutPerSec; ///< The average number of datagrams we are sending/second.
	float msgsInPerSec; ///< The average number of KristalliProtocol messages we are receiving/second.
	float msgsOutPerSec; ///< The average number of KristalliProtocol messages we are sending/second.
	float bytesInPerSec; ///< The average number of bytes we are receiving/second. This includes KristalliProtocol headers.
	float bytesOutPerSec; ///< The average number of bytes we are sending/second. This includes KristalliProtocol headers.

	/// Serializes several messages into a single UDP/TCP packet and sends it out to the wire.
	enum PacketSendResult
	{
		PacketSendOK,
		PacketSendSocketClosed,
		PacketSendSocketFull,
		PacketSendNoMessages,
		PacketSendThrottled   ///< Cannot send just yet, throttle timer is in effect.
	};

	/// Info struct used to track acks of reliable packets.
	struct PacketAckTrack
	{
		/// The timestamp of when a packet with this ID was last sent out.
		clb::tick_t tick;

		/// The packet ID of the packet that was sent out.
		packet_id_t packetID;

		/// The number of times this packet has been resent due to having timed out.
		int resendCount;

		/// If true, the packet with this ID was an inOrder packet.
		bool inOrder;

		/// if inOrder==true, this tells the packetID of the previous in-order packet that was transmitted.
		packet_id_t previousInOrderPacketID;

		std::vector<NetworkMessage*> messages;
	};

	PacketSendResult SendOutPacket();
	PacketSendResult StreamSendOutPacket();
	PacketSendResult DatagramSendOutPacket(PacketAckTrack *inOrderResendPacket = 0);

	/// Used to perform flow control on outbound UDP messages.
	clb::PolledTimer datagramSendThrottleTimer;

	typedef std::map<packet_id_t, PacketAckTrack> PacketAckTrackMap;
	/// Contains the messages we have sent out that we are waiting for the other party to Ack.
	PacketAckTrackMap outboundPacketAckTrack;

	void FreeOutboundPacketAckTrack(packet_id_t packetID);

	// Contains a list of all messages we've received that we need to Ack at some point.
	PacketAckTrackMap inboundPacketAckTrack;

	/// The number of UDP packets to send out per second.
	int datagramOutRatePerSecond;

	/// The number of UDP packets to receive per second. Of course the local end of the
	/// connection cannot directly control this, but it uses the FlowControlRequest
	/// packet to send it to the other party.
	int datagramInRatePerSecond;

	/// A running number attached to each outbound message (not present in network stream) to 
	/// break ties when deducing which message should come before which.
	unsigned long outboundMessageNumberCounter;

	/// A (messageID, contentID) pair.
	typedef std::pair<u32, u32> MsgContentIDPair;

	typedef std::map<MsgContentIDPair, std::pair<packet_id_t, clb::tick_t> > ContentIDReceiveTrack;
	/// Each (messageID, contentID) pair has a packetID "stamp" associated to them to track 
	/// and decimate out-of-order received obsoleted messages.
	ContentIDReceiveTrack inboundContentIDStamps;

	typedef std::map<MsgContentIDPair, NetworkMessage*> ContentIDSendTrack;

	ContentIDSendTrack outboundContentIDMessages;

	void CheckAndSaveOutboundMessageWithContentID(NetworkMessage *msg);

	void ClearOutboundMessageWithContentID(NetworkMessage *msg);

	/// Checks whether the given (messageID, contentID)-pair is already out-of-date and obsoleted
	/// by a newer packet and should not be processed.
	/// @return True if the packet should be processed (there was no superceding record), and
	///         false if the packet is old and should be discarded.
	bool CheckAndSaveContentIDStamp(u32 messageID, u32 contentID, packet_id_t packetID);

	void SplitAndQueueMessage(NetworkMessage *message, bool internalQueue, size_t maxFragmentSize);

	static const unsigned long MsgIdPingRequest = 0;
	static const unsigned long MsgIdPingReply = 1;
	static const unsigned long MsgIdFlowControlRequest = 2;
	static const unsigned long MsgIdPacketAck = 3;
	static const unsigned long MsgIdDisconnect = 0x3FFFFFFF;
	static const unsigned long MsgIdDisconnectAck = 0x3FFFFFFE;

	explicit MessageConnection(Socket *socket, ConnectionState startingState);

	void StartWorkerThread();
	void StopWorkerThread();

public:
	~MessageConnection();

	/// Returns the current connection state.
	ConnectionState GetConnectionState() const { return connectionState; }

	/// Runs a modal processing loop and produces events for all inbound received data.
	void RunModalClient();

	/// Starts a benign disconnect procedure. Transitions ConnectionState to ConnectionDisconnecting.
	void Disconnect();
	/// Forcibly closes down the connection immediately.
	void Close();

	/// Stores all the statistics about the current connection.
	Lockable<ConnectionStatistics> stats;

	/// Start building a new message with the given ID.
	NetworkMessage &StartNewMessage(unsigned long id);
	/// Finishes building the message and submits it to the outbound send queue.
	void EndAndQueueMessage(NetworkMessage &msg, bool internalQueue = false);

	/// Sends a message using a serializable structure.
	template<typename SerializableData>
	void SendStruct(const SerializableData &data, unsigned long id, bool inOrder, 
		bool reliable, unsigned long priority, unsigned long contentID = 0);

	/// Sends a message using a compiled message structure.
	template<typename SerializableMessage>
	void Send(const SerializableMessage &data, unsigned long contentID = 0);

	/// Stops all outbound sends until ResumeOutboundSends is called. Use if you need to guarantee that some messages be sent in the same datagram.
	/// Do not stop outbound sends for long periods, or the other end may time out the connection.
	void PauseOutboundSends();
	/// Resumes sending of outbound messages.
	void ResumeOutboundSends();

	/// Returns the number of messages in the outbound queue that are pending to be sent.
	size_t OutBoundMessagesPending() const { return outboundQueue.Size() + outboundAcceptQueue.Size(); }
	/// Returns the number of bytes we have received from the network that haven't been able to be reassembled into whole messages.
	size_t InboundPartialBytes() const { return inboundBytesPending; }

	/// Returns the underlying raw socket.
	Socket *GetSocket() { return socket; }

	EndPoint GetEndPoint() const;

	/// Returns true if this socket is connected. \todo Remove this in favor of GetConnectionState().
	bool Connected() const { return socket && socket->Connected(); }

	/// Sets the UDP packets/sec to receive. This will cause a flow control message to be sent
	/// to the other end to request the new rate to take effect.
	void SetDatagramInFlowRatePerSecond(int newDatagramReceiveRate, bool internalQueue = false);

	/// Returns the currently measured rate of UDP packets in/sec.
	int GetDatagramInFlowRatePerSecond() const { return datagramInRatePerSecond; }

	/// Returns the currently measured rate of UDP packets out/sec.
	int GetDatagramOutFlowRatePerSecond() const { return datagramOutRatePerSecond; }

	/// Processes all messages for this frame.
	void ProcessMessages();

	/// Adds the given datagram to the inbound queue of this connection, as if it had been received
	/// from the other end of the connection. \todo Optimize to remove the extra copy occurring here.
	/// Only applicable if the MessageConnection is operating in UDP mode. (reliable == false)
	void InjectDatagram(const Datagram &datagram);
	
	/// Enables or disables the message injection.
	void SetInjectionMode(bool injectionEnabled);

	/// Registers a new listener object for the events of this connection.
	void RegisterInboundMessageHandler(IMessageHandler *handler) { inboundMessageHandler = handler; }

	/// Returns the average number of inbound packet loss, packets/sec.
	float GetPacketLossCount() const { return packetLossCount; }

	/// Returns the percentage of inbound packets that are being lost, [0, 1].
	float GetPacketLossRate() const { return packetLossRate; }

	/// Returns the estimated RTT of the connection, in milliseconds. RTT is the time taken to communicate a message from client->host->client.
	float RoundTripTime() const { return rtt; }

	/// Returns the estimated delay time from this connection to host, in milliseconds.
	float Ping() const { return rtt / 2.f; }

	/// Returns the number of milliseconds since we last received data from the socket.
	float LastHeardTime() const { return clb::Clock::TicksToMilliseconds(clb::Clock::TicksInBetween(clb::Clock::Tick(), lastHeardTime)); }

	float PacketsInPerSec() const { return packetsInPerSec; }
	float PacketsOutPerSec() const { return packetsOutPerSec; }
	float MsgsInPerSec() const { return msgsInPerSec; }
	float MsgsOutPerSec() const { return msgsOutPerSec; }
	float BytesInPerSec() const { return bytesInPerSec; }
	float BytesOutPerSec() const { return bytesOutPerSec; }

	/// Returns a single-line message describing the connection state.
	std::string ToString() const;

	/// Dumps a long multi-line status message of this connection state to stdout.
	void DumpStatus() const;
};

template<typename SerializableData>
void MessageConnection::SendStruct(const SerializableData &data, unsigned long id, bool inOrder, 
		bool reliable, unsigned long priority, unsigned long contentID)
{
	const size_t dataSize = data.Size();

	NetworkMessage &msg = StartNewMessage(id);

	msg.data.resize(dataSize);
	if (dataSize > 0)
	{
		DataSerializer mb(&msg.data[0], dataSize);
		data.SerializeTo(mb);
		assert(mb.Size() == dataSize);
	}

	msg.id = id;
	msg.contentID = contentID;
	msg.inOrder = inOrder;
	msg.priority = priority;
	msg.reliable = reliable;

	EndAndQueueMessage(msg);
}

template<typename SerializableMessage>
void MessageConnection::Send(const SerializableMessage &data, unsigned long contentID)
{
	const size_t dataSize = data.Size();

	NetworkMessage &msg = StartNewMessage(data.MessageID());

	msg.data.resize(dataSize);
	if (dataSize > 0)
	{
		DataSerializer mb(&msg.data[0], dataSize);
		data.SerializeTo(mb);
		assert(mb.BytesFilled() == dataSize);
	}

	msg.id = data.MessageID();
	msg.contentID = contentID;
	msg.inOrder = data.inOrder;
	msg.priority = data.priority;
	msg.reliable = data.reliable;

	EndAndQueueMessage(msg);
}

#endif
