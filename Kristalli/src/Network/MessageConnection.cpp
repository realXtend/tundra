/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#include <algorithm>
#include <iostream>
#include <cassert>

//#include "clb/Core/Logging.h"
#include "clb/Time/PolledTimer.h"
#include "clb/Algorithm/Sort.h"
#include "clb/Math/Util/BitOps.h"

#include "clb/Network/Network.h"
#include "clb/Network/MessageConnection.h"
#include "clb/Network/DataSerializer.h"
#include "clb/Network/DataDeserializer.h"
#include "clb/Network/VLEPacker.h"
#include "clb/Network/FragmentedTransferManager.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

template<typename Cont>
bool MessageListContains(const Cont &cont, const NetworkMessage *msg)
{
    for(typename Cont::const_iterator iter = cont.begin(); iter != cont.end(); ++iter)
        if (*iter == msg)
            return true;

    return false;
}

template<typename Cont>
bool ContainsDuplicates(const Cont &cont)
{
    for(int i = 0; i < cont.Size(); ++i)
        for(int j = i+1; j < cont.Size(); ++j)
            if (cont.data[i] == cont.data[j])
                return true;

    return false;
}

namespace
{
	const int initialDatagramRatePerSecond = 30;

	/// The interval at which we send ping messages.
	const float pingIntervalMSecs = 5 * 1000.f;
	/// The interval at which we update the internal statistics fields.
	const float statsRefreshIntervalMSecs = 1000.f;
	/// The maximum time to wait before acking a packet. If there are enough packets to ack for a full ack message,
	/// acking will be performed earlier. (milliseconds)
	const float maxAckDelay = 33.f; // (1/30th of a second)
	/// The time counter after which an unacked reliable message will be resent. (UDP only)
	const float timeOutMilliseconds = 2000.f;//750.f;
	/// The time interval after which, if we don't get a response to a PingRequest message, the connection is declared lost.
	const float connectionLostTimeout = pingIntervalMSecs * 3;
	/// The maximum number of times sending a reliable message will be tried until the connection is declared lost.
	const int maxMessageSendCount = 50; ///< \todo This should probably be removed altogether. Not hearing from PingRequest is more effective.
	/// The maximum number of datagrams to read in from the socket at one go - after this reads will be throttled
	/// to give time for data sending as well.
	const int cMaxDatagramsToReadInOneFrame = 256;
	/// The maximum size for a TCP message.
	const u32 cMaxTCPMessageSize = 256 * 1024;

	const u32 cMaxUDPMessageFragmentSize = 470;

	const float cConnectTimeOutMSecs = 10 * 1000.f; ///< \todo Actually use this time limit.

	const float cDisconnectTimeOutMSecs = 5 * 1000.f; ///< \todo Actually use this time limit.
}

void AppendU8ToVector(std::vector<char> &data, unsigned long value)
{
	data.insert(data.end(), (const char *)&value, (const char *)&value + 1);
}

void AppendU16ToVector(std::vector<char> &data, unsigned long value)
{
	data.insert(data.end(), (const char *)&value, (const char *)&value + 2);
}

void AppendU32ToVector(std::vector<char> &data, unsigned long value)
{
	data.insert(data.end(), (const char *)&value, (const char *)&value + 4);
}

std::string ConnectionStateToString(ConnectionState state)
{
	std::string str[] = 
	{
		"ConnectionPending",
		"ConnectionOK",
		"ConnectionDisconnecting",
		"ConnectionClosed"
	};

	return str[state];
}

MessageConnection::MessageConnection(Socket *socket_, ConnectionState startingState)
:inboundMessageHandler(0), inboundBytesPending(0), socket(socket_), 
eventSocketReady(0), eventMsgsOutAvailable(0), bOutboundSendsPaused(false),
outboundAcceptQueue(16*1024), inboundMessageQueue(16*1024), 
inboundInjectedDatagramQueue(1024), 
workThreadState(WorkThreadNotRunning), rtt(0.f), datagramPacketIDCounter(0),
packetLossRate(0.f), packetLossCount(0.f), datagramOutRatePerSecond(initialDatagramRatePerSecond), 
datagramInRatePerSecond(initialDatagramRatePerSecond), packetsInPerSec(0), packetsOutPerSec(0), 
msgsInPerSec(0), msgsOutPerSec(0), bytesInPerSec(0), bytesOutPerSec(0),
lastHeardTime(0), outboundMessageNumberCounter(0), lastReceivedInOrderPacketID(0), 
lastSentInOrderPacketID(0), workThreadErrorOccurred(false),
injectionEnabled(false)
{
	/*
	if (socket->Reliable())
		connectionState = ConnectionOK; // We have an established TCP connection.
	else
		connectionState = ConnectionPending; // We are waiting for an UDP connection initialization reply.
	*/
	connectionState = startingState;
}

void MessageConnection::StartWorkerThread()
{
	if (workThreadState == WorkThreadRunning)
		return;
	if (workThreadState == WorkThreadShouldQuit)
		StopWorkerThread();

	lastHeardTime = clb::Clock::Tick();

//	LOG("Starting network worker thread.");
	eventSocketReady = WSACreateEvent();
	eventMsgsOutAvailable = WSACreateEvent();
	WSAResetEvent(eventMsgsOutAvailable);
	WSASetEvent(eventSocketReady);
	WSAEventSelect(socket->WinSocket(), eventSocketReady, FD_READ | FD_WRITE | FD_CLOSE);

	workThreadState = WorkThreadRunning;
	workThread = CreateThread(NULL, 0, NetworkWorkerMain, this, 0, NULL);
	if (workThread == NULL)
		throw std::exception("Failed to create worker thread!");
}

void MessageConnection::StopWorkerThread()
{
	if (workThread == NULL)
	{
		assert(workThreadState == WorkThreadNotRunning);
		return;
	}

	workThreadState = WorkThreadShouldQuit;
	// Not doing any message sending anymore.
	bOutboundSendsPaused = true;
	// Wake the thread up from any sleep state it might be in by alarming all bells.
	WSASetEvent(eventSocketReady); 
	WSASetEvent(eventMsgsOutAvailable);

	Sleep(10);
	assert(workThread != 0);

	int numTries = 100;
	while(numTries-- > 0)
	{
		DWORD exitCode = 0;
		BOOL result = GetExitCodeThread(workThread, &exitCode);

		if (result == 0)
		{
			LOG("Warning: Received error %d from GetExitCodeThread in StopWorkerThread!", GetLastError());
			break;
		}
		else if (exitCode != STILL_ACTIVE)
		{
			CloseHandle(workThread);
			workThread = NULL;
			break;
		}
		Sleep(50);
	}

	if (workThread != NULL)
	{
		TerminateThread(workThread, -1);
		CloseHandle(workThread);
		workThread = NULL;
		LOG("Warning: Had to forcibly terminate NetworkWorkerThread!");
	}
	workThreadState = WorkThreadNotRunning;
}

void MessageConnection::RunModalClient()
{
	while(GetConnectionState() != ConnectionClosed)
	{
		ProcessMessages();

		///\todo WSACreateEvent/WSAWaitForMultipleEvents for improved responsiveness and performance.
		Sleep(10);
	}
}

void MessageConnection::Disconnect()
{
	switch(connectionState)
	{
	case ConnectionPending:
		LOG("MessageConnection::Disconnect called when in ConnectionPending state! %s", ToString().c_str());
		break;
	case ConnectionOK:
//		LOG("MessageConnection::Disconnect. Sending Disconnect Message. %s", ToString().c_str());
		SendDisconnectMessage(false);
		connectionState = ConnectionDisconnecting;
		break;
	case ConnectionDisconnecting:
//		LOG("MessageConnection::Disconnect. Already disconnecting. %s", ToString().c_str());
		break;
	case ConnectionClosed:
//		LOG("MessageConnection::Disconnect. Already closed connection. %s", ToString().c_str());
		break;
	default:
		LOG("ERROR! MessageConnection::Disconnect called when in an unknown state! %s", ToString().c_str());
		break;
	}
}

void MessageConnection::Close()
{
	LOG("Closed connection to %s.", ToString().c_str());

	StopWorkerThread();
	if (socket)
	{
		socket->Disconnect();
		connectionState = ConnectionClosed;
		socket = 0;
	}
	workThreadErrorOccurred = false;
}

void MessageConnection::PauseOutboundSends()
{
	WSAResetEvent(eventMsgsOutAvailable);
	bOutboundSendsPaused = true;
}

void MessageConnection::ResumeOutboundSends()
{
	bOutboundSendsPaused = false;
	if (OutBoundMessagesPending())
		WSASetEvent(eventMsgsOutAvailable);
}

MessageConnection::~MessageConnection()
{
	StopWorkerThread();

	WSACloseEvent(eventSocketReady);
	WSACloseEvent(eventMsgsOutAvailable);

	eventSocketReady = 0;
	eventMsgsOutAvailable = 0;

	// The worker thread has to be properly shut down, we're accessing data that would be owned
	// by the worker thread.
	assert(workThreadState == WorkThreadNotRunning);
	assert(workThread == NULL);

	while(outboundAcceptQueue.Size() > 0)
	{
		delete *outboundAcceptQueue.Front();
		outboundAcceptQueue.PopFront();
	}

	while(inboundMessageQueue.Size() > 0)
	{
		delete *inboundMessageQueue.Front();
		inboundMessageQueue.PopFront();
	}

	for(int i = 0; i < outboundQueue.Size(); ++i)
		delete outboundQueue.data[i];
}

DWORD WINAPI NetworkWorkerMain(LPVOID lpParameter)
{
	// This only works when MessageConnection is not a polymorphic type. Remember not to add any virtual functions to the class.
	MessageConnection *owner = reinterpret_cast<MessageConnection*>(lpParameter);

	try
	{
		owner->NetworkWorkerThreadMain();
	}
	catch(const std::exception &e)
	{
		std::cout << "Fatal exception was thrown in MessageConnection worker thread! std::exception: " << e.what() << std::endl;
		owner->workThreadErrorOccurred = true;
		owner->connectionState = ConnectionClosed;
	}
	catch(...)
	{
		std::cout << "Fatal exception of unknown type was thrown in MessageConnection worker thread! " << std::endl;
		owner->workThreadErrorOccurred = true;
		owner->connectionState = ConnectionClosed;
	}
	
	return 0;
}

