// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <iostream>

#include "Poco/Net/DatagramSocket.h" // To get htons etc.

#include "NetInMessage.h"
#include "ZeroCode.h"

#include "QuatUtils.h"
#include "RexUUID.h"

using namespace RexTypes;

#undef min

namespace ProtocolUtilities
{

/// Reads the message number from the given byte stream that represents an SLUDP message body.
/// @param data Pointer to the start of the message body. See NetMessageManager.cpp for a detailed description of the structure.
/// @param numBytes The number of bytes in data.
/// @param [out] messageIDLength The number of bytes taken by the VLE-encoding of the message ID.
/// @return The message number, or 0 to denote an invalid message.
static NetMsgID ExtractNetworkMessageID(const uint8_t *data, size_t numBytes, size_t *messageIDLength)
{
    assert(data || numBytes == 0);
    assert(messageIDLength);

    // Defend against past-buffer read for malformed packets: What is the max size the packetNumber can be in bytes?
    int maxMsgNumBytes = std::min((int)numBytes, 4); 
    if (maxMsgNumBytes <= 0)
    {
        *messageIDLength = 0;
        return 0;
    }
    if (maxMsgNumBytes >= 1 && data[0] != 0xFF)
    {
        *messageIDLength = 1;
        return data[0];
    }
    if (maxMsgNumBytes >= 2 && data[1] != 0xFF)
    {
        *messageIDLength = 2;
        return ntohs(*(u_short*)&data[0]);
    }
    if (maxMsgNumBytes >= 4)
    {
        *messageIDLength = 4;
        return ntohl(*(u_long*)&data[0]);
    }

    return 0;
}
/*
/// This function skips the messageNumber from the given message body buffer.
/// @param data A pointer to message body that has *already* been zerodecoded.
/// @param numBytes The number of bytes in the message body.
/// @param messageLength [out] The remaining length of the buffer is returned here, in bytes.
/// @return A pointer to the start of the message content, or 0 if the size of the body is 0 bytes or if the message was malformed.
static uint8_t *ComputeMessageContentStartAddrAndLength(uint8_t *data, size_t numBytes, size_t *messageLength)
{
    // Then there's a VLE-encoded packetNumber (1,2 or 4 bytes) we need to skip: 
    // Defend against past-buffer read for malformed packets: What is the max size the packetNumber can be in bytes?
    int maxMsgNumBytes = min((int)numBytes, 4); 
    if (maxMsgNumBytes <= 0) // Packet too short.
    {
        if (messageLength)
            *messageLength = 0;
        return 0;
    }
    // packetNumber is one byte long.
    if (maxMsgNumBytes >= 1 && data[0] != 0xFF)
    {
        if (messageLength)
            *messageLength = numBytes - 1;
        return *messageLength > 0 ? data + 1 : 0;
    }
    // packetNumber is two bytes long.
    if (maxMsgNumBytes >= 2 && data[1] != 0xFF)
    {
        if (messageLength)
            *messageLength = numBytes - 2;
        return *messageLength > 0 ? data + 2 : 0;
    }
    // packetNumber is four bytes long.
    if (maxMsgNumBytes >= 4)
    {
        if (messageLength)
            *messageLength = numBytes - 4;
        return *messageLength > 0 ? data + 4 : 0;
    }

    // The packet was malformed if we get here.
    if (messageLength)
        *messageLength = 0;
    return 0;
}
*/

NetInMessage::NetInMessage(size_t seqNum, const uint8_t *data, size_t numBytes, bool zeroCoded) :
    messageInfo(0), sequenceNumber(seqNum)
{
    if (zeroCoded)
    {
        size_t decodedLength = CountZeroDecodedLength(data, numBytes);
        if (decodedLength == 0)
            throw Exception("Corrupted zero-encoded stream received!");
        messageData.resize(decodedLength, 0); ///\todo Can optimize here for extra-extra bit of performance if profiling shows the need for so..
        bool success = ZeroDecode(&messageData[0], decodedLength, data, numBytes);
        if (!success)
            throw Exception("Zero-decoding input data failed!");
    }
    else
    {
        messageData.reserve(numBytes);
        messageData.insert(messageData.end(), data, data + numBytes);
    }

    size_t messageIDLength = 0;
    messageID = ExtractNetworkMessageID(&messageData[0], numBytes, &messageIDLength);
    if (messageIDLength == 0)
        throw Exception("Malformed SLUDP packet read! MessageID not present!");
    
    // We remove the messageID from the beginning of the message data buffer, since we just want to store the message content.
    messageData.erase(messageData.begin(), messageData.begin() + messageIDLength);
}

NetInMessage::NetInMessage(const NetInMessage &rhs)
{
    sequenceNumber = rhs.sequenceNumber;
    messageInfo = rhs.messageInfo;
    messageData = rhs.messageData;
    currentBlock = rhs.currentBlock;
    currentBlockInstanceNumber = rhs.currentBlockInstanceNumber;
    currentBlockInstanceCount = rhs.currentBlockInstanceCount;
    currentVariable = rhs.currentVariable;
    currentVariableSize = rhs.currentVariableSize;
    bytesRead = rhs.bytesRead;
    messageID = rhs.messageID;
}

NetInMessage::~NetInMessage()
{
}

void NetInMessage::SetMessageInfo(const NetMessageInfo *info)
{
    assert(messageInfo == 0);
    assert(info != 0);
    assert(info->id == messageID);

    messageInfo = info;

    ResetReading();
}

void NetInMessage::ResetReading()
{
    assert(messageInfo);

    currentBlock = 0;
    currentBlockInstanceNumber = 0;
    currentVariable = 0;
    currentVariableSize = 0;
    bytesRead = 0;
    variableCountBlockNext = false;

    // If first block's type is variable, prevent the user proceeding before he has read the block instance count
    // by setting the variableCountBlockNext true.
    if (messageInfo->blocks.size())
    {
        const NetMessageBlock &firstBlock = messageInfo->blocks[currentBlock];
        if (firstBlock.type == NetBlockVariable)
            variableCountBlockNext = true;
    }

    StartReadingNextBlock();
    ReadNextVariableSize();
}

#ifdef _DEBUG
void NetInMessage::SetMessageID(NetMsgID id)
{
    messageID = id;
}
#endif

uint8_t NetInMessage::ReadU8()
{
    RequireNextVariableType(NetVarU8);

    uint8_t *data = (uint8_t*)ReadBytesUnchecked(sizeof(uint8_t));
    if (!data)
        throw Exception("NetInMessage::ReadU8 failed!");
    AdvanceToNextVariable();

    return *data;
}

uint16_t NetInMessage::ReadU16()
{
    RequireNextVariableType(NetVarU16);
    
    uint16_t *data = (uint16_t*)ReadBytesUnchecked(sizeof(uint16_t));
    if (!data)
        throw Exception("NetInMessage::ReadU16 failed!");
    AdvanceToNextVariable();

    return *data;
}

uint32_t NetInMessage::ReadU32()
{
    RequireNextVariableType(NetVarU32);

    uint32_t *data = (uint32_t*)ReadBytesUnchecked(sizeof(uint32_t));
    if (!data)
        throw Exception("NetInMessage::ReadU32 failed!");
    AdvanceToNextVariable();
    return *data;
}

uint64_t NetInMessage::ReadU64()
{
    RequireNextVariableType(NetVarU64);

    uint64_t *data = (uint64_t*)ReadBytesUnchecked(sizeof(uint64_t));
    if (!data)
        throw Exception("NetInMessage::ReadU64 failed!");
    AdvanceToNextVariable();

    return *data;
}

int8_t NetInMessage::ReadS8()
{
    RequireNextVariableType(NetVarS8);

    uint8_t *data = (uint8_t*)ReadBytesUnchecked(sizeof(int8_t));
    if (!data)
        throw Exception("NetInMessage::ReadS8 failed!");
    AdvanceToNextVariable();

    return *data;
}

int16_t NetInMessage::ReadS16()
{
    RequireNextVariableType(NetVarS16);

    int16_t *data = (int16_t*)ReadBytesUnchecked(sizeof(int16_t));
    if (!data)
        throw Exception("NetInMessage::ReadS16 failed!");
    AdvanceToNextVariable();

    return *data;
}

int32_t NetInMessage::ReadS32()
{
    RequireNextVariableType(NetVarS32);

    int32_t *data = (int32_t*)ReadBytesUnchecked(sizeof(int32_t));
    if (!data)
        throw Exception("NetInMessage::ReadS32 failed!");
    AdvanceToNextVariable();

    return *data;
}

int64_t NetInMessage::ReadS64()
{
    RequireNextVariableType(NetVarS64);

    int64_t *data = (int64_t*)ReadBytesUnchecked(sizeof(int64_t));
    if (!data)
        throw Exception("NetInMessage::ReadS64 failed!");
    AdvanceToNextVariable();

    return *data;
}

float NetInMessage::ReadF32()
{
    RequireNextVariableType(NetVarF32);

    float *data = (float*)ReadBytesUnchecked(sizeof(float));
    if (!data)
        throw Exception("NetInMessage::ReadF32 failed!");
    AdvanceToNextVariable();

    return *data;
}

double NetInMessage::ReadF64()
{
    RequireNextVariableType(NetVarF64);

    double *data = (double*)ReadBytesUnchecked(sizeof(double));
    if (!data)
        throw Exception("NetInMessage::ReadF64 failed!");
    AdvanceToNextVariable();

    return *data;
}

bool NetInMessage::ReadBool()
{
    RequireNextVariableType(NetVarBOOL);

    bool *data = (bool*)ReadBytesUnchecked(sizeof(bool));
    if (!data)
        throw Exception("NetInMessage::ReadBool failed!");
    AdvanceToNextVariable();

    return *data;
}

Vector3 NetInMessage::ReadVector3()
{
    RequireNextVariableType(NetVarVector3);

    Vector3 *data = (Vector3*)ReadBytesUnchecked(sizeof(Vector3));
    if (!data)
        throw Exception("NetInMessage::ReadVector3 failed!");
    AdvanceToNextVariable();

    return *data;
}

Vector3d NetInMessage::ReadVector3d()
{
    RequireNextVariableType(NetVarVector3d);

    Vector3d *data = (Vector3d*)ReadBytesUnchecked(sizeof(Vector3d));
    if (!data)
        throw Exception("NetInMessage::ReadVector3d failed!");
    AdvanceToNextVariable();

    return *data;
}

Vector4 NetInMessage::ReadVector4()
{
    RequireNextVariableType(NetVarVector4);

    Vector4 *data = (Vector4*)ReadBytesUnchecked(sizeof(Vector4));
    if (!data)
        throw Exception("NetInMessage::ReadVector4 failed!");
    AdvanceToNextVariable();

    return *data;
}

Quaternion NetInMessage::ReadQuaternion()
{
    RequireNextVariableType(NetVarQuaternion);

    Vector3 *data = (Vector3*)ReadBytesUnchecked(sizeof(Vector3));
    if (!data)
        throw Exception("NetInMessage::ReadQuaternion failed!");
    Quaternion quat = UnpackQuaternionFromFloat3(*data);
    AdvanceToNextVariable();

    return quat;
}

RexUUID NetInMessage::ReadUUID()
{
    RequireNextVariableType(NetVarUUID);

    RexUUID *data = (RexUUID*)ReadBytesUnchecked(sizeof(RexUUID));
    if (!data)
        throw Exception("NetInMessage::ReadUUID failed!");
    AdvanceToNextVariable();

    return *data;
}

void NetInMessage::ReadString(char *dst, size_t maxSize)
{
    // The OpenSim protocol doesn't specify variable type for strings so use "NetVarNone".
    RequireNextVariableType(NetVarNone);
    
    if (maxSize == 0)
        return;

    size_t copyLen = std::min((maxSize-1), ReadVariableSize());

    size_t read = 0;
    const uint8_t *buf = ReadBuffer(&read);
    memcpy(dst, buf, copyLen);
    dst[copyLen] = '\0';
}

std::string NetInMessage::ReadString()
{
    // The OpenSim protocol doesn't specify variable type for strings so use "NetVarNone".
    RequireNextVariableType(NetVarNone);

    char tmp[257];
    ReadString(tmp, 256);

    return std::string(tmp);
}

///\ todo Add the rest of the reading functions (IPPORT, IPADDR).

const uint8_t *NetInMessage::ReadBuffer(size_t *bytesRead)
{
    if (CheckNextVariableType() < NetVarFixed ||
        CheckNextVariableType() > NetVarBuffer4Bytes)
    {
        throw NetMessageException(NetMessageException::ET_VariableTypeMismatch);
    }

    RequireNextVariableType(NetVarNone);

    if (bytesRead)
        *bytesRead = currentVariableSize;

    const uint8_t *data = (const uint8_t *)ReadBytesUnchecked(currentVariableSize);
    if (!data && currentVariableSize != 0)
        throw Exception("NetInMessage::ReadBuffer failed!");
    AdvanceToNextVariable();

    return data;
}

NetVariableType NetInMessage::CheckNextVariableType() const
{
    assert(messageInfo);

    if (currentBlock >= messageInfo->blocks.size())
        return NetVarNone;

    const NetMessageBlock &block = messageInfo->blocks[currentBlock];

    if (currentVariable >= block.variables.size())
        return NetVarNone;

    const NetMessageVariable &var = block.variables[currentVariable];

    return var.type;
}

void NetInMessage::AdvanceToNextVariable()
{
    assert(messageInfo);

    if (currentBlock >= messageInfo->blocks.size()) // We're finished reading if currentBlock points past all the blocks.
        return;

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];

