// For conditions of distribution and use, see copyright notice in license.txt
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "Poco/Net/NetException.h"
#include "NetMessageManager.h"
#include "Poco/Net/DatagramSocket.h" // To get htons etc.
#include "ZeroCode.h"
#include "RexProtocolMsgIDs.h"

#include "curl/curl.h"
using namespace std;

NetMessageManager::NetMessageManager(const char *messageListFilename)
:messageList(boost::shared_ptr<NetMessageList>(new NetMessageList(messageListFilename)))
,inboundMessageListener(0), sequenceNumber(1)
{
	receivedSequenceNumbers.clear();
}

NetMessageManager::~NetMessageManager()
{
	for(std::list<NetOutMessage*>::iterator iter = unusedMessagePool.begin(); iter != unusedMessagePool.end(); ++iter)
		delete *iter;

	// We're supposed to free up all of our memory, but someone's using it!
	assert(usedMessagePool.size() == 0 && "Warning! Unsafe teardown of NetMessageManager detected!");
	for(std::list<NetOutMessage*>::iterator iter = usedMessagePool.begin(); iter != usedMessagePool.end(); ++iter)
		delete *iter;

	for(MessageResendList::iterator iter = messageResendQueue.begin(); iter != messageResendQueue.end(); ++iter)
		delete iter->second;
}

/* For reference, here's how the message looks:
struct UDPMessageHeader
{
	uint8_t flags;
	uint32_t sequenceNumber;
	uint8_t extraHeaderSize;
	// (extraHeaderSize bytes of extra header data.)
	// 1,2 or 4 bytes of message number information.
	// Message body.
	// Appended acks.
};
*/

/// This function skips the message header from the given data packet.
/// @param data A pointer to the message data.
/// @param numBytes The size of data, in bytes.
/// @param messageLength [out] The remaining length of the buffer is returned here, in bytes.
/// @return A pointer to the start of the message body, or 0 if the size of the body is 0 bytes.
static uint8_t *ComputeMessageBodyStartAddrAndLength(uint8_t *data, size_t numBytes, size_t *messageLength)
{
	// Too small buffer, no body existing at all.
	if (numBytes < 6)
	{
		if (messageLength)
			*messageLength = 0;
		return 0;
	}
	// There's a variable-sized extra header we need to skip. How many bytes is it?
	const uint8_t extraHeaderSize = data[5];

	// Then there's a VLE-encoded packetNumber (1,2 or 4 bytes) we need to skip: 
	// Defend against past-buffer read for malformed packets: What is the max size the packetNumber can be in bytes?
	int maxMsgNumBytes = min((int)numBytes - 6 - extraHeaderSize, 4); 
	if (maxMsgNumBytes <= 0) // Packet too short.
	{
		if (messageLength)
			*messageLength = 0;
		return 0;
	}
	// packetNumber is one byte long.
	if (maxMsgNumBytes >= 1 && data[6 + extraHeaderSize] != 0xFF)
	{
		if (messageLength)
			*messageLength = numBytes - 6 - extraHeaderSize - 1;
		return *messageLength > 0 ? data + 6 + extraHeaderSize + 1 : 0;
	}
	// packetNumber is two bytes long.
	if (maxMsgNumBytes >= 2 && data[6 + extraHeaderSize + 1] != 0xFF)
	{
		if (messageLength)
			*messageLength = numBytes - 6 - extraHeaderSize - 2;
		return *messageLength > 0 ? data + 6 + extraHeaderSize + 2 : 0;
	}
	// packetNumber is four bytes long.
	if (maxMsgNumBytes >= 4)
	{
		if (messageLength)
			*messageLength = numBytes - 6 - extraHeaderSize - 4;
		return *messageLength > 0 ? data + 6 + extraHeaderSize + 4 : 0;
	}

	// The packet was malformed if we get here.
	if (messageLength)
		*messageLength = 0;
	return 0;
}

/// const version of above.
static const uint8_t *ComputeMessageBodyStartAddrAndLength(const uint8_t *data, size_t numBytes, size_t *messageLength)
{
	return ComputeMessageBodyStartAddrAndLength(const_cast<uint8_t *>(data), numBytes, messageLength);
}