void MessageConnection::NetworkWorkerThreadMain()
{
	using namespace std;

//	LOG("%s work thread main start.", (socket->TransportLayer() == SocketOverTCP) ? "TCP" : "UDP");
	WSAEVENT events[2] = { eventMsgsOutAvailable, eventSocketReady };

	clb::PolledTimer pingTimer;
	clb::PolledTimer statsRefreshTimer;
	pingTimer.StartMSecs(pingIntervalMSecs);
	statsRefreshTimer.StartMSecs(statsRefreshIntervalMSecs);
	datagramSendThrottleTimer.StartTicks(clb::Clock::TicksPerSec() / datagramOutRatePerSecond);

	// The main thread should have set this for us.
	assert(workThreadState == WorkThreadRunning);

	bool outboundMessagesAvailable = true;
	bool socketReadReady = true;
	bool socketWriteReady = true;

	workThreadErrorOccurred = false;
	while(!workThreadErrorOccurred && connectionState != ConnectionClosed && socket && socket->Connected() && workThreadState == WorkThreadRunning)
	{
		// If we're sending UDP packets, throttle the sends to implement flow control.
		DWORD waitTime = (socket->TransportLayer() == SocketOverTCP) ? 16 : (DWORD)clb::Clock::TicksToMilliseconds(datagramSendThrottleTimer.TicksLeft());

		const DWORD maxWaitTime = 1000;
		waitTime = min(waitTime, maxWaitTime);

		// Wait until an event occurs either from the application end or in the socket.
		// When the application wants to send out a message, it is signaled by an event here.
		// Also, when the socket is ready for reading, writing or if it has been closed, it is signaled here.
		DWORD ret = WSAWaitForMultipleEvents(2, events, FALSE, waitTime, TRUE);
		// Reset the event that we received so that it can be received during the next iteration as well.
		if (ret != WSA_WAIT_TIMEOUT)
			WSAResetEvent(events[ret - WSA_WAIT_EVENT_0]);

		// Check which event occurred.
		switch(ret)
		{
		case WSA_WAIT_EVENT_0:
			// eventMsgsOutAvailable occurred, meaning that the client application pushed in new messages.
			// If the socket is ready for writing, these messages will be pushed out this cycle below.
			outboundMessagesAvailable = (OutBoundMessagesPending() > 0);
			break;
		case WSA_WAIT_EVENT_0+1: 
			{
				// eventSocketReady occurred, meaning the socket is ready for some operation. Read next
				// what that operation is:
				WSANETWORKEVENTS networkEvents;
				int ret = WSAEnumNetworkEvents(socket->WinSocket(), eventSocketReady, &networkEvents);
				if (ret != 0)
					LOG("Error on WSAEnumNetworkEvents!");

				if ((networkEvents.lNetworkEvents & FD_READ) != 0)
					socketReadReady = true;
				if ((networkEvents.lNetworkEvents & FD_WRITE) != 0)
					socketWriteReady = true;
				if ((networkEvents.lNetworkEvents & FD_CLOSE) != 0)
				{
					// Would like to call socket->Disconnect(); socket = 0; here instantly, but 
					// that would be a race condition. Now signal with the following
					// flag so that the main thread will call Disconnect. 
//					LOG("FD_CLOSE received on socket.");
					workThreadState = WorkThreadShouldQuit;
					connectionState = ConnectionClosed;
				}
			}
			break;
		default:
		case WSA_WAIT_TIMEOUT:
			// We timed out, so we've lost track of what we can do. Assume socket is ready for both
			// reading and writing and try to do both operations this cycle. If it doesn't work, the
			// worst is that we lost a few CPU cycles and these flags will be reset back to false.
			socketReadReady = true;
			socketWriteReady = true;
			break;
		}

		if (workThreadState == WorkThreadShouldQuit)
		{
//			LOG("Network thread quitting, WorkThreadShouldQuit received.");
			break;
		}

		// If the socket has some inbound data, read those in.
		if (socketReadReady || inboundInjectedDatagramQueue.Size() > 0)
		{
			size_t newBytesReceived = 0;
			SocketReadResult result = ReadSocket(newBytesReceived);
			switch(result)
			{
			case SocketReadOK:
				socketReadReady = false;
				break;
			case SocketReadError:
				///\todo Handle SocketReadError result from ReadSocket. Now we just warn with a log message - examine the error and check if we need to disconnect.
				break;
			case SocketReadThrottled:
				// Don't want to do anything here, just pass through and give send out a chance.
				break;
			}

			// Generate an Ack message if we've accumulated enough reliable messages to make it
			// worthwhile or if some of them are timing out.
			if (socket->TransportLayer() == SocketOverUDP)
				PerformPacketAckSends();
		}

		if (workThreadState == WorkThreadShouldQuit)
		{
//			LOG("Network thread quitting, WorkThreadShouldQuit received.");
			break;
		}

		// Send out new messages if the socket is ready and there's something to send.
		int numPacketsWritten = 0;
		bool throttled = false;
		while(!throttled && outboundMessagesAvailable && socketWriteReady && workThreadState == WorkThreadRunning)
		{
			// Pull in all the new messages that the application has generated and place them to the 
			// outbound priority queue. This stage will also prune/replace obsolete packets in the queue
			// that did not yet have the time to leave this end.
			AcceptOutboundMessages();

			// Send out a single packet.
			PacketSendResult sendResult = SendOutPacket();
			switch(sendResult)
			{
			case PacketSendOK: ++numPacketsWritten; break;
			case PacketSendSocketClosed:
			case PacketSendSocketFull: socketWriteReady = false; break;
			case PacketSendNoMessages: outboundMessagesAvailable = false; break;
			case PacketSendThrottled: throttled = true; break;
			default: break;
			}
		}
/*
		if (connectionState == ConnectionDisconnecting && outboundQueue.Size() == 0)
		{
			LOG("Exhausted all outbound messages while ConnectionDisconnecting. Quitting.");
			connectionState = ConnectionClosed;
			break;
		}
*/
		if (workThreadState == WorkThreadShouldQuit)
		{
//			LOG("Network thread quitting, WorkThreadShouldQuit received.");
			break;
		}

		// MessageConnection needs to automatically manage the sending of ping messages in an unreliable channel.
		if (connectionState == ConnectionOK && pingTimer.Test())
		{
			if (!bOutboundSendsPaused)
				SendPingRequestMessage();
			DetectConnectionTimeOut();
			pingTimer.StartMSecs(pingIntervalMSecs);
		}

		// Produce stats back to the application about the current connection state.
		if (statsRefreshTimer.Test())
		{
			ComputePacketLoss();
			ComputeStats();
//			PerformFlowControl();
			statsRefreshTimer.StartMSecs(statsRefreshIntervalMSecs);
		}
	}

//	LOG("Socket %s work thread quit.", socket->ToString().c_str());
}

void MessageConnection::DetectConnectionTimeOut()
{
	if (connectionState == ConnectionClosed)
		return;

	float lastHeardTime = LastHeardTime();
	if (lastHeardTime > connectionLostTimeout)
	{
		LOG("It's been %.2fms since last heard from other end. connectionLostTimeout=%.2fms, so closing connection.",
			lastHeardTime, connectionLostTimeout);
		connectionState = ConnectionClosed;
	}
}

void MessageConnection::PerformPacketAckSends()
{
	clb::tick_t now = clb::Clock::Tick();
	while(inboundPacketAckTrack.size() > 0)
	{
		if (clb::Clock::TimespanToMilliseconds(inboundPacketAckTrack.begin()->second.tick, now) < maxAckDelay &&
			inboundPacketAckTrack.size() < 33)
			break;

		SendPacketAckMessage();
	}
}

void MessageConnection::AcceptOutboundMessages()
{
	if (connectionState != ConnectionOK)
		return;

	while(outboundAcceptQueue.Size() > 0)
	{
		NetworkMessage **message = outboundAcceptQueue.Front();
		assert(message);

		NetworkMessage *msg = *message;
		outboundAcceptQueue.PopFront();
		CheckAndSaveOutboundMessageWithContentID(msg);
		outboundQueue.Insert(msg);
//        LOG("Added message 0x%X to outboundQueue, %d items in outboundQueue.", msg, outboundQueue.Size());
	}
    assert(!ContainsDuplicates(outboundQueue));
}

NetworkMessage *MessageConnection::AllocateNewMessage()
{
	NetworkMessage *msg = new NetworkMessage();

	return msg;
}

void MessageConnection::FreeMessage(NetworkMessage *msg)
{
	assert(msg);
	assert(msg->contentID != 0xCDCDCDCD);
	ClearOutboundMessageWithContentID(msg);

	delete msg;
}

NetworkMessage &MessageConnection::StartNewMessage(unsigned long id)
{
	NetworkMessage &msg = *AllocateNewMessage();

	msg.id = id;
	msg.reliable = false;
	msg.contentID = 0;
	msg.obsolete = false;

	// Give the new message the lowest priority by default.
	msg.priority = 0;

	// By default, the message is not fragmented. Later when admitting the message into the send queue, the need for
	// fragmentation is examined and this field will be updated if needed.
	msg.transfer = 0; 

	return msg;
}