    assert(currentVariable < curBlock.variables.size());
    assert(currentBlockInstanceNumber < currentBlockInstanceCount);

    ++currentVariable;
    if (currentVariable >= curBlock.variables.size())
    {
        /// Reached the end of this block, proceed to the next block
        /// or repeat the same block if it's multiple or variable.
        currentVariable = 0;
        ++currentBlockInstanceNumber;
        if (currentBlockInstanceNumber >= currentBlockInstanceCount)
        {
            currentBlockInstanceNumber = 0;
            // When entering a new block, the very first thing we do is to read the new block count instance.
            ++currentBlock;
            StartReadingNextBlock();
        }
    }

    ReadNextVariableSize();
}

size_t NetInMessage::ReadCurrentBlockInstanceCount()
{
    assert(messageInfo);

    if (currentBlock >= messageInfo->blocks.size())
        return 0;

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    switch(curBlock.type)
    {
    case NetBlockSingle:
    case NetBlockMultiple:
        variableCountBlockNext = false;
        break;
    case NetBlockVariable:
        //if (currentBlock == currentVariableCountBlock)
            //throw NetMessageException(ET_BlockInstanceCountAlreadyRead);
        variableCountBlockNext = true;
        break;
    default:
        break;
    }

    return currentBlockInstanceCount;
}