/// Reads the message number from the given byte stream that represents a message.
/// @param data Pointer to the start of the message data (to first byte of header).
/// @param numBytes The size of the message (including headers).
/// @return The message number, or 0 to denote an invalid message.
static NetMsgID ExtractNetworkMessageNumber(const uint8_t *data, size_t numBytes)
{
	assert(data || numBytes == 0);

	if (numBytes < 6)
		return 0;
	uint8_t extraHeaderSize = data[5];

	// Defend against past-buffer read for malformed packets: What is the max size the packetNumber can be in bytes?
	int maxMsgNumBytes = min((int)numBytes - 6 - extraHeaderSize, 4); 
	if (maxMsgNumBytes <= 0)
		return 0;
	if (maxMsgNumBytes >= 1 && data[6 + extraHeaderSize] != 0xFF)
		return data[6 + extraHeaderSize];
	if (maxMsgNumBytes >= 2 && data[6 + extraHeaderSize + 1] != 0xFF)
		return ntohs(*(u_short*)&data[6 + extraHeaderSize]);
	if (maxMsgNumBytes >= 4)
		return ntohl(*(u_long*)&data[6 + extraHeaderSize]);

	return 0;
}

/// Reads the packet sequence number from the given byte stream that represents a message.
/// @param data Pointer to the start of the message data (to first byte of header).
/// @param numBytes The size of the message (including headers).
/// @return The message sequence number, or 0 to denote an error occurred.
static uint32_t ExtractNetworkMessageSequenceNumber(const uint8_t *data, size_t numBytes)
{	
	assert(data || numBytes == 0);

	if (numBytes < 6)
		return 0;

   return (uint32_t)ntohl(*(u_long*)&data[1]);//((data[1] << 24) + (data[2] << 16) + (data[3] << 8) + data[4]);	
}

const char *VariableTypeToStr(NetVariableType type)
{
	const char *data[] = { "Invalid", "U8", "U16", "U32", "U64", "S8", "S16", "S32", "S64", "F32", "F64", "LLVector3", "LLVector3d", "LLVector4",
	                       "LLQuaternion", "UUID", "BOOL", "IPADDR", "IPPORT", "Fixed", "Variable", "BufferByte", "Buffer2Bytes", "Buffer4Bytes" };
	if (type < 0 || type >= NUMELEMS(data))
		return data[0];

	return data[type];
}

void NetMessageManager::DumpNetworkMessage(NetMsgID id, NetInMessage *msg)
{
	const NetMessageInfo *info = msg->GetMessageType();
	if (!info)
	{
		std::cout << "Packet with invalid ID received: 0x" << std::hex << id << "." << std::endl;
		return;
	}

	size_t dataSize = msg->GetDataSize();
	
	std::cout << "\"" << info->name << "\" length " << dataSize << " bytes:" << std::endl;

	int prevBlock = msg->GetCurrentBlock();
	while(msg->GetCurrentBlock() < msg->GetBlockCount())
	{
		int curBlock = msg->GetCurrentBlock();
		const NetMessageBlock &block = info->blocks[curBlock];
		size_t blockInstanceCount = msg->ReadCurrentBlockInstanceCount();
		const NetMessageVariable &var = block.variables[msg->GetCurrentVariable()];

		if (curBlock != prevBlock || (curBlock == 0 && prevBlock == 0))
			std::cout << "  Block \"" << block.name << "\":" << std::endl;
		prevBlock = curBlock;
		
		NetVariableType varType = msg->CheckNextVariableType();
		size_t varSize = msg->ReadVariableSize();
		const uint8_t* data = (const uint8_t*)msg->ReadBytesUnchecked(varSize);
		msg->SkipToNextVariable(true);

		stringstream varData;
		bool bMalformed = false;

		if (data && varSize > 0)
			for(size_t k = 0; k < varSize && k < 15; ++k) // Print only first 15 bytes of data.
				varData << hex << (int)data[k] << " ";
		else
		{
			varData << "Size: " << varSize << " but packet was malformed!";
			bMalformed = true;
		}

		std::cout << "      Var \"" << var.name << "\"(" << VariableTypeToStr(varType)<< "):" << varData.str() << std::endl;

		if(bMalformed) return;
	}
}