void MessageConnection::SplitAndQueueMessage(NetworkMessage *message, bool internalQueue, size_t maxFragmentSize)
{
	using namespace std;

	assert(message);
	assert(!message->obsolete);

	// We need this many fragments to represent the whole message.
	const size_t totalNumFragments = (message->data.size() + maxFragmentSize - 1) / maxFragmentSize;
	assert(totalNumFragments > 1); // Shouldn't be calling this function if the message can well fit into one fragment.

    // If we can't fit the message, just discard it.
    if (!internalQueue && outboundAcceptQueue.SpaceLeft() < totalNumFragments)
    {
        FreeMessage(message); // We just drop the message we were trying to send.
        return;
    }

//	LOG("Splitting a message of %db into %d fragments of %db size at most.",
//		message->data.size(), totalNumFragments, maxFragmentSize);

/** \todo: Would like to do this:
	FragmentedSendManager::FragmentedTransfer *transfer;
	{
		Lock<FragmentedSendManager> sends = fragmentedSends.Acquire();
		transfer = sends->AllocateNewFragmentedTransfer();
	}
*/
	// But instead, have to resort to function-wide lock.
	Lock<FragmentedSendManager> sends = fragmentedSends.Acquire();
	FragmentedSendManager::FragmentedTransfer *transfer = sends->AllocateNewFragmentedTransfer();

	size_t currentFragmentIndex = 0;
	size_t byteOffset = 0;

	assert(transfer != 0);
	transfer->totalNumFragments = totalNumFragments;

//	if (!message->reliable)
//		LOG("Upgraded a nonreliable message with ID %d and size %d to a reliable message since it had to be fragmented!", message->id, message->data.size());

	while(byteOffset < message->data.size())
	{
//		NetworkMessage *fragment = AllocateNewMessage();
		NetworkMessage *fragment = &StartNewMessage(message->id);
		fragment->contentID = message->contentID;
//		fragment->id = message->id;
		fragment->inOrder = message->inOrder;
		fragment->reliable = true; // We don't send fragmented messages as unreliable messages - the risk of a fragment getting lost wastes bandwidth.
		fragment->messageNumber = outboundMessageNumberCounter++;
		fragment->priority = message->priority;
		fragment->sendCount = 0;

		fragment->transfer = transfer;
		fragment->fragmentIndex = currentFragmentIndex++;

		// Copy the data from the old message that's supposed to go into this fragment.
		const size_t thisFragmentSize = min(maxFragmentSize, message->data.size() - byteOffset);
		fragment->data.insert(fragment->data.end(), message->data.begin() + byteOffset, message->data.begin() + byteOffset + thisFragmentSize);
		byteOffset += thisFragmentSize;

		transfer->AddMessage(fragment);

		if (internalQueue)
        {
			outboundQueue.Insert(fragment);
//            LOG("Added message 0x%X to outboundQueue(internal), %d items in outboundQueue.", fragment, outboundQueue.Size());
            assert(!ContainsDuplicates(outboundQueue));
        }
		else
        {
			if (!outboundAcceptQueue.Insert(fragment))
            {
                ///\todo The 
                FreeMessage(fragment);
            }
        }

	}

	// Signal the worker thread that there are new outbound events available.
	if (!bOutboundSendsPaused)
		WSASetEvent(eventMsgsOutAvailable);

	// The original message that was split into fragments is no longer needed - it is represented by the newly created fragments
	// that have now been queued.
	FreeMessage(message);
}

void MessageConnection::EndAndQueueMessage(NetworkMessage &msg, bool internalQueue)
{
	// If the message was marked obsolete to start with, discard it.
	if (msg.obsolete || !socket || GetConnectionState() == ConnectionClosed)
	{
		FreeMessage(&msg);
		return;
	}

	// Check if the message is too big - in that case we split it into fixed size fragments and add them into the queue.
	///\todo We can optimize here by doing the splitting at datagram creation time to create optimally sized datagrams, but
	/// it is quite more complicated, so left for later. 
	const size_t sendHeaderUpperBound = 32; // Reserve some bytes for the packet and message headers. (an approximate upper bound)
	if (msg.data.size() + sendHeaderUpperBound > socket->MaxSendSize())
	{
		const size_t maxFragmentSize = cMaxUDPMessageFragmentSize;//socket->MaxSendSize() / 4 - sendHeaderUpperBound;
		assert(maxFragmentSize > 0 && maxFragmentSize < socket->MaxSendSize());
		SplitAndQueueMessage(&msg, internalQueue, maxFragmentSize);
		return;
	}

	msg.messageNumber = outboundMessageNumberCounter++;
	msg.sendCount = 0;

	if (internalQueue)
    {
		outboundQueue.Insert(&msg);
//        LOG(".Added message 0x%X to outboundQueue(internal), %d items in outboundQueue.", msg, outboundQueue.Size());
        assert(!ContainsDuplicates(outboundQueue));
    }
	else
    {
        // Try to insert the message - if there's no space, we must take care of freeing the message structure.
		if (!outboundAcceptQueue.Insert(&msg))
            FreeMessage(&msg);
    }

	// Signal the worker thread that there are new outbound events available.
	if (!bOutboundSendsPaused)
		WSASetEvent(eventMsgsOutAvailable);
}

MessageConnection::SocketReadResult MessageConnection::ReadStreamSocket(size_t &totalBytesRead)
{
	totalBytesRead = 0;

	const size_t maxBytesToRead = 1024 * 1024;
	const size_t cReadSize = 1024 * 512;

	// Pump the socket's receiving end until it's empty or can't process any more for now.
	while(totalBytesRead < maxBytesToRead)
	{
		if (inboundBytesPending + cReadSize >= inboundData.size())
			inboundData.resize(inboundBytesPending + cReadSize);

		size_t bytesReceived = socket->Receive(&inboundData[inboundBytesPending], cReadSize);
		assert(bytesReceived <= cReadSize);
		assert(inboundBytesPending + bytesReceived <= inboundData.size());

		totalBytesRead += bytesReceived;
		inboundBytesPending += bytesReceived;

		if (bytesReceived == 0)
			break;
	}

	if (totalBytesRead > 0)
	{
		lastHeardTime = clb::Clock::Tick();
//		LOG("Received %d bytes from TCP socket.", totalBytesRead);
	}

	AddInboundStats(totalBytesRead, 0, 0);
	StreamExtractMessages();

	if (totalBytesRead >= maxBytesToRead)
		return SocketReadThrottled;
	else
		return SocketReadOK;
}

MessageConnection::SocketReadResult MessageConnection::ReadDatagramSocket(size_t &totalBytesRead)
{
	assert(inboundBytesPending == 0);

	totalBytesRead = 0;

	// Cap the number of datagrams to read in a single loop to perform throttling.
	int datagramsLeftToRead = cMaxDatagramsToReadInOneFrame;

	/// Read in all the bytes that are available in the socket.
	const size_t cReadSize = 2048;
	inboundData.resize(2048);
	size_t bytesReceived = 0;
	do
	{
		bytesReceived = socket->Receive(&inboundData[0], inboundData.size());
		totalBytesRead += bytesReceived;

		if (bytesReceived > 0)
			DatagramExtractMessages(&inboundData[0], bytesReceived);

	} while(bytesReceived != 0 && --datagramsLeftToRead > 0);

	if (totalBytesRead > 0)
		lastHeardTime = clb::Clock::Tick();

	AddInboundStats(totalBytesRead, 0, 0);

	if (datagramsLeftToRead <= 0)
	{
		LOG("Warning: Too many inbound messages: Datagram read loop throttled!");
		return SocketReadThrottled;
	}
	else
		return SocketReadOK;
}

MessageConnection::SocketReadResult MessageConnection::ReadInjectedDatagrams(size_t &totalBytesRead)
{
	totalBytesRead = 0;

	// Cap the number of datagrams to read in a single loop to perform throttling.
	int datagramsLeftToRead = cMaxDatagramsToReadInOneFrame;

	while(datagramsLeftToRead-- > 0 && inboundInjectedDatagramQueue.Size() > 0)
	{
		Datagram *data = inboundInjectedDatagramQueue.Front();
		assert(data);
		assert(data->size > 0);
		totalBytesRead += data->size;

		DatagramExtractMessages((char*)&data->data[0], data->size);

		inboundInjectedDatagramQueue.PopFront();
	}

	if (totalBytesRead > 0)
		lastHeardTime = clb::Clock::Tick();

	AddInboundStats(totalBytesRead, 0, 0);

	if (datagramsLeftToRead == 0)
	{
		LOG("Warning: Too many inbound messages: Datagram read loop throttled!");
		return SocketReadThrottled;
	}
	else
		return SocketReadOK;
}

MessageConnection::SocketReadResult MessageConnection::ReadSocket(size_t &bytesRead)
{
	assert(socket);
	if (socket->TransportLayer() == SocketOverTCP)
		return ReadStreamSocket(bytesRead);
	else
	{
		size_t injectedBytesRead = 0;
		SocketReadResult readResult;
		
		if (injectionEnabled)
			readResult = ReadInjectedDatagrams(bytesRead);
		else
			readResult = ReadDatagramSocket(bytesRead);
//		bytesRead += injectedBytesRead;

		if (bytesRead > 0 && connectionState == ConnectionPending)
		{
			connectionState = ConnectionOK;
//			LOG("Established connection to socket %s.", socket->ToString().c_str());
		}
		if (readResult == SocketReadError)
			return SocketReadError;
		if (readResult == SocketReadThrottled)
			return SocketReadThrottled;
		return SocketReadOK;
	}
}

/// Called from the main thread to fetch & handle all new inbound messages.
void MessageConnection::ProcessMessages()
{
	if (workThreadErrorOccurred || connectionState == ConnectionClosed)
	{
		if (socket)
			Close();
		return;
	}

	if (inboundMessageQueue.Size() > 0 && !inboundMessageHandler)
	{
		LOG("Warning! Cannot process messages since no message handler registered to connection %s!",
			ToString().c_str());
		return;
	}

	while(inboundMessageQueue.Size() > 0)
	{
		NetworkMessage **message = inboundMessageQueue.Front();
		assert(message);
		NetworkMessage *msg = *message;
		inboundMessageQueue.PopFront();
		assert(msg);
		assert(msg->contentID != 0xCDCDCDCD);

		inboundMessageHandler->HandleMessage(this, msg->id, (msg->data.size() == 0) ? 0 : &msg->data[0], msg->data.size());

		FreeMessage(msg);
	}
}

void MessageConnection::InjectDatagram(const Datagram &datagram)
{
	assert(socket && socket->TransportLayer() == SocketOverUDP);
	inboundInjectedDatagramQueue.Insert(datagram);
	WSASetEvent(eventSocketReady);
}

void MessageConnection::SetInjectionMode(bool injectionEnabled_)
{
	injectionEnabled = injectionEnabled_;
}

bool EraseReliableIfObsoleteOrNotInOrderCmp(const NetworkMessage *msg)
{
	assert(msg->reliable);
	return msg->inOrder == false || msg->obsolete;
}