void NetInMessage::SkipToNextVariable(bool bytesAlreadyRead)
{
    assert(messageInfo);

    // We're finished reading if currentBlock points past all the blocks.
    if (currentBlock >= messageInfo->blocks.size())
        return;

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    const NetMessageVariable &curVar = curBlock.variables[currentVariable];

    assert(currentVariable < curBlock.variables.size());
    assert(currentBlockInstanceNumber < currentBlockInstanceCount);

    if(!bytesAlreadyRead)
    {
        switch(curVar.type)
        {
        case NetVarFixed:
        case NetVarBufferByte:
        case NetVarBuffer2Bytes:
            bytesRead += currentVariableSize;
            break;
        case NetVarBuffer4Bytes:
            assert(false);
            break;
        default:
            bytesRead += NetVariableSizes[curVar.type];
            break;
        }
    }

    ++currentVariable;
    if (currentVariable >= curBlock.variables.size())
    {
        /// Reached the end of this block, proceed to the next block
        /// or repeat the same block if it's multiple or variable.
        currentVariable = 0;
        ++currentBlockInstanceNumber;
        if (currentBlockInstanceNumber >= currentBlockInstanceCount)
        {
            currentBlockInstanceNumber = 0;
            // When entering a new block, the very first thing we do is to read the new block count instance.
            ++currentBlock;
            StartReadingNextBlock();
        }
    }

    ReadNextVariableSize();
}