///\todo REMOVE
void NetMessageManager::DumpNetworkMessage(const uint8_t *data, size_t numBytes)
{
	NetMsgID id = ExtractNetworkMessageNumber(data, numBytes);
	const NetMessageInfo *info = messageList->GetMessageInfoByID(id);
	if (!info)
	{
		std::cout << "Packet with invalid ID received: 0x" << std::hex << id << ". Size was " << numBytes << " bytes." << std::endl;
		return;
	}

	size_t messageLength = 0;
	size_t msgInd = 0;
	const uint8_t *message = ComputeMessageBodyStartAddrAndLength(data, numBytes, &messageLength);
		
	std::cout << "Message \"" << info->name << "\" length " << messageLength << " bytes:" << std::endl;
	for(size_t i = 0; i < info->blocks.size(); ++i)
	{
		size_t repeatCounter = 0;
		const NetMessageBlock &block = info->blocks[i];
		if (block.type == NetBlockMultiple)
			repeatCounter = block.repeatCount;

		std::cout << "  Block \"" << block.name << "\":" << std::endl;
		for(size_t j = 0; j < block.variables.size(); ++j)
		{	
			const NetMessageVariable &var = block.variables[j];
			const size_t &varSize = NetVariableSizes[var.type];
			const char *varStr = VariableTypeToStr(var.type);

			//void *varDataBuf = malloc(varSize);
			//memcpy(varDataBuf, &message[msgInd], varSize);
			
			//std::cout << "    " << msgInd << ":" << var.name << "(" << varStr << ", " << varSize << " bytes):" << std::endl;
			/*if(var.type == NetVarU8) {uint8_t *varData = (uint8_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarU16){uint16_t *varData = (uint16_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarU32){uint32_t *varData = (uint32_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarU64){uint64_t *varData = (uint64_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarS8){int8_t *varData = (int8_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarS16){int16_t *varData = (int16_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarS32){int32_t *varData = (int32_t *)varDataBuf; std::cout << varData << std::endl;}
			if(var.type == NetVarS64){int64_t *varData = (int64_t *)varDataBuf; std::cout << varData << std::endl;}*/

			msgInd += varSize;

			if (msgInd > messageLength)
			{
				std::cout << "Warning! Message index (" << msgInd << ") " << "exceeded the message length (" << messageLength << ")" << std::endl;
				break;
			}
			
		}
		if (repeatCounter > 0 && (block.type == NetBlockMultiple || block.type == NetBlockMultiple))
		{
			--repeatCounter;
			std::cout << "Repeating block " << block.name << " for " << repeatCounter << " more times. " << std::endl;
			continue;
		}
	}
}

