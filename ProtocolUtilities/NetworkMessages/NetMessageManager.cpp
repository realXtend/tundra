// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <boost/timer.hpp>

#include "DebugOperatorNew.h"

#include "Poco/Net/NetException.h"
#include "Poco/Net/DatagramSocket.h" // To get htons etc.

#include "NetMessageManager.h"
#include "ZeroCode.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "Profiler.h"

using namespace std;

namespace ProtocolUtilities
{

    /* For reference, here's how an SLUDP packet frame looks like:
    struct UDPMessagePacket
    {
        uint8_t flags;
        uint32_t sequenceNumber;
        uint8_t extraHeaderSize;
        byte extraHeaderData[extraHeaderSize];  // Not currently parsed at all.
        // Message body. (possibly zerocoded)
        // Appended acks.

        // The format for message body:
        // u8/u16/u32 messageNumber; // Variable-length -encoded.
        // Message content.          // Serialized data, the format is described by the SLUDP message_template.msg
    };
    */

    /// This function skips the message header from the given data packet.
    /// @param data A pointer to the message data.
    /// @param numBytes The size of data, in bytes.
    /// @param messageLength [out] The remaining length of the buffer is returned here, in bytes.
    /// @return A pointer to the start of the message body, or 0 if the size of the body is 0 bytes or if the message was malformed.
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

        if (messageLength)
        {
            *messageLength = numBytes - 6 - extraHeaderSize;
            
            // If there's pending acks, remove them from message length
            if (data[0] & NetFlagAck)
            {
                size_t acksize = 1 + data[numBytes-1] * 4;
                // Malformed?
                if (*messageLength < acksize)
                {
                    *messageLength = 0;
                    return 0; 
                }
                *messageLength -= acksize;
            }
        }
        
        return data + 6 + extraHeaderSize;
    }

    /// Returns a list of appended acks from packet
    /// @param data A pointer to the message data.
    /// @param numBytes The size of data, in bytes.
    static std::vector<uint32_t> GetAppendedAckList(uint8_t *data, size_t numBytes)
    {
        std::vector<uint32_t> acks;
        if ((data[0] & NetFlagAck) && (numBytes > 6))
        {
            if (numBytes > 6)
            {
                size_t num_acks = data[numBytes-1];
                acks.resize(num_acks);
                
                int idx = numBytes - 1 - num_acks * 4;
                if (idx < 6) return acks;
                
                for (size_t i = 0; i < num_acks; ++i)
                {
                    acks.push_back((uint32_t)ntohl(*(u_long*)&data[idx]));
                }
            }
        }
        return acks;
    }

    /// const version of above.
    /*
    static const uint8_t *ComputeMessageBodyStartAddrAndLength(const uint8_t *data, size_t numBytes, size_t *messageLength)
    {
        return ComputeMessageBodyStartAddrAndLength(const_cast<uint8_t *>(data), numBytes, messageLength);
    }
    */

    /// Reads the packet sequence number from the given byte stream that represents an SLUDP packet.
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

    NetMessageManager::NetMessageManager(const char *messageListFilename)
    :messageList(boost::shared_ptr<NetMessageList>(new NetMessageList(messageListFilename)))
    ,messageListener(0), 
    sequenceNumber(1), // Note here: We always start outbound communication with PacketID==1.
    lastReceivedSequenceNumber(0)
#ifdef PROFILING
    ,sentDatagrams(65536)
    ,sentDatabytes(65536)
    ,receivedDatagrams(65536)
    ,receivedDatabytes(65536)
    ,resentPackets(65536)
    ,lostPackets(65536)
    ,duplicatesReceived(65536)