void NetInMessage::SkipToFirstVariableByName(const std::string &variableName)
{
    assert(messageInfo);

    /// \todo Make sure that one can't skip to inside variable-count block.

    // Check out that the variable really exists.
    bool bFound = false;
    size_t skip_count = 0;
    for(size_t block_it = currentBlock; block_it <  messageInfo->blocks.size(); ++block_it)
    {
        const NetMessageBlock &curBlock = messageInfo->blocks[block_it];
        for(size_t var_it = currentVariable; var_it <  messageInfo->blocks[block_it].variables.size(); ++var_it)
        {
            const NetMessageVariable &curVar = curBlock.variables[var_it];
            if(variableName == curVar.name)
            {
                bFound = true;
                break;
            }
            
            ++skip_count;
        }
    }

    if (!bFound)
        throw NetMessageException(NetMessageException::ET_InvalidVariableName);

    // Skip to the desired variable.
    for(size_t it = 0; it < skip_count; ++it)
        SkipToNextVariable();
}

void NetInMessage::SkipToNextInstanceStart()
{
    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    const int numVariablesLeftInThisInstance = (curBlock.variables.size() - currentVariable);

    for(int i = 0; i < numVariablesLeftInThisInstance; ++i)
        SkipToNextVariable();
}

void NetInMessage::StartReadingNextBlock()
{
    assert(messageInfo);

    if (currentBlock >= messageInfo->blocks.size())
    {
        currentBlockInstanceCount = 0;
        return; // The client has read all the blocks already. Nothing left to read.
    }

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    switch(curBlock.type)
    {
    case NetBlockSingle:
        currentBlockInstanceCount = 1;  // This block occurs exactly once.
        return;
    case NetBlockMultiple:
        // Multiple block quantity is always constant, so use the repeatCount from the MessageInfo
        currentBlockInstanceCount = curBlock.repeatCount;
        assert(currentBlockInstanceCount != 0);
        return;
    case NetBlockVariable:
        // Malformity check.
        if (bytesRead >= messageData.size())
        {
            SkipToPacketEnd();
            return;
        }
        // The block is variable-length. Read how many instances of it are present.
        currentBlockInstanceCount = (size_t)messageData[bytesRead++];

        // If 0 instances present, skip over this block (tail-recursively re-enter this function to do the job.)
        if (currentBlockInstanceCount == 0)
        {
            ++currentBlock;

            // Malformity check.
            if (bytesRead >= messageData.size() || currentBlock >= messageInfo->blocks.size())
            {
                SkipToPacketEnd();
                return;
            }

            // Re-do this function, read the size of the next block.
            StartReadingNextBlock();
            return;
        }
        return;
    default:
        assert(false);
        currentBlockInstanceCount = 0;
        return;
    }
}