/// Polls the inbound socket until the message queue is empty. Also resends any timed out reliable messages.
void NetMessageManager::ProcessMessages()
{
	if (!ResendQueueIsEmpty())
		ProcessResendQueue();

	while(connection->PacketsAvailable())
	{
		const int cMaxPayload = 2048;
		std::vector<uint8_t> data(cMaxPayload, 0);
		int numBytes = connection->ReceiveBytes(&data[0], cMaxPayload);
		
		if (!inboundMessageListener)
		{
			cout << "No UDP message listener set! Dropping incoming packet as unhandled:" << endl;
			DumpNetworkMessage(&data[0], numBytes);
			continue;
		}

		NetMsgID id = ExtractNetworkMessageNumber(&data[0], numBytes);
		uint32_t seqNum = ExtractNetworkMessageSequenceNumber(&data[0], numBytes);
		
		// We need to do pruning of inbound duplicates, so add the sequence number to the set of received sequence numbers, 
		// and check if we've seen this packet before.
		pair<set<uint32_t>::iterator, bool> ret = receivedSequenceNumbers.insert(seqNum);
		if (ret.second == false) 
			continue; // A message with this sequence number has already been given to the application for processing. Drop it this time.

		// Send ACK for reliable messages.
		if ((data[0] & NetFlagReliable) != 0)
			SendPacketACK(seqNum);
	
		size_t messageLength = 0;
		const uint8_t *message = ComputeMessageBodyStartAddrAndLength(&data[0], numBytes, &messageLength);
		NetInMessage msg(messageList->GetMessageInfoByID(id), &message[0], messageLength, (data[0] & NetFlagZeroCode) != 0);
			
		// NetMessageManager handles all Acks and Pings. Those are not passed to the application.
		switch(id)
		{
		case RexNetMsgPacketAck:
			ProcessPacketACK(&msg);
			break;
		case RexNetMsgStartPingCheck:
			SendCompletePingCheck(msg.ReadU8());
			break;
		default:
			// Pass the message to application.
			inboundMessageListener->OnNetworkMessageReceived(id, &msg);
			break;
		}
	}

	// To keep memory footprint down and to defend against memory attacks, keep the list of seen sequence numbers to a fixed size.
	const size_t cMaxSeqNumMemorySize = 300;
	while(receivedSequenceNumbers.size() > cMaxSeqNumMemorySize)
		receivedSequenceNumbers.erase(receivedSequenceNumbers.begin()); // We remove from the front to guarantee the smallest(oldest) are removed first.
}

void NetMessageManager::ConnectTo(const char *serverAddress, int port)
{
	try
	{
		connection = boost::shared_ptr<NetworkConnection>(new NetworkConnection(serverAddress, port));
	} catch(Poco::Net::NetException &e)
	{
		std::cout << "Failed to connect to " << serverAddress << ":" << port << ". Error: " << e.message() << std::endl;
	}
}

void NetMessageManager::Disconnect()
{
	connection.reset();
	curl_global_cleanup();
}

NetOutMessage *NetMessageManager::StartNewMessage(NetMsgID id)
{
	const NetMessageInfo *info = messageList->GetMessageInfoByID(id);
	if (!info) 
		return 0;

	NetOutMessage *newMsg = 0;
	if (unusedMessagePool.size() > 0)
	{
		newMsg = unusedMessagePool.front();
		unusedMessagePool.pop_front();
	}
	else
		newMsg = new NetOutMessage();

	newMsg->SetMessageType(info);
	newMsg->AddMessageHeader();
	usedMessagePool.push_back(newMsg);
	
	return newMsg;
}

void NetMessageManager::FinishMessage(NetOutMessage *message)
{
	assert(message);
	
	message->SetSequenceNumber(GetNewSequenceNumber());

	// Try to Zero-encode the message if that is desired. If encoding worsens the size, we'll send unencoded.
	if (message->GetMessageInfo()->encoding == NetZeroEncoded)
	{
		std::vector<uint8_t> &data = message->GetData();
		if (data.size() == 0)
			return;
		assert(data.size() >= message->BytesFilled());
		data.resize(message->BytesFilled());

		size_t bodyLength = 0;
		const uint8_t *bodyData = ComputeMessageBodyStartAddrAndLength(&data[0], message->BytesFilled(), &bodyLength);
		assert(bodyLength < message->BytesFilled());
		size_t headerLength = message->BytesFilled() - bodyLength;

		size_t encodedBodyLength = CountZeroEncodedLength(bodyData, bodyLength);
		std::vector<uint8_t> zeroCodedData;
		zeroCodedData.resize(headerLength + encodedBodyLength, 0);

		// If the encoded message would take more space than the non-coded, just send non-coded.
		if (headerLength + encodedBodyLength >= message->BytesFilled()) 
		{
			data[0] &= ~NetFlagZeroCode;
		}
		else // Send out zerocoded, it's actually compressed something.
		{
			data[0] |= NetFlagZeroCode;

			memcpy(&zeroCodedData[0], &data[0], headerLength);
			ZeroEncode(&zeroCodedData[headerLength], encodedBodyLength, bodyData, bodyLength);
			data = zeroCodedData;
		}
	}

	// Push reliable messages to queue to wait ACK from the server.
	if (message->IsReliable())
		AddMessageToResendQueue(message);

	SendProcessedMessage(message);
}