MessageConnection::PacketSendResult MessageConnection::SendOutPacket()
{
	if (socket->TransportLayer() == SocketOverTCP)
		return StreamSendOutPacket();
	else
	{
		PacketAckTrack *timedOutReliableInOrderPacket = 0;
		packet_id_t inOrderPacketID = 0;

		clb::tick_t now = clb::Clock::Tick();

		// Check whether any reliable packets have timed out and not acked.
		PacketAckTrackMap::iterator iter = outboundPacketAckTrack.begin();
		int numPacketsTimedOut = 0;
		while(iter != outboundPacketAckTrack.end())
		{
			// The packets are stored sorted in their timestamps, oldest first.
			// If not yet timed out, break out and continue normally by sending new messages.
			if (clb::Clock::TimespanToMilliseconds(iter->second.tick, now) < timeOutMilliseconds)
				break; // If this packet has not timed out, no subsequent messages have timed out either, so break. (stored oldest to newest)

			++numPacketsTimedOut;
			
//			LOG("A packet with ID %d timed out. Age: %.2fms. Contains %d messages.", iter->first, clb::Clock::TimespanToMilliseconds(iter->second.tick, now), iter->second.messages.size());

/* This logic is not used currently. Ping timeout is a better method.
			// Check if a message in this timed out packet has been sent too many times. In this case,
			// declare the connection timed out.
			for(std::vector<NetworkMessage*>::iterator iter2 = iter->second.messages.begin(); iter2 != iter->second.messages.end(); ++iter2)
				if ((*iter2)->sendCount >= maxMessageSendCount)
				{
					LOG("Message resend count limit reached! Connection timed out, closing.");
					connectionState = ConnectionClosed;
					return PacketSendSocketClosed;
				}
*/

			// For each message in the timed out packet, we have to decide what to do with it:
			// 1) We didn't store any unreliable packets, since these don't need to be acked.
			// 2) If the message has become obsolete, discard it. (even if it was in-order and reliable, since it IS obsolete)
			// 3) If the message is not obsolete, but is not with in-order requirements, release it back to the outbound message queue.
			//    This will cause it to be reprioritized with respect to any new messages that are being sent.
			// 4) If the message has in-order requirements, we have to respect it and send it with the exact packetID that it was meant to.
			//    In that case, leave this message into the PacketAckTrack map and pass it to DatagramSendOutPacket to be sent now.
			std::vector<NetworkMessage*>::iterator newEnd = std::remove_if(iter->second.messages.begin(), iter->second.messages.end(), EraseReliableIfObsoleteOrNotInOrderCmp);
			for(std::vector<NetworkMessage*>::iterator iter2 = newEnd; iter2 != iter->second.messages.end(); ++iter2)
			{
				if ((*iter2)->obsolete)
					FreeMessage(*iter2); // Don't resend obsolete message (don't care if they were inorder or reliable, they were marked obsolete now)
				else
					outboundQueue.Insert(*iter2); // This is a reliable non-in-order message, just put it back in the outbound queue.
			}
            assert(!ContainsDuplicates(outboundQueue));
			iter->second.messages.erase(newEnd, iter->second.messages.end());

			// If there are any messages left to be sent with this packetID, rembember this packetID.
			if (iter->second.messages.size() > 0)
			{
				if (!timedOutReliableInOrderPacket)
					timedOutReliableInOrderPacket = &iter->second;
				++iter;
			}
			else
			{
				// If there are no messages left with this packetID, we don't bother resubmitting a packet with this ID,
				// just move the messages into a packet with a new ID.
				PacketAckTrackMap::iterator next = iter;
				++next;
				outboundPacketAckTrack.erase(iter);
				iter = next;
			}
		}

		MessageConnection::PacketSendResult result = DatagramSendOutPacket(timedOutReliableInOrderPacket);

		// If we didn't have any messages to put into the packet with the old packetID, send a new packet with new PacketID.
		if (result == PacketSendNoMessages && outboundQueue.Size() > 0 && timedOutReliableInOrderPacket != 0) 
			result = DatagramSendOutPacket(0);

		return result;
	}
}


/// Packs several messages from the outbound priority queue into a single packet and sends it out the wire.
/// @return False if the send was a failure and sending should not be tried again at this time, true otherwise.
MessageConnection::PacketSendResult MessageConnection::StreamSendOutPacket()
{
	if (bOutboundSendsPaused)
		return PacketSendNoMessages;

	const size_t minSendSize = 1;
	const size_t maxSendSize = socket->MaxSendSize();
	// Push out all the pending data to the socket.
	std::vector<NetworkMessage*> serializedMessages;
	std::vector<char> out;
	out.resize(maxSendSize);
	int numMessagesPacked = 0;
	DataSerializer writer(&out[0], out.size());
	while(outboundQueue.Size() > 0)
	{
		NetworkMessage *msg = outboundQueue.Front();

		if (msg->obsolete)
		{
			FreeMessage(msg);
			outboundQueue.PopFront();
			continue;
		}
		const int encodedMsgIdLength = VLE8_16_32::GetEncodedBitLength(msg->id) / 8;
		const size_t messageContentSize = msg->data.size() + encodedMsgIdLength; // 1 byte: Message ID. X bytes: Content.
		const int encodedMsgSizeLength = VLE8_16_32::GetEncodedBitLength(messageContentSize) / 8;
		const size_t totalMessageSize = messageContentSize + encodedMsgSizeLength; // 2 bytes: Content length. X bytes: Content.
		// If this message won't fit into the buffer, send out all the previously gathered messages.
		if (writer.BytesFilled() + totalMessageSize >= maxSendSize)
			break;

		writer.AddVLE<VLE8_16_32 >(messageContentSize);
		writer.AddVLE<VLE8_16_32 >(msg->id);
		if (msg->data.size() > 0)
			writer.AddArray<u8>((const u8*)&msg->data[0], msg->data.size());
		++numMessagesPacked;

		serializedMessages.push_back(msg);
		outboundQueue.PopFront();
	}

	if (writer.BytesFilled() == 0 && outboundQueue.Size() > 0)
		LOG("Failed to send any messages to socket %s! (Probably next message was too big to fit in the buffer.", socket->ToString().c_str());

	if (writer.BytesFilled() >= minSendSize)
	{
		bool success = socket->Send(writer.GetData(), writer.BytesFilled());
		if (!success)
		{
			for(size_t i = 0; i < serializedMessages.size(); ++i)
				outboundQueue.Insert(serializedMessages[i]);

            assert(!ContainsDuplicates(outboundQueue));

			return PacketSendSocketFull;
		}

//		LOG("Sent %d bytes (%d messages) to socket.", writer.BytesFilled(), serializedMessages.size());
		AddOutboundStats(writer.BytesFilled(), 0, numMessagesPacked);

		for(size_t i = 0; i < serializedMessages.size(); ++i)
			FreeMessage(serializedMessages[i]);

		return PacketSendOK;
	}
	else // Not enough bytes to send out. Put all the messages back in the queue.
	{
		for(size_t i = 0; i < serializedMessages.size(); ++i)
			outboundQueue.Insert(serializedMessages[i]);

        assert(!ContainsDuplicates(outboundQueue));

		return PacketSendNoMessages;
	}
}

int NetworkMessage::GetTotalDatagramPackedSize() const
{
	const int idLength = (transfer == 0 || fragmentIndex == 0) ? VLE8_16_32::GetEncodedBitLength(id)/8 : 0;
	const int headerLength = 2;
	const int contentLength = data.size();
	const int fragmentStartLength = (transfer && fragmentIndex == 0) ? VLE8_16_32::GetEncodedBitLength(transfer->totalNumFragments)/8 : 0;
	const int fragmentLength = (transfer ? 1 : 0) + ((transfer && fragmentIndex != 0) ? VLE8_16_32::GetEncodedBitLength(fragmentIndex)/8 : 0);

	///\todo Take into account the inOrder field.
	return idLength + headerLength + contentLength + fragmentStartLength + fragmentLength;
}

namespace
{
	struct OutboundSerializeTrack
	{
		NetworkMessage *msg;

		/// If true, this message is being added from an old in-order reliable packet that timed out.
		bool fromOldInOrderPacket;
	};
}

bool OutboundSerializeTrackContains(const std::vector<OutboundSerializeTrack> &cont, const NetworkMessage *msg)
{
    for(std::vector<OutboundSerializeTrack>::const_iterator iter = cont.begin(); iter != cont.end(); ++iter)
        if (iter->msg == msg)
            return true;

    return false;
}