void NetInMessage::ReadNextVariableSize()
{
    if (currentBlock >= messageInfo->blocks.size())
    {
        currentVariableSize = 0;
        return;
    }

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    assert(currentVariable < curBlock.variables.size());
    const NetMessageVariable &curVar = curBlock.variables[currentVariable];

    switch(curVar.type)
    {
    case NetVarBufferByte:
        // Variable-sized variable, size denoted with 1 byte.
        if (bytesRead >= messageData.size())
        {
            SkipToPacketEnd();
            return;
        }
        currentVariableSize = messageData[bytesRead++];
        /*if (currentVariableSize == 0)
            ///\todo Causes issues when when skipping consecutive variable-length variables!
            AdvanceToNextVariable();*/
        return;
    case NetVarBuffer2Bytes:
        // Variable-sized variable, size denoted with 2 bytes.
        if (bytesRead + 1 >= messageData.size())
        {
            SkipToPacketEnd();
            return;
        }
        currentVariableSize = (size_t)messageData[bytesRead] + ((size_t)messageData[bytesRead + 1] << 8);
        bytesRead += 2;
        /*if (currentVariableSize == 0)
            ///\todo Causes issues when skipping consecutive variable-length variables!
            AdvanceToNextVariable();*/
        return;
    case NetVarBuffer4Bytes:
        assert(false);
        currentVariableSize = 0;
        return;
    case NetVarFixed:
        currentVariableSize = curVar.count;
        assert(currentVariableSize != 0);
        return;
    default:
        currentVariableSize = NetVariableSizes[curVar.type];
        assert(currentVariableSize != 0);
        return;
    }
}

void *NetInMessage::ReadBytesUnchecked(size_t count)
{
    if (bytesRead >= messageData.size() || count == 0)
        return 0;

    if (bytesRead + count > messageData.size())
    {
        bytesRead = messageData.size(); // Jump to the end of the whole message so that we don't after this read anything.
        std::cout << "Error: Size of the message exceeded. Can't read bytes anymore." << std::endl;
        return 0;
    }

    void *data = &messageData[bytesRead];
    bytesRead += count;

    return data;
}

void NetInMessage::SkipToPacketEnd()
{
    assert(messageInfo);

    currentBlock = messageInfo->blocks.size();
    currentBlockInstanceCount = 0;
    currentVariable = 0;
    currentVariableSize = 0;
    bytesRead = messageData.size();
}

void NetInMessage::RequireNextVariableType(NetVariableType type)
{
    assert(messageInfo);

    const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
    
    // Current block is variable, but user hasn't called ReadCurrentBlockInstanceCount().
    if (curBlock.type == NetBlockVariable && !variableCountBlockNext)
        throw NetMessageException(NetMessageException::ET_BlockInstanceCountNotRead);
        //NetMessageException("Current block is variable: use ReadCurrentBlockInstanceCount first in order to proceed.");
        

    // In case of string or buffer just return.
    if (type == NetVarNone)
        return;
    
    // Check that the variable type matches.
    if (CheckNextVariableType() != type)
        throw NetMessageException(NetMessageException::ET_VariableTypeMismatch);
        //"Tried to read wrong variable type."
}

}