void NetMessageManager::SendProcessedMessage(NetOutMessage *msg)
{
	assert(msg);

	std::vector<uint8_t> &data = msg->GetData();
	connection->SendBytes(&data[0], data.size());
}

///\todo Send multiple ACKs in one packet?
void NetMessageManager::SendPacketACK(uint32_t packetID)
{
	NetOutMessage *m = StartNewMessage(RexNetMsgPacketAck);
	assert(m);
	m->SetVariableBlockCount(1);
	m->AddU32(packetID);
	FinishMessage(m);
}

void NetMessageManager::ProcessPacketACK(NetInMessage *msg)
{
	size_t blockCount = msg->ReadCurrentBlockInstanceCount();
	for(size_t i = 0; i < blockCount; ++i)
	{
		uint32_t id = msg->ReadU32();
		std::cout << "Received ACK for packet " << id  << std::endl;
		RemoveMessageFromResendQueue(id);
	}
}

void NetMessageManager::SendCompletePingCheck(uint8_t pingID)
{
	NetOutMessage *m = StartNewMessage(RexNetMsgCompletePingCheck);
	assert(m);
	m->AddU8(pingID);
	FinishMessage(m);
}

/// A unary find predicate that looks for a NetOutMessage that has the given desired sequence number in a resendqueue container.
class MsgSeqNumMatchPred
{
public:
	MsgSeqNumMatchPred(uint32_t seqNum_):seqNum(seqNum_) {}

	bool operator()(const std::pair<time_t, NetOutMessage*> &elem) const { return elem.second && elem.second->GetSequenceNumber() == seqNum; }
private:
	uint32_t seqNum;
};

void NetMessageManager::AddMessageToResendQueue(NetOutMessage *msg)
{
	// Don't add duplicates (timeouted and resent messages).
	MessageResendList::iterator it = std::find_if(messageResendQueue.begin(), messageResendQueue.end(), MsgSeqNumMatchPred(msg->GetSequenceNumber()));
	if (it != messageResendQueue.end())
		return;

	messageResendQueue.push_back(std::make_pair(time(NULL), msg));
}

void NetMessageManager::RemoveMessageFromResendQueue(uint32_t packetID)
{
	MessageResendList::iterator it = std::find_if(messageResendQueue.begin(), messageResendQueue.end(), MsgSeqNumMatchPred(packetID));

	if (it != messageResendQueue.end())
		messageResendQueue.erase(it);
}

void NetMessageManager::ProcessResendQueue()
{
	const int cTimeoutSeconds = 5;

	const time_t timeNow = time(NULL);
	for(MessageResendList::iterator it = messageResendQueue.begin(); it != messageResendQueue.end(); ++it)
	{
		if (timeNow - it->first >= cTimeoutSeconds)
		{
			it->first = timeNow;
			SendProcessedMessage(it->second);
		}
	}
}

#ifndef RELEASE

void NetMessageManager::DebugSendHardcodedTestPacket()
{
	const uint8_t data[] =
	{ 0x40, 0x00, 0x00, 0x00, 0x01, 0x00, 0xff, 0xff, 0x00, 0x03, 0x37, 0x2d, 0x5e, 0x0d, 0xde, 0xc9, 0x50, 
	  0xb7, 0x40, 0x2d, 0xa2, 0x23, 0xc1, 0xae, 0xe0, 0x7e, 0x35, 0xe6, 0x8e, 0x5b, 0x78, 0x42, 0xac, 0x6d, 
	  0x11, 0x67, 0x47, 0xde, 0x91, 0xf3, 0x58, 0xa9, 0x65, 0x49, 0xb0, 0xf0 };

	connection->SendBytes(data, NUMELEMS(data));
}

void NetMessageManager::DebugSendHardcodedRandomPacket(size_t numBytes)
{
	if (numBytes == 0)
		return;

	std::vector<uint8_t> data(numBytes, 0);
	for(size_t i = 0; i < numBytes; ++i)
		data[i] = rand() & 0xFF;

	connection->SendBytes(&data[0], numBytes);
}

#endif