/// Packs several messages from the outbound priority queue into a single packet and sends it out the wire.
/// @return False if the send was a failure and sending should not be tried again at this time, true otherwise.
MessageConnection::PacketSendResult MessageConnection::DatagramSendOutPacket(PacketAckTrack *resendPacket)
{
	if (bOutboundSendsPaused)
		return PacketSendNoMessages;

	if (datagramSendThrottleTimer.Enabled() && !datagramSendThrottleTimer.Test())
		return PacketSendThrottled;

	const size_t minSendSize = 1;
	const size_t maxSendSize = socket->MaxSendSize();

	// Push out all the pending data to the socket.
	std::vector<OutboundSerializeTrack> serializedMessages;

	// If true, the receiver needs to Ack the packet we are now crafting.
	bool reliable = false;
	// If true, the packet contains in-order deliverable messages.
	bool inOrder = false;

	int packetSizeInBytes = 3; // PacketID + Flags take at least three bytes to start with.
	const int cBytesForInOrderDeltaCounter = 2;

	// If we are sending a resend of a previous packet, add all messages from that packet to the send queue first.
	if (resendPacket)
	{
		for(size_t i = 0; i < resendPacket->messages.size(); ++i)
		{
			NetworkMessage *msg = resendPacket->messages[i];
			assert(msg->reliable); // Only reliable messages are in the ack queue.
			assert(msg->inOrder); // Reliable not-in-order messages should have been put back into outboundqueue for better priorization.
			assert(!msg->obsolete); // We should've pruned all obosoletes out before entering here.
			// If this message is part of a fragmented transfer, the transferID of that transfer must have been previously set.
			// assert(!msg->transfer || msg->transfer->id != -1); // Cannot have this assert here - race condition!

			OutboundSerializeTrack t;
			t.fromOldInOrderPacket = true;
			t.msg = msg;
            assert(!OutboundSerializeTrackContains(serializedMessages, msg));
			serializedMessages.push_back(t);
			packetSizeInBytes += msg->GetTotalDatagramPackedSize();

			assert((size_t)packetSizeInBytes <= maxSendSize);

			if (!inOrder)
			{
				inOrder = true;
				packetSizeInBytes += cBytesForInOrderDeltaCounter;
			}
			reliable = true;	
		}
//		LOG("Resending timed out packetID %u. %d messages/%d bytes to be added inOrder", resendPacket->packetID, (int)serializedMessages.size(), packetSizeInBytes);
	}

	std::vector<NetworkMessage *> skippedMessages;

    assert(!ContainsDuplicates(outboundQueue));

	// Fill up the rest of the packet from messages from the outbound queue.
	while(outboundQueue.Size() > 0)
	{
		NetworkMessage *msg = outboundQueue.Front();
//        LOG("Reading message 0x%X, %d items in outboundQueue.", msg, outboundQueue.Size());
		if (msg->obsolete)
		{
			outboundQueue.PopFront();
			FreeMessage(msg);
			continue;
		}

		// If we're resending an old in-order reliable packet, we can't add any new
		// in-order messages into it, since we'll be sending the packet with an old
		// packetID. Also, we can't add any reliable packets, since it might be that
		// the Ack was just delayed. Finally, we can't add any packets with a contentID,
		// since the packetID numbers will differ.
		if (resendPacket && (msg->inOrder || msg->reliable || msg->contentID != 0))
		{
			outboundQueue.PopFront();
            assert(!MessageListContains(skippedMessages, msg));
			skippedMessages.push_back(msg);
			continue;
		}

		// If we're sending a fragmented message, allocate a new transferID for that message,
		// or skip it if there are no transferIDs free.
		if (msg->transfer)
		{
			Lock<FragmentedSendManager> sends = fragmentedSends.Acquire();
			if (msg->transfer->id == -1)
			{
				bool success = sends->AllocateFragmentedTransferID(*msg->transfer);

				if (!success) // No transferIDs free - skip this message for now.
				{
					LOG("Throttling fragmented transfer send! No free TransferID to start a new fragmented transfer with!");
					outboundQueue.PopFront();
                    assert(!MessageListContains(skippedMessages, msg));
					skippedMessages.push_back(msg);
					continue;
				}
			}
		}

		// We need to add extra 2 bytes for the VLE-encoded InOrder PacketID delta counter.
		int totalMessageSize = msg->GetTotalDatagramPackedSize() + ((msg->inOrder && !inOrder) ? cBytesForInOrderDeltaCounter : 0);

		// If this message won't fit into the buffer, send out all the previously gathered messages.
		if ((size_t)packetSizeInBytes >= minSendSize && (size_t)packetSizeInBytes + totalMessageSize >= maxSendSize)
			break;

		OutboundSerializeTrack t;
		t.fromOldInOrderPacket = false;
		t.msg = msg;
        assert(!OutboundSerializeTrackContains(serializedMessages, msg));
		serializedMessages.push_back(t);
		outboundQueue.PopFront();

		packetSizeInBytes += totalMessageSize;

		if (msg->reliable)
			reliable = true;

		if (msg->inOrder)
			inOrder = true;
	}
//	if (skippedMessages.size() > 0)
//		LOG("Had to skip %d messages when resending a packet with old PacketID %d, but could add %d unreliable messages more.", (int)skippedMessages.size(), (int)resendPacket->packetID,
//		(int)(serializedMessages.size() - resendPacket->messages.size()));

	for(size_t i = 0; i < skippedMessages.size(); ++i)
    {
        assert(outboundQueue.Search(skippedMessages[i]) == -1);
		outboundQueue.Insert(skippedMessages[i]);
    }

    assert(!ContainsDuplicates(outboundQueue));

	if (serializedMessages.size() == 0 || packetSizeInBytes < minSendSize)
	{
		//	Not enough bytes to send out. Put all the messages back in the queue.
		for(size_t i = 0; i < serializedMessages.size(); ++i)
			if (!serializedMessages[i].fromOldInOrderPacket)
            {
                assert(outboundQueue.Search(serializedMessages[i].msg) == -1);
				outboundQueue.Insert(serializedMessages[i].msg);
            }

		if (resendPacket)
			LOG("Doing a ResendPacket on ID %d, but no messages to serialize!", resendPacket->packetID);

        assert(!ContainsDuplicates(outboundQueue));

		return PacketSendNoMessages;
	}

	// Finally proceed to crafting the actual UDP packet.

	std::vector<char> out;
	out.resize(maxSendSize);

	DataSerializer writer(&out[0], out.size());

	// Write the packet header.
	packet_id_t packetID = resendPacket ? resendPacket->packetID : datagramPacketIDCounter;
	packet_id_t lastInOrderID = resendPacket ? resendPacket->previousInOrderPacketID : lastSentInOrderPacketID;
	writer.Add<u8>((u8)((datagramPacketIDCounter & 63) | ((reliable ? 1 : 0) << 6)  | ((inOrder ? 1 : 0) << 7)));
	writer.Add<u16>((u16)(datagramPacketIDCounter >> 6));
	if (inOrder)
	{
		int packetDelta = SubPacketID(packetID, lastInOrderID);
		if (packetDelta > 0x7FFF) ///\todo Remove these two lines and do the check in SubPacketID.
			packetDelta = 0;

		writer.AddVLE<VLE8_16>(packetDelta);
	}

	bool sentDisconnectAckMessage = false;

	// Write all the messages in this UDP packet.
	for(size_t i = 0; i < serializedMessages.size(); ++i)
	{
		NetworkMessage *msg = serializedMessages[i].msg;
		assert(!msg->transfer || msg->transfer->id != -1);

		const int encodedMsgIdLength = (msg->transfer == 0 || msg->fragmentIndex == 0) ? VLE8_16_32::GetEncodedBitLength(msg->id)/8 : 0;
		const size_t messageContentSize = msg->data.size() + encodedMsgIdLength; // 1/2/4 bytes: Message ID. X bytes: Content.
		assert(messageContentSize < (1 << 11));

		if (msg->id == MsgIdDisconnectAck)
			sentDisconnectAckMessage = true;

		const u16 inOrder = (msg->inOrder ? 1 : 0) << 13;
		const u16 fragmentedTransfer = (msg->transfer != 0 ? 1 : 0) << 14;
		const u16 firstFragment = (msg->transfer != 0 && msg->fragmentIndex == 0 ? 1 : 0) << 15;
		writer.Add<u16>((u16)messageContentSize | inOrder | fragmentedTransfer | firstFragment);

		///\todo Add the InOrder index here to track which datagram/message we depended on.

		if (firstFragment != 0)
			writer.AddVLE<VLE8_16_32>(msg->transfer->totalNumFragments);
		if (fragmentedTransfer != 0)
			writer.Add<u8>((u8)msg->transfer->id);
		if (firstFragment == 0 && fragmentedTransfer != 0)
			writer.AddVLE<VLE8_16_32>(msg->fragmentIndex);

		if (msg->transfer == 0 || msg->fragmentIndex == 0)
			writer.AddVLE<VLE8_16_32>(msg->id);
		if (msg->data.size() > 0)
			writer.AddArray<u8>((const u8*)&msg->data[0], (u32)msg->data.size());

		assert(writer.BytesFilled() <= (size_t)packetSizeInBytes); // Oops, serializing the messages overflowed the UDP MTU limit we have!
	}

	// Send the crafted packet out to the socket.
	bool success = socket->Send(writer.GetData(), writer.BytesFilled());
	if (!success)
	{
		// We failed, so put all messages back to the outbound queue, except for those that are from old in-order packet,
		// since they need to be resent with the old packet ID and not as fresh messages.
		for(size_t i = 0; i < serializedMessages.size(); ++i)
			if (!serializedMessages[i].fromOldInOrderPacket)
            {
                assert(outboundQueue.Search(serializedMessages[i].msg) == -1);
				outboundQueue.Insert(serializedMessages[i].msg);
            }

		if (resendPacket)
			LOG("Doing a ResendPacket, but Socket::Send failed!");

        assert(!ContainsDuplicates(outboundQueue));

		LOG("Socket::Send failed to socket %s!", socket->ToString().c_str());
		return PacketSendSocketFull;
	}

	// Sending the datagram succeeded - increment the send count of each message by one, to remember the retry timeout count.
	for(size_t i = 0; i < serializedMessages.size(); ++i)
		++serializedMessages[i].msg->sendCount;

//	LOG("Sent out a packet with ID %d.", (unsigned long)packetID);

	assert(socket->TransportLayer() == SocketOverUDP);
	datagramSendThrottleTimer.StartTicks(clb::Clock::TicksPerSec() / datagramOutRatePerSecond);

	// The send was successful, we can increment our next free PacketID counter to use for the next packet.
	if (!resendPacket)
	{
		lastSentInOrderPacketID = datagramPacketIDCounter;
		datagramPacketIDCounter = AddPacketID(datagramPacketIDCounter, 1);
	}

	AddOutboundStats(writer.BytesFilled(), 1, serializedMessages.size());

	if (reliable)
	{
		if (!resendPacket)
			outboundPacketAckTrack[packetID] = PacketAckTrack();
		PacketAckTrack &ack = resendPacket ? *resendPacket : outboundPacketAckTrack[packetID]; 
		ack.packetID = packetID;
		ack.tick = clb::Clock::Tick();
		
		for(size_t i = 0; i < serializedMessages.size(); ++i)
		{
			assert(!resendPacket || serializedMessages[i].fromOldInOrderPacket || !serializedMessages[i].msg->inOrder);
			if (!serializedMessages[i].fromOldInOrderPacket && serializedMessages[i].msg->reliable)
            {
                assert(!MessageListContains(ack.messages, serializedMessages[i].msg));
				ack.messages.push_back(serializedMessages[i].msg);
            }
		}
	}
	else
	{
		if (resendPacket)
		{
			LOG("Reliable PacketID %d was demoted to a nonreliable packetID so removing ack track.", resendPacket->packetID);
			outboundPacketAckTrack.erase(resendPacket->packetID);
		}

		// This is send-and-forget, we can free the message data we just sent.
		for(size_t i = 0; i < serializedMessages.size(); ++i)
			FreeMessage(serializedMessages[i].msg);
	}

	// If we sent out the DisconnectAck message, we can close down the connection right now.
	if (sentDisconnectAckMessage)
	{
		connectionState = ConnectionClosed;
		LOG("Connection closed by peer: %s.", ToString().c_str());
	}

	return PacketSendOK;
}