#endif
    {      
        receivedSequenceNumbers.clear();        
    }

    NetMessageManager::~NetMessageManager()
    {
        ClearMessagePoolMemory();            
        receivedSequenceNumbers.clear();
    }

    void NetMessageManager::DumpNetworkMessage(NetMsgID id, NetInMessage *msg)
    {
        const NetMessageInfo *info = msg->GetMessageInfo();
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

    const NetMessageInfo *NetMessageManager::GetMessageInfoByID(NetMsgID id) const
    {
        return messageList->GetMessageInfoByID(id);
    }

    /// Polls the inbound socket until the message queue is empty. Also resends any timed out reliable messages.
    void NetMessageManager::ProcessMessages()
    {
        PROFILE (NetMessageManager_ProcessMessages);
        if (!connection)
            return;
            
        if (!ResendQueueIsEmpty())
            ProcessResendQueue();
        
        // Process network messages for max. 0.1 seconds, to prevent lack of rendering/mainloop execution during heavy processing
        static const double MAX_PROCESS_TIME = 0.1;
        boost::timer timer;
        
        PROFILE(NetMessageManager_WhilePacketsAvailable);
        while(connection->PacketsAvailable() && timer.elapsed() < MAX_PROCESS_TIME)
        {
            const int cMaxPayload = 2048;
            std::vector<uint8_t> data(cMaxPayload, 0);
            int numBytes = connection->ReceiveBytes(&data[0], cMaxPayload);
            
            if (numBytes == 0)
                break;

#ifdef PROFILING
            receivedDatagrams.InsertRecord(1.0);
            receivedDatabytes.InsertRecord(numBytes);
#endif

            if (!messageListener)
            {
                cout << "No UDP message listener set! Dropping incoming packet as unhandled:" << endl;
    //            DumpNetworkMessage(&data[0], numBytes);
                continue;
            }

            uint32_t seqNum = ExtractNetworkMessageSequenceNumber(&data[0], numBytes);

#ifdef PROFILING
            if (receivedSequenceNumbers.size() > 0 && seqNum - lastReceivedSequenceNumber < 16)
            {
                for(int i = lastReceivedSequenceNumber+1; i < seqNum; ++i)
                    if (receivedSequenceNumbers.find(i) == receivedSequenceNumbers.end())
                        lostPackets.InsertRecord(1.0);
            }
#endif
            lastReceivedSequenceNumber = seqNum;

            // Send ACK for reliable messages.
            if ((data[0] & NetFlagReliable) != 0)
                QueuePacketACK(seqNum);

            // We need to do pruning of inbound duplicates, so add the sequence number to the set of received sequence numbers, 
            // and check if we've seen this packet before.
            pair<set<uint32_t>::iterator, bool> ret = receivedSequenceNumbers.insert(seqNum);
            if (ret.second == false) 
            {
#ifdef PROFILING
                duplicatesReceived.InsertRecord(1.0);
#endif
                continue; // A message with this sequence number has already been given to the application for processing. Drop it this time.
            }

    //        NetMsgID id = ExtractNetworkMessageNumber(&data[0], numBytes);

            size_t messageLength = 0;
            const uint8_t *message = ComputeMessageBodyStartAddrAndLength(&data[0], numBytes, &messageLength);
            if (!message)
            {
                cout << "Malformed packet received, could not determine message size" << endl;
                continue;
            }
            
            std::vector<uint32_t> appended_acks = GetAppendedAckList(&data[0], numBytes);
            
            NetInMessage msg(seqNum, &message[0], messageLength, (data[0] & NetFlagZeroCode) != 0);
            const NetMessageInfo *messageInfo = messageList->GetMessageInfoByID(msg.GetMessageID());
            if (!messageInfo)
            {
                cout << "Unknown message received with Message ID " << msg.GetMessageID() << "!" << endl;
                continue;
            }
            msg.SetMessageInfo(messageInfo);

            // Process appended acks
            if (appended_acks.size() > 0)
            {
                for (unsigned i = 0; i < appended_acks.size(); ++i)
                    ProcessPacketACK(appended_acks[i]);
            }
            
            // NetMessageManager handles all Acks and Pings. Those are not passed to the application.
            switch(msg.GetMessageID())
            {
            case RexNetMsgPacketAck:
                ProcessPacketACK(&msg);
                break;
            case RexNetMsgStartPingCheck:
                SendCompletePingCheck(msg.ReadU8());
                break;
            default:
                // Pass the message to the listener(s).
                messageListener->OnNetworkMessageReceived(msg.GetMessageID(), &msg);
                break;
            }
        }
        
        if (!connection->Open())
            connection.reset();
            
        // To keep memory footprint down and to defend against memory attacks, keep the list of seen sequence numbers to a fixed size.
        const size_t cMaxSeqNumMemorySize = 300;
        while(receivedSequenceNumbers.size() > cMaxSeqNumMemorySize)
            receivedSequenceNumbers.erase(receivedSequenceNumbers.begin()); // We remove from the front to guarantee the smallest(oldest) are removed first.

        // Acknowledge all the new accumulated packets that the server sent as reliable.
        SendPendingACKs();
    }

    bool NetMessageManager::ConnectTo(const char *serverAddress, int port)
    {
        try
        {
            connection = boost::shared_ptr<NetworkConnection>(new NetworkConnection(serverAddress, port));
            return true;
        } catch(Poco::Net::NetException &e)
        {
            std::cout << "Failed to connect to " << serverAddress << ":" << port << ". Error: " << e.message() << std::endl;
            return false;
        }
    }

    void NetMessageManager::Disconnect()
    {
        connection->Close();
        ClearMessagePoolMemory();
        receivedSequenceNumbers.clear();
    }

    NetOutMessage *NetMessageManager::StartNewMessage(NetMsgID id)
    {
        // There should exist a message 'template' with the given ID in the message info list. If not,
        // we can't start crafting a new message and must fail. See message_template.msg file for a list
        // of all the protocol messages supported by the system.
        const NetMessageInfo *info = messageList->GetMessageInfoByID(id);
        if (!info) 
            return 0;

        NetOutMessage *newMsg = 0;

        // Find if we have an old message struct in the unused pool that we can use.
        if (unusedMessagePool.size() > 0)
        {
            newMsg = unusedMessagePool.front();
            unusedMessagePool.pop_front();
            newMsg->ResetWriting();
        }
        else
            newMsg = new NetOutMessage();

        newMsg->SetMessageInfo(info);
        newMsg->AddMessageHeader();
        usedMessagePool.push_back(newMsg);
        
        return newMsg;
    }

    void NetMessageManager::FinishMessage(NetOutMessage *message)
    {
        assert(message);
        message->SetSequenceNumber(GetNewSequenceNumber());

        std::vector<uint8_t> &data = message->GetData();
        if (data.size() == 0)
        {
            unusedMessagePool.push_back(message);
            return;
        }
        assert(data.size() >= message->BytesFilled());
        data.resize(message->BytesFilled());

        // Find and remove the given message from the usedMessagePool list, it has to be there.
#ifdef _DEBUG
        const size_t usedMessagePoolSize = usedMessagePool.size();
#endif
        std::list<NetOutMessage*>::iterator newEnd = std::remove(usedMessagePool.begin(), usedMessagePool.end(), message);
        usedMessagePool.erase(newEnd, usedMessagePool.end());
#ifdef _DEBUG
        assert(usedMessagePoolSize == usedMessagePool.size() + 1);
#endif
        
        // Try to Zero-encode the message if that is desired. If encoding worsens the size, we'll send unencoded.
        if (message->GetMessageInfo()->encoding == NetZeroEncoded)
        {
            size_t bodyLength = 0;
            const uint8_t *bodyData = ComputeMessageBodyStartAddrAndLength(&data[0], message->BytesFilled(), &bodyLength);
            assert(bodyLength < message->BytesFilled());
            size_t headerLength = message->BytesFilled() - bodyLength;

            size_t encodedBodyLength = CountZeroEncodedLength(bodyData, bodyLength);

            // If the encoded message would take more space than the non-coded, just send non-coded.
            if (headerLength + encodedBodyLength >= message->BytesFilled()) 
            {
                data[0] &= ~NetFlagZeroCode;
            }
            else // Send out zerocoded, it's actually compressed something.
            {
                data[0] |= NetFlagZeroCode;

                std::vector<uint8_t> zeroCodedData;
                zeroCodedData.resize(headerLength + encodedBodyLength, 0);
                memcpy(&zeroCodedData[0], &data[0], headerLength);
                ZeroEncode(&zeroCodedData[headerLength], encodedBodyLength, bodyData, bodyLength);
                data = zeroCodedData;
            }
        }

        SendProcessedMessage(message);

        // Push reliable messages to queue to wait ACK from the server.
        if (message->IsReliable())
            AddMessageToResendQueue(message);
        else
            unusedMessagePool.push_back(message);
    }

    void NetMessageManager::SendProcessedMessage(NetOutMessage *msg)
    {
        assert(msg);

        std::vector<uint8_t> &data = msg->GetData();
        assert(data.size() > 0);
        connection->SendBytes(&data[0], data.size());

#ifdef PROFILING
        sentDatagrams.InsertRecord(1.0);
        sentDatabytes.InsertRecord(data.size());
#endif

        if (messageListener)
            messageListener->OnNetworkMessageSent(msg);
    }

    void NetMessageManager::QueuePacketACK(uint32_t packetID)
    {
        pendingACKs.insert(packetID);
    }

    void NetMessageManager::ClearMessagePoolMemory()
    {
        for(std::list<NetOutMessage*>::iterator iter = unusedMessagePool.begin(); iter != unusedMessagePool.end(); ++iter)
            delete *iter;

        // We're supposed to free up all of our memory, but someone's using it!
        assert(usedMessagePool.size() == 0 && "Warning! Unsafe teardown of NetMessageManager detected!");
        for(std::list<NetOutMessage*>::iterator iter = usedMessagePool.begin(); iter != usedMessagePool.end(); ++iter)
            delete *iter;

        for(MessageResendList::iterator iter = messageResendQueue.begin(); iter != messageResendQueue.end(); ++iter)
            delete iter->second;

        unusedMessagePool.clear();
        usedMessagePool.clear();
        messageResendQueue.clear();
    }

    ///\todo Have better delay method for pending ACKs, currently sends everything accumulated just over one frame
    void NetMessageManager::SendPendingACKs()
    {
        PROFILE(NetMessageManager_SendPendingACKs);
        // If we aren't even connected (or not connected anymore), clear any old pending ACKs and return.
        if (!connection.get())
        {
            pendingACKs.clear();
            return;
        }

        static const size_t max_acks_in_msg = 100;

        while (pendingACKs.size() > 0)
        {
            size_t acks_to_send = pendingACKs.size();
            if (acks_to_send > max_acks_in_msg)
                acks_to_send = max_acks_in_msg;

            NetOutMessage *m = StartNewMessage(RexNetMsgPacketAck);
            assert(m);
            m->SetVariableBlockCount(acks_to_send);
            
            std::set<uint32_t>::iterator i = pendingACKs.begin();
            size_t added_acks = 0;
            
            while (added_acks < acks_to_send)
            {
                // Note! Horrible protocol design issue! The sequence numbers that both
                // server and client use are sent in big endian, but in the ACK packets
                // they need to be transferred in little endian. !! So, no conversion to
                // big endian here.
                m->AddU32(*i);
                ++added_acks;
                ++i;
            }
            
            FinishMessage(m);
            
            pendingACKs.erase(pendingACKs.begin(), i);
        }
    }

    void NetMessageManager::ProcessPacketACK(NetInMessage *msg)
    {
        size_t blockCount = msg->ReadCurrentBlockInstanceCount();
        for(size_t i = 0; i < blockCount; ++i)
            ProcessPacketACK(msg->ReadU32());
    }

    void NetMessageManager::ProcessPacketACK(uint32_t id)
    {
        //std::cout << "Received ACK for packet " << id  << std::endl;
        RemoveMessageFromResendQueue(id);
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
        // Don't add this message to the queue, if it already exists in the queue, i.e. it has already been resent once due to a timeout.
        MessageResendList::iterator it = std::find_if(messageResendQueue.begin(), messageResendQueue.end(), MsgSeqNumMatchPred(msg->GetSequenceNumber()));
        if (it != messageResendQueue.end())
        {
            // If the sequence numbers matched but these are different message structs, add the message to unusedMessagePool, it's extraneous.
            if (it->second != msg)
                unusedMessagePool.push_back(msg);
            return;
        }

        messageResendQueue.push_back(std::make_pair(time(0), msg));
    }

    void NetMessageManager::RemoveMessageFromResendQueue(uint32_t packetID)
    {
        MessageResendList::iterator it = std::find_if(messageResendQueue.begin(), messageResendQueue.end(), MsgSeqNumMatchPred(packetID));

        if (it != messageResendQueue.end())
        {
            unusedMessagePool.push_back(it->second);
            messageResendQueue.erase(it);
        }
    }

    void NetMessageManager::ProcessResendQueue()
    {
        PROFILE(NetMessageManager_ProcessResendQueue);
        const int cTimeoutSeconds = 5;

        const time_t timeNow = time(0);
        for(MessageResendList::iterator it = messageResendQueue.begin(); it != messageResendQueue.end(); ++it)
        {
            if (timeNow - it->first >= cTimeoutSeconds)
            {
                it->first = timeNow;
                it->second->MarkResend();
                SendProcessedMessage(it->second);
                //std::cout << "Resending packet " << it->second->GetSequenceNumber() << std::endl;
#ifdef PROFILING
                resentPackets.InsertRecord(1.0);
#endif
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

}