void MessageConnection::AddOutboundStats(unsigned long numBytes, unsigned long numPackets, unsigned long numMessages)
{
	if (numBytes == 0 && numMessages == 0 && numPackets == 0)
		return;

	ConnectionStatistics &cs = stats.Lock();
	cs.traffic.push_back(ConnectionStatistics::TrafficTrack());
	ConnectionStatistics::TrafficTrack &t = cs.traffic.back();
	t.bytesIn = t.messagesIn = t.packetsIn = 0;
	t.bytesOut = numBytes;
	t.packetsOut = numPackets;
	t.messagesOut = numMessages;
	t.tick = clb::Clock::Tick();
	stats.Unlock();
}

void MessageConnection::AddInboundStats(unsigned long numBytes, unsigned long numPackets, unsigned long numMessages)
{
	if (numBytes == 0 && numMessages == 0 && numPackets == 0)
		return;

	ConnectionStatistics &cs = stats.Lock();
	cs.traffic.push_back(ConnectionStatistics::TrafficTrack());
	ConnectionStatistics::TrafficTrack &t = cs.traffic.back();
	t.bytesOut = t.messagesOut = t.packetsOut = 0;
	t.bytesIn = numBytes;
	t.packetsIn = numPackets;
	t.messagesIn = numMessages;
	t.tick = clb::Clock::Tick();
	stats.Unlock();
}

///\todo Can remove the lock, convert from a vector to a set.
bool MessageConnection::HaveReceivedPacketID(packet_id_t packetID) const
{
	const ConnectionStatistics &cs = stats.Lock();

	for(size_t i = 0; i < cs.recvPacketIDs.size(); ++i)
		if (cs.recvPacketIDs[i].packetID == packetID)
		{
			stats.Unlock();
			return true;
		}

	stats.Unlock();
	return false;
}

void MessageConnection::AddReceivedPacketIDStats(packet_id_t packetID)
{
	ConnectionStatistics &cs = stats.Lock();

	// Simple method to prevent computation errors caused by wraparound - we start from scratch when packet with ID 0 is received.
//	if (packetID == 0)
//		cs.recvPacketIDs.clear();

	cs.recvPacketIDs.push_back(ConnectionStatistics::DatagramIDTrack());
	ConnectionStatistics::DatagramIDTrack &t = cs.recvPacketIDs.back();
	t.tick = clb::Clock::Tick();
	t.packetID = packetID;
//	LOG("Marked packet with ID %d received.", (unsigned long)packetID);
	stats.Unlock();
}

void MessageConnection::ComputeStats()
{
    using namespace std;

	ConnectionStatistics &cs = stats.Lock();

	const clb::tick_t maxEntryAge = clb::Clock::TicksPerSec() * 5;
	const clb::tick_t timeNow = clb::Clock::Tick();
	const clb::tick_t maxTickAge = timeNow - maxEntryAge;

	for(size_t i = 0; i < cs.traffic.size(); ++i)
		if (clb::Clock::IsNewer(cs.traffic[i].tick, maxTickAge))
		{
			cs.traffic.erase(cs.traffic.begin(), cs.traffic.begin() + i);
			break;
		}

	if (cs.traffic.size() <= 1)
	{
		bytesInPerSec = bytesOutPerSec = msgsInPerSec = msgsOutPerSec = packetsInPerSec = packetsOutPerSec = 0.f;
		stats.Unlock();
		return;
	}

	unsigned long totalBytesIn = 0;
	unsigned long totalBytesOut = 0;
	unsigned long totalMsgsIn = 0;
	unsigned long totalMsgsOut = 0;
	unsigned long totalPacketsIn = 0;
	unsigned long totalPacketsOut = 0;

	for(size_t i = 0; i < cs.traffic.size(); ++i)
	{
		totalBytesIn += cs.traffic[i].bytesIn;
		totalBytesOut += cs.traffic[i].bytesOut;
		totalPacketsIn += cs.traffic[i].packetsIn;
		totalPacketsOut += cs.traffic[i].packetsOut;
		totalMsgsIn += cs.traffic[i].messagesIn;
		totalMsgsOut += cs.traffic[i].messagesOut;
	}
	clb::tick_t ticks = cs.traffic.back().tick - cs.traffic.front().tick;
    // Cap the number of seconds so that the samples are evaluated at least on one second interval.
	float secs = max(5.f, clb::Clock::TicksToMilliseconds(ticks) / 1000.f);
	bytesInPerSec = (float)totalBytesIn / secs;
	bytesOutPerSec = (float)totalBytesOut / secs;
	packetsInPerSec = (float)totalPacketsIn / secs;
	packetsOutPerSec = (float)totalPacketsOut / secs;
	msgsInPerSec = (float)totalMsgsIn / secs;
	msgsOutPerSec = (float)totalMsgsOut / secs;

	stats.Unlock();
}

void MessageConnection::DatagramExtractMessages(const char *data, size_t numBytes)
{
//	LOG("Received a datagram of %d bytes.", numBytes);
	assert(data);
	assert(numBytes > 0);

	if (numBytes < 3)
	{
		LOG("Malformed UDP packet! Size = %d bytes, no space for packet header, which is at least 3 bytes.", numBytes);
		return;
	}

	DataDeserializer reader(data, numBytes);

	// Start by reading the packet header (flags, packetID).
	u8 flags = reader.Read<u8>();
	bool inOrder = (flags & (1 << 7)) != 0;
	bool reliable = (flags & (1 << 6)) != 0;
	packet_id_t packetID = (reader.Read<u16>() << 6) | (flags & 63);

	// If the 'reliable'-flag is set, remember this PacketID, we need to Ack it later on.
	if (reliable)
	{
		PacketAckTrack &t = inboundPacketAckTrack[packetID];
		t.packetID = packetID;
		t.tick = clb::Clock::Tick();
	}

	if (HaveReceivedPacketID(packetID))
	{
		LOG("Received packetID %d, but it is a duplicate of a packet received before!", packetID);
		return;
	}

	// If the 'inOrder'-flag is set, there's an extra 'Order delta counter' field present,
	// that specifies the processing ordering of this packet.
	packet_id_t inOrderID = 0;
	if (inOrder)
	{
		inOrderID = reader.ReadVLE<VLE8_16>();
		if (inOrderID == DataDeserializer::VLEReadError)
		{
			LOG("Malformed UDP packet! Size = %d bytes, no space for packet header field 'inOrder'!", numBytes);
			return;
		}
	}

	size_t numMessagesReceived = 0;
	while(reader.BytesLeft() > 0)
	{
		if (reader.BytesLeft() < 2)
		{
			LOG("Malformed UDP packet! Parsed %d messages ok, but after that there's not enough space for UDP message header! BytePos %d, total size %d",
				reader.BytePos(), numBytes);
			return;
		}

		// Read the message header (2 bytes at least).
		u16 contentLength = reader.Read<u16>();
		bool fragmentStart = (contentLength & (1 << 15)) != 0;
		bool fragment = (contentLength & (1 << 14)) != 0 || fragmentStart; // If fragmentStart is set, then fragment is set.
		bool inOrder = (contentLength & (1 << 13)) != 0;
		contentLength &= (1 << 11) - 1;

		if (contentLength == 0)
		{
			LOG("Malformed UDP packet! Byteofs %d, Packet length %d. Message had zero length (Must have at least 1-3 bytes for the MsgID)!", reader.BytePos(), numBytes);
			return;
		}

		u32 numTotalFragments = (fragmentStart ? reader.ReadVLE<VLE8_16_32>() : 0);
		u8 fragmentTransferID = (fragment ? reader.Read<u8>() : 0);
		u32 fragmentNumber = (fragment && !fragmentStart ? reader.ReadVLE<VLE8_16_32>() : 0);

		if (reader.BytesLeft() < contentLength)
		{
			LOG("Malformed UDP packet! Byteofs %d, Packet length %d. Expected %d bytes of content, but only %d left!",
				reader.BytePos(), numBytes, contentLength, reader.BytesLeft());
			return;
		}

		// If we received the start of a new fragment, start tracking a new fragmented transfer.
		if (fragmentStart)
		{
			if (numTotalFragments == DataDeserializer::VLEReadError || numTotalFragments <= 1)
			{
				LOG("Malformed UDP packet!");
				return;
			}

			fragmentedReceives.NewFragmentStartReceived(fragmentTransferID, numTotalFragments, &data[reader.BytePos()], contentLength);
		}
		// If we received a fragment that is a part of an old fragmented transfer, pass it to the fragmented transfer manager
		// so that it can reconstruct the final stream when the transfer finishes.
		else if (fragment)
		{
			if (fragmentNumber == DataDeserializer::VLEReadError)
			{
				LOG("Malformed UDP packet!");
				return;
			}

			bool messageReady = fragmentedReceives.NewFragmentReceived(fragmentTransferID, fragmentNumber, &data[reader.BytePos()], contentLength);
			if (messageReady)
			{
				// This was the last fragment of the whole message - reconstruct the message from the fragments and pass it on to
				// the client to handle.
				std::vector<char> assembledData;
				fragmentedReceives.AssembleMessage(fragmentTransferID, assembledData);
				assert(assembledData.size() > 0);
				HandleInboundMessage(packetID, &assembledData[0], assembledData.size());
				++numMessagesReceived;
				fragmentedReceives.FreeMessage(fragmentTransferID);
			}
		}
		else
		{
			// Not a fragment, so directly call the handling code.
			HandleInboundMessage(packetID, &data[reader.BytePos()], contentLength);
			++numMessagesReceived;
		}

		reader.SkipBytes(contentLength);
	}

	// Store the packetID for inbound packet loss statistics purposes.
	AddReceivedPacketIDStats(packetID);
	// Save general statistics (bytes, packets, messages rate).
	AddInboundStats(0, 1, numMessagesReceived);
}

void MessageConnection::StreamExtractMessages()
{
	///\todo Remove try-catch. Not wanted here.
	try
	{

	size_t numMessagesReceived = 0;
	for(;;)
	{
		if (inboundBytesPending == 0) // No new packets in yet.
			break;

		DataDeserializer reader(&inboundData[0], inboundBytesPending);
		u32 messageSize = reader.ReadVLE<VLE8_16_32>();
		if (messageSize == DataDeserializer::VLEReadError)
			break; // The packet hasn't yet been streamed in.

		if (messageSize == 0 || messageSize > cMaxTCPMessageSize)
		{
			LOG("Received an invalid message size %d! Closing connection!", messageSize);
			workThreadErrorOccurred = true;
			return;
		}

		if (reader.BytesLeft() < messageSize)
			break;

		HandleInboundMessage(0, &inboundData[reader.BytePos()], messageSize);
		reader.SkipBytes(messageSize);

		assert(reader.BitPos() == 0);
		u32 bytesConsumed = reader.BytePos();
		assert(bytesConsumed <= inboundBytesPending);
		inboundBytesPending -= bytesConsumed;
		inboundData.erase(inboundData.begin(), inboundData.begin() + bytesConsumed);
		++numMessagesReceived;
	}
	AddInboundStats(0, 1, numMessagesReceived);

	} catch(...) ///\todo Write own exception class.
	{
		LOG("DataSerializer exception thrown in StreamExtractMessages!");
	}
}

void MessageConnection::CheckAndSaveOutboundMessageWithContentID(NetworkMessage *msg)
{
	assert(msg);

	if (msg->contentID == 0)
		return;

	MsgContentIDPair key = std::make_pair(msg->id, msg->contentID);
	ContentIDSendTrack::iterator iter = outboundContentIDMessages.find(key);
	if (iter != outboundContentIDMessages.end())
	{
		if (msg->IsNewerThan(*iter->second))
		{				
			iter->second->obsolete = true;

			assert(iter->second != msg);
			assert(iter->first.first == iter->second->id);
			assert(iter->first.second == iter->second->contentID);
			assert(iter->first.first == msg->id);
			assert(iter->first.second == msg->contentID);
			iter->second = msg;
		}
		else
		{
			LOG("Warning! Adding new message ID %u, number %u, content ID %u, priority %u, but it was obsoleted by an already existing message number %u.", 
				msg->id, msg->messageNumber, msg->contentID, iter->second->priority, iter->second->messageNumber);
			msg->obsolete = true;
		}
	}
	else
	{
		outboundContentIDMessages[key] = msg;
	}
}

void MessageConnection::ClearOutboundMessageWithContentID(NetworkMessage *msg)
{
	///\bug Possible race condition here. Accessed by both main and worker thread through a call from FreeMessage.
	assert(msg);
	if (msg->contentID == 0)
		return;
	MsgContentIDPair key = std::make_pair(msg->id, msg->contentID);
	ContentIDSendTrack::iterator iter = outboundContentIDMessages.find(key);
	if (iter != outboundContentIDMessages.end())
		if (msg == iter->second)
			outboundContentIDMessages.erase(iter);
}

bool MessageConnection::CheckAndSaveContentIDStamp(u32 messageID, u32 contentID, packet_id_t packetID)
{
	assert(contentID != 0);

	clb::tick_t now = clb::Clock::Tick();

	MsgContentIDPair key = std::make_pair(messageID, contentID);
	ContentIDReceiveTrack::iterator iter = inboundContentIDStamps.find(key);
	if (iter == inboundContentIDStamps.end())
	{
		inboundContentIDStamps[key] = std::make_pair(packetID, now);
		return true;
	}
	else
	{
		if (PacketIDIsNewerThan(packetID, iter->second.first) || clb::Clock::TimespanToMilliseconds(iter->second.second, now) > 5.f * 1000.f)
		{
			iter->second = std::make_pair(packetID, now);
			return true;
		}
		else
			return false;
	}
}

void MessageConnection::HandleInboundMessage(packet_id_t packetID, const char *data, size_t numBytes)
{
	assert(data && numBytes > 0);

	// Read the message ID.
	DataDeserializer reader(data, numBytes);
	u32 messageID = reader.ReadVLE<VLE8_16_32>(); ///\todo Check that there actually is enough space to read.
	if (messageID == DataDeserializer::VLEReadError)
	{
		LOG("Error parsing messageID of a message in socket %s. Data size: %d bytes.", socket->ToString().c_str(), numBytes);
		return;
	}

	switch(messageID)
	{
	case MsgIdPingRequest:
		HandlePingRequestMessage(data + reader.BytePos(), reader.BytesLeft());
		break;
	case MsgIdPingReply:
		HandlePingReplyMessage(data + reader.BytePos(), reader.BytesLeft());
		break;
	case MsgIdFlowControlRequest:
		HandleFlowControlRequestMessage(data + reader.BytePos(), reader.BytesLeft());
		break;
	case MsgIdPacketAck:
		HandlePacketAckMessage(data + reader.BytePos(), reader.BytesLeft());
		break;
	case MsgIdDisconnect:
		HandleDisconnectMessage();
		break;
	case MsgIdDisconnectAck:
		HandleDisconnectAckMessage();
		break;
	default:
		{
			if (!inboundMessageHandler)
				LOG("Warning! Cannot compute the content ID of an incoming message since no handler was registered to connection %s!", ToString().c_str());

			if (socket->TransportLayer() == SocketOverUDP && inboundMessageHandler)
			{
				u32 contentID = inboundMessageHandler->ComputeContentID(messageID, data, numBytes);
				if (contentID != 0 && CheckAndSaveContentIDStamp(messageID, contentID, packetID) == false)
				{
					LOG("MessageID %u in packetID %d and contentID %u is obsolete! Skipped.", messageID, (int)packetID, contentID);
					return;
				}
			}

			NetworkMessage *msg = AllocateNewMessage();
			msg->data.insert(msg->data.end(), data + reader.BytePos(), data + numBytes);
			msg->id = messageID;
			msg->contentID = 0;
			if (!inboundMessageQueue.Insert(msg))
                FreeMessage(msg);
		}
		break;
	}
}

void MessageConnection::SetDatagramInFlowRatePerSecond(int newDatagramReceiveRate, bool internalQueue)
{
	if (newDatagramReceiveRate == datagramInRatePerSecond) // No need to set it multiple times.
		return;

	if (newDatagramReceiveRate < 5 || newDatagramReceiveRate > 10 * 1024)
	{
		LOG("Tried to set invalid UDP receive rate %d packets/sec! Ignored.", newDatagramReceiveRate);
		return;
	}
	
	datagramInRatePerSecond = newDatagramReceiveRate;

	NetworkMessage &msg = StartNewMessage(MsgIdFlowControlRequest);
	AppendU16ToVector(msg.data, newDatagramReceiveRate);
	msg.priority = NetworkMessage::cMaxPriority - 1;
	EndAndQueueMessage(msg, internalQueue);
}

void MessageConnection::SendPingRequestMessage()
{
	ConnectionStatistics &cs = stats.Lock();
	
	u8 pingID = (u8)((cs.ping.size() == 0) ? 1 : (cs.ping.back().pingID + 1));
	cs.ping.push_back(ConnectionStatistics::PingTrack());
	ConnectionStatistics::PingTrack &pingTrack = cs.ping.back();
	pingTrack.replyReceived = false;
	pingTrack.pingSentTick = clb::Clock::Tick();
	pingTrack.pingID = pingID;

	stats.Unlock();

	NetworkMessage &msg = StartNewMessage(MsgIdPingRequest);
	AppendU8ToVector(msg.data, pingID);
	msg.priority = NetworkMessage::cMaxPriority - 2;
	EndAndQueueMessage(msg, true);
}

void MessageConnection::SendPacketAckMessage()
{
	while(inboundPacketAckTrack.size() > 0)
	{
		packet_id_t packetID = inboundPacketAckTrack.begin()->first;
		u32 sequence = 0;

		inboundPacketAckTrack.erase(packetID);
		for(int i = 0; i < 32; ++i)
		{
			packet_id_t id = AddPacketID(packetID, i + 1);
			
			PacketAckTrackMap::iterator iter = inboundPacketAckTrack.find(id);
			if (iter != inboundPacketAckTrack.end())
			{
				sequence |= 1 << i;
				inboundPacketAckTrack.erase(id);
			}
		}

		NetworkMessage &msg = StartNewMessage(MsgIdPacketAck);
		msg.data.resize(7);
		DataSerializer mb(&msg.data[0], 7);
		mb.Add<u8>(packetID & 0xFF);
		mb.Add<u16>((u16)packetID >> 8);
		mb.Add<u32>(sequence);
		msg.priority = NetworkMessage::cMaxPriority - 1;
		EndAndQueueMessage(msg, true);
	}
}

void MessageConnection::SendDisconnectMessage(bool isInternal)
{
	NetworkMessage &msg = StartNewMessage(MsgIdDisconnect);
	msg.priority = NetworkMessage::cMaxPriority; ///\todo Highest or lowest priority depending on whether to finish all pending messages?
	msg.reliable = true;
	EndAndQueueMessage(msg, isInternal);
//	LOG("Added Disconnect message to queue. %d", (int)isInternal);
}

void MessageConnection::SendDisconnectAckMessage()
{
	NetworkMessage &msg = StartNewMessage(MsgIdDisconnectAck);
	msg.priority = NetworkMessage::cMaxPriority; ///\todo Highest or lowest priority depending on whether to finish all pending messages?
	msg.reliable = false;
	EndAndQueueMessage(msg, true);
//	LOG("Added DisconnectAck message to queue.");
}

void MessageConnection::HandlePingRequestMessage(const char *data, size_t numBytes)
{
	if (numBytes != 1)
	{
		LOG("Malformed PingRequest message received! Size was %d bytes, expected 1 byte!", numBytes);
		return;
	}

	u8 pingID = (u8)*data;
	NetworkMessage &msg = StartNewMessage(MsgIdPingReply);
	AppendU8ToVector(msg.data, pingID);
	msg.priority = NetworkMessage::cMaxPriority - 1;
	EndAndQueueMessage(msg, true);
}

void MessageConnection::HandlePingReplyMessage(const char *data, size_t numBytes)
{
	if (numBytes != 1)
	{
		LOG("Malformed PingReply message received! Size was %d bytes, expected 1 byte!", numBytes);
		return;
	}

	ConnectionStatistics &cs = stats.Lock();

	// How much to bias the new rtt value against the old rtt estimation. 1.f - 100% biased to the new value. near zero - very stable and nonfluctuant.
	const float rttPredictBias = 0.5f;

	u8 pingID = *(u8*)data;
	for(size_t i = 0; i < cs.ping.size(); ++i)
		if (cs.ping[i].pingID == pingID && cs.ping[i].replyReceived == false)
		{
			cs.ping[i].pingReplyTick = clb::Clock::Tick();
			float newRtt = clb::Clock::TicksToMilliseconds(clb::Clock::TicksInBetween(cs.ping[i].pingReplyTick, cs.ping[i].pingSentTick));
			cs.ping[i].replyReceived = true;
			stats.Unlock();
			rtt = rttPredictBias * newRtt + (1.f * rttPredictBias) * rtt;
			return;
		}
	stats.Unlock();
	LOG("Received PingReply with ID %d in socket %s, but no matching PingRequest was ever sent!", pingID, socket->ToString().c_str());
}

void MessageConnection::HandleFlowControlRequestMessage(const char *data, size_t numBytes)
{
	if (numBytes != 2)
	{
		LOG("Malformed FlowControlRequest message received! Size was %d bytes, expected 2 bytes!", numBytes);
		return;
	}

	const u16 minOutboundRate = 5;
	const u16 maxOutboundRate = 10 * 1024;
	u16 newOutboundRate = *reinterpret_cast<const u16*>(data);
	if (newOutboundRate < minOutboundRate || newOutboundRate > maxOutboundRate)
	{
		LOG("Invalid FlowControlRequest rate %d packets/sec received! Ignored. Valid range (%d, %d)", newOutboundRate,
			minOutboundRate, maxOutboundRate);
		return;
	}

//	LOG("Received FlowControl message. Adjusting OutRate from %d to %d msgs/sec.", datagramOutRatePerSecond, newOutboundRate);

	datagramOutRatePerSecond = newOutboundRate;
}

void MessageConnection::FreeOutboundPacketAckTrack(packet_id_t packetID)
{
	PacketAckTrackMap::iterator iter = outboundPacketAckTrack.find(packetID);
	if (iter == outboundPacketAckTrack.end())
	{
//		LOG("Tried to free OutboundPacketAckTrack with ID %d, but it doesn't exist!", (int)packetID);
		return;
	}

	PacketAckTrack &track = iter->second;
	for(size_t i = 0; i < track.messages.size(); ++i)
	{
		if (track.messages[i]->transfer)
		{
			Lock<FragmentedSendManager> sends = fragmentedSends.Acquire();
			sends->RemoveMessage(track.messages[i]->transfer, track.messages[i]);
		}

		FreeMessage(track.messages[i]);
	}

	outboundPacketAckTrack.erase(iter);
}

void MessageConnection::HandlePacketAckMessage(const char *data, size_t numBytes)
{
	if (numBytes != 7)
	{
		LOG("Malformed PacketAck message received! Size was %d bytes, expected 7 bytes!", numBytes);
		return;
	}

	DataDeserializer mr(data, numBytes);
	packet_id_t packetID = mr.Read<u8>() | (mr.Read<u16>() << 8);
	u32 sequence = mr.Read<u32>();

	FreeOutboundPacketAckTrack(packetID);
	for(size_t i = 0; i < 32; ++i)
		if ((sequence & (1 << i)) != 0)
		{
			packet_id_t id = AddPacketID(packetID, 1 + i);
			FreeOutboundPacketAckTrack(id);
		}
}

void MessageConnection::HandleDisconnectMessage()
{
	if (connectionState != ConnectionClosed)
	{
		connectionState = ConnectionDisconnecting;

		SendDisconnectAckMessage();

//		LOG("Received Disconnect message! Sending DisconnectAck.");
	}
}

void MessageConnection::HandleDisconnectAckMessage()
{
	if (connectionState != ConnectionDisconnecting)
		LOG("Received DisconnectAck message on a MessageConnection not in ConnectionDisconnecting state! (state was %d)",
		connectionState);
	else
		LOG("Connection closed to %s.", ToString().c_str());

	connectionState = ConnectionClosed;
	workThreadState = WorkThreadShouldQuit;
}

void MessageConnection::PerformFlowControl()
{
	// The manual flow control only applies to UDP connections.
	if (socket->TransportLayer() == SocketOverTCP)
		return;

	const float maxAllowedPacketLossRate = 0.f;
	if (GetPacketLossRate() > maxAllowedPacketLossRate)
	{
		float newInboundRate = PacketsInPerSec() * (1.f - GetPacketLossRate());
//		LOG("Packet loss rate: %.2f. Adjusting InRate from %d to %d!", GetPacketLossRate(), datagramInRatePerSecond, (int)newInboundRate);
		SetDatagramInFlowRatePerSecond((int)newInboundRate, true);
	}
	else if (PacketsInPerSec() >= (float)datagramInRatePerSecond / 2)
	{
		const int flowRateIncr = 50;
//		LOG("Have received %.2f packets in/sec with loss rate of %.2f. Increasing InRate from %d to %d.",
//			PacketsInPerSec(), GetPacketLossRate(), datagramInRatePerSecond, datagramInRatePerSecond + flowRateIncr);
		SetDatagramInFlowRatePerSecond(datagramInRatePerSecond + flowRateIncr, true);
	}
}

void MessageConnection::ComputePacketLoss()
{
	Lock<ConnectionStatistics> &cs = stats.Acquire();

	if (cs->recvPacketIDs.size() <= 1)
	{
		packetLossRate = packetLossCount = 0.f;
		return;
	}

	const clb::tick_t maxEntryAge = clb::Clock::TicksPerSec() * 5;
	const clb::tick_t timeNow = clb::Clock::Tick();
	const clb::tick_t maxTickAge = timeNow - maxEntryAge;

	// Remove old entries.
	for(size_t i = 0; i < cs->recvPacketIDs.size(); ++i)
		if (clb::Clock::IsNewer(cs->recvPacketIDs[i].tick, maxTickAge))
		{
			cs->recvPacketIDs.erase(cs->recvPacketIDs.begin(), cs->recvPacketIDs.begin() + i);
			break;
		}

	if (cs->recvPacketIDs.size() <= 1)
	{
		packetLossRate = packetLossCount = 0.f;
		return;
	}

	// Find the oldest packet (in terms of messageID)
	int oldestIndex = 0;
	for(size_t i = 1; i < cs->recvPacketIDs.size(); ++i)
		if (PacketIDIsNewerThan(cs->recvPacketIDs[oldestIndex].packetID, cs->recvPacketIDs[i].packetID))
			oldestIndex = i;

	std::vector<packet_id_t> relIDs;
	relIDs.reserve(cs->recvPacketIDs.size());
	for(size_t i = 0; i < cs->recvPacketIDs.size(); ++i)
		relIDs.push_back(SubPacketID(cs->recvPacketIDs[i].packetID, cs->recvPacketIDs[oldestIndex].packetID));

	clb::sort::CocktailSort(&relIDs[0], relIDs.size());

	int numMissedPackets = 0;
	for(size_t i = 0; i+1 < cs->recvPacketIDs.size(); ++i)
	{
		assert(relIDs[i+1] > relIDs[i]);
		numMissedPackets += relIDs[i+1] - relIDs[i] - 1;
	}
	
	float timespanMs = clb::Clock::TimespanToMilliseconds(maxTickAge, timeNow);
	int allPackets = cs->recvPacketIDs.size() + numMissedPackets;
	if ((timespanMs > 0.0f) && (allPackets))
    {
        packetLossRate = (float)numMissedPackets / allPackets;
        packetLossCount = (float)numMissedPackets * 1000.f / timespanMs;
    }
    else
    {
        packetLossRate = packetLossCount = 0.f;
        return;
    }
}

std::string MessageConnection::ToString() const
{
	if (socket)
		return socket->ToString();
	else
		return "(Not connected)";
}

void MessageConnection::DumpStatus() const
{
	char str[2048];

	sprintf(str, "Connection Status: %s.\n"
		"\tOutboundMessagesPending: %d.\n"
		"\tInboundPartialBytes: %d.\n"
			"\tConnected: %s.\n"
		"\tDatagram flow Inrate: %d/sec.\n"
		"\tDatagram flow Outrate: %d/sec.\n"
		"\tPacket loss count: %.2f.\n"
		"\tPacket loss rate: %.2f.\n"
		"\tRound-Trip Time: %.2fms.\n"
		"\tPing: %.2fms.\n"
		"\tLastHeardTime: %.2fms.\n"
		"\tDatagrams in: %.2f/sec.\n"
		"\tDatagrams out: %.2f/sec.\n"
		"\tMessages in: %.2f/sec.\n"
		"\tMessages out: %.2f/sec.\n"
		"\tBytes in: %s/sec.\n"
		"\tBytes out: %s/sec.\n",
		ConnectionStateToString(GetConnectionState()).c_str(),
		OutBoundMessagesPending(), InboundPartialBytes(), Connected() ? "true" : "false",
		GetDatagramInFlowRatePerSecond(), GetDatagramOutFlowRatePerSecond(),
		GetPacketLossCount(), GetPacketLossRate(), RoundTripTime(),
		Ping(), LastHeardTime(), PacketsInPerSec(), PacketsOutPerSec(),
		MsgsInPerSec(), MsgsOutPerSec(), FormatBytes((size_t)BytesInPerSec()).c_str(), FormatBytes((size_t)BytesOutPerSec()).c_str());

	LOG(str);
}

EndPoint MessageConnection::GetEndPoint() const
{
	assert(socket);
	return socket->GetEndPoint();
}
