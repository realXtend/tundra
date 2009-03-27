// For conditions of distribution and use, see copyright notice in license.txt
#include "NetOutMessage.h"
#include "Poco/Net/DatagramSocket.h" // To get htons etc.
#include <iostream>
#include <cstring>

using namespace RexTypes;

NetOutMessage::NetOutMessage()
{
	const size_t maxMessageSize = 2048;
	messageData.resize(maxMessageSize);
	bytesFilled = 0;
	currentBlock = 0;
	currentVariable = 0;
	blockQuantityCounter = 0;
	for(size_t i = 0; i < maxMessageSize; ++i)
	    messageData[i] = 0;
}

NetOutMessage::~NetOutMessage()
{
}

void NetOutMessage::AddU8(uint8_t value)
{
	if (CheckNextVariable() == NetVarU8)
	{
		AddBytesUnchecked(sizeof(uint8_t), &value);
		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddU16(uint16_t value)
{
	if (CheckNextVariable() == NetVarU16) 
	{
		uint16_t networkValue = value; //htons
		AddBytesUnchecked(NetVariableSizes[2], &networkValue);
		AdvanceToNextVariable();
	}
}

///\todo Advance. hton.
void NetOutMessage::AddU32(uint32_t value)
{
	if (CheckNextVariable() == NetVarU32) 
	{
		uint32_t networkValue = value; //htonl
		AddBytesUnchecked(sizeof(uint32_t), &networkValue);
		AdvanceToNextVariable();
	}
}

///\todo Advance. hton.
void NetOutMessage::AddU64(uint64_t value)
{
	if (CheckNextVariable() == NetVarU64)
	{
		AddBytesUnchecked(sizeof(uint64_t), &value);
		AdvanceToNextVariable();
    }
}

///\todo Advance. hton.
void NetOutMessage::AddS8(int8_t value)
{
	if (CheckNextVariable() == NetVarS8) 
	{
		AddBytesUnchecked(sizeof(int8_t), &value);
		AdvanceToNextVariable();
    }		
}

void NetOutMessage::AddS16(int16_t value)
{
	if (CheckNextVariable() == NetVarS16) 
	{
		AddBytesUnchecked(sizeof(int16_t), &value);
		AdvanceToNextVariable();
    }
}

void NetOutMessage::AddS32(int32_t value)
{
	if (CheckNextVariable() == NetVarS32) 
	{
		AddBytesUnchecked(sizeof(int32_t), &value);
		AdvanceToNextVariable();
    }
}

void NetOutMessage::AddS64(int64_t value)
{
	if (CheckNextVariable() == NetVarS64)
    {	
		AddBytesUnchecked(sizeof(int64_t), &value);
		AdvanceToNextVariable();
    }		
}

void NetOutMessage::AddF32(float value)
{
	if (CheckNextVariable() == NetVarF32)
	{
		AddBytesUnchecked(sizeof(float), &value);
		AdvanceToNextVariable();
    }
}

void NetOutMessage::AddF64(double value)
{
	if (CheckNextVariable() == NetVarF64)
	{
		AddBytesUnchecked(sizeof(double), &value);
		AdvanceToNextVariable();
    }
}

void NetOutMessage::AddVector3(const Vector3 &value)
{
	if (CheckNextVariable() == NetVarVector3)
	{
		//uint32_t networkValue = htonl(*(uint32_t*)&value.x);
		AddBytesUnchecked(sizeof(float), &value.x/*&networkValue*/);

		//networkValue = htonl(*(float)&value.y);
		AddBytesUnchecked(sizeof(float), &value.y/*&networkValue*/);

		//networkValue = htonl(*(float*)&value.z);
		AddBytesUnchecked(sizeof(float), &value.z/*&networkValue*/);

		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddVector3d(const Vector3d &value)
{
	if (CheckNextVariable() == NetVarVector3d) 
	{
		AddBytesUnchecked(sizeof(double), &value.x);
		AddBytesUnchecked(sizeof(double), &value.y);
		AddBytesUnchecked(sizeof(double), &value.z);
		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddVector4(const Vector4 &value)
{
	if (CheckNextVariable() == NetVarVector4)
	{
		AddBytesUnchecked(sizeof(float), &value.x);
		AddBytesUnchecked(sizeof(float), &value.y);
		AddBytesUnchecked(sizeof(float), &value.z);
		AddBytesUnchecked(sizeof(float), &value.w);
		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddQuaternion(const Quaternion &value)
{
	if (CheckNextVariable() == NetVarQuaternion)
	{
		AddBytesUnchecked(sizeof(float), &value.x);
		AddBytesUnchecked(sizeof(float), &value.y);
		AddBytesUnchecked(sizeof(float), &value.z);
		AddBytesUnchecked(sizeof(float), &value.w);
		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddUUID(const RexUUID &value)
{
	if (CheckNextVariable() == NetVarUUID) 
	{
		AddBytesUnchecked(sizeof(RexUUID), &value);
		AdvanceToNextVariable();
	}
}

void NetOutMessage::AddBool(bool value)
{
	if (CheckNextVariable() == NetVarBOOL)
	{
		AddBytesUnchecked(sizeof(bool), &value);
		AdvanceToNextVariable();
	}
}

/*void NetOutMessage::AddIPAddr(IPADDR value);
{
	if (CheckNextVariable() == NetVarIPADDR) 
    {	
		AddBytesUnchecked(NetVariableSizes[17], &value);
		AdvanceToNextVariable();
    }		
}*/

/*void NetOutMessage::AddIPPort(IPPORT value);
{
	if (CheckNextVariable() == NetVarIPPORT)
	{
		AddBytesUnchecked(NetVariableSizes[18], &value);
		AdvanceToNextVariable();
    }
}*/

void NetOutMessage::AddBuffer(size_t count, uint8_t *data)
{
	const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
	const NetMessageVariable &curVar = curBlock.variables[currentVariable];
	switch(curVar.type)
	{
	case NetVarBufferByte:
		// A variable-length buffer where the length is encoded by one byte. (<= 255 bytes in size)
		assert(count <= 255);
		AddBytesUnchecked(1, &count);
		AddBytesUnchecked(count, &data[0]);
		AdvanceToNextVariable();
		break;
	case NetVarBuffer2Bytes:
		// A variable-length buffer where the length is encoded by two bytes. (<= 65535 bytes in size)
		assert(count <= 65535);
		AddBytesUnchecked(2, &count);
		AddBytesUnchecked(count, &data[0]);
		AdvanceToNextVariable();
		break;
	case NetVarBuffer4Bytes:
		// A variable-length buffer where the length is encoded by four bytes. (<= 4 GB in size)
		AddBytesUnchecked(4, &count);
		AddBytesUnchecked(count, &data[0]);
		AdvanceToNextVariable();
		break;
	default:
		std::cout << "Can't add variable sized buffer for a non-variable sized variable!" << std::endl;
		break;
	}
}

NetVariableType NetOutMessage::CheckNextVariable() const
{
	if(messageInfo)
	{
		const NetMessageBlock &block = messageInfo->blocks[currentBlock];
		const NetMessageVariable &var = block.variables[currentVariable];
		return var.type;
	}
	else
	{
		std::cout << "MessageInfo not set. Could not check the variable type." << std::endl;
		return NetVarInvalid;
	}

}

void NetOutMessage::AdvanceToNextVariable()
{
	++currentVariable;

	const NetMessageBlock &curBlock = messageInfo->blocks[currentBlock];
	if (curBlock.type == NetBlockVariable && blockQuantityCounter < 1)
	{
		///\todo: test
		std::cout << "Repeat count not set for block whose type is variable! Can't proceed." << std::endl;
		return;
	}

	size_t var_size = curBlock.variables.size();
	if (currentVariable >= var_size)
	{
		/// Reached the end of this block, proceed to the next block
		/// or repeat the same block if it's multiple or variable.
		currentVariable = 0;
		if (curBlock.repeatCount > 1 && blockQuantityCounter <= curBlock.repeatCount)
			--blockQuantityCounter;
		else
			currentBlock++;
	}
}

void NetOutMessage::SetVariableBlockCount(size_t count)
{
	const NetMessageBlockType &curType = messageInfo->blocks[currentBlock].type;
	if (curType == NetBlockVariable)
	{
		assert(count <= 255);
		blockQuantityCounter = count;
		AddBytesUnchecked(1, &count);
	}
	else if (curType == NetBlockMultiple)
		std::cout << "Can't set block count for a multiple type block!" << std::endl;
	else if (curType == NetBlockSingle)
		std::cout << "Can't set block count for a single type block!" << std::endl;
}

void NetOutMessage::AddBytesUnchecked(size_t count, const void *data)
{
	if (bytesFilled + count > messageData.size())
		messageData.resize(bytesFilled + count, 0);
	
	memcpy(&messageData[bytesFilled], data, count);
	bytesFilled += count;
}

void NetOutMessage::AddMessageHeader()
{	
	if(!messageInfo) return;
	NetMsgID id = messageInfo->id;
	bytesFilled = 0;
	
	// Flags: Byte 0 \todo: How about ack and resent flags?
	if (messageInfo->encoding == NetZeroEncoded)
		messageData[0] |= NetFlagZeroCode;
	bytesFilled += 1;

	// Sequence number: Bytes 1-4, added with separate function.
	messageData[0] = 0;
	messageData[1] = 0;
	messageData[2] = 0;
	messageData[3] = 0;
	bytesFilled += 4;
	
	///\todo?	If byte 5 is non-zero, then there is some extra header information. 
	///			Clients which are not expecting that header information may skip it 
	///			by jumping forward 'Extra' bytes into the message payload.
	///			ATM it seems that the server doesn't check this at all?
	// Extra Byte: Byte 5, ignored for now
	messageData[5] = 0;
	bytesFilled += 1;

	// Extra Header: Bytes 6-9
	if (id >= 0x01 && id <= 0xFE)
	{
		// High priority
//		messageData[5] = 1;
		messageData[6] = (uint8_t)id;
		bytesFilled += 1;
	}
	else if (id >= 0xFF01 && id <= 0xFFFE)
	{
		// Medium priority
//		messageData[5] = 2;
        messageData[6] = 0xff;
		messageData[7] = (uint8_t)id;
		bytesFilled += 2;
	}
	else
	{
		// Low priority, id >= 0xFFFF0001
//		messageData[5] = 4;
		messageData[6] = 0xff;
		messageData[7] = 0xff;
		messageData[8] = (uint8_t)(id >> 8);
		messageData[9] = (uint8_t)(id % 256);
		bytesFilled += 4;
	}
}

void NetOutMessage::SetSequenceNumber(size_t seqNum)
{
	sequenceNumber = seqNum;
    messageData[1] = (uint8_t)(seqNum >> 24);
    messageData[2] = (uint8_t)(seqNum >> 16);
    messageData[3] = (uint8_t)(seqNum >> 8);
    messageData[4] = (uint8_t)(seqNum % 256);
}

void NetOutMessage::SetMessageInfo(const NetMessageInfo *info)
{
	messageInfo = info;

	// Set block quantity counter for blocks whose type is multiple.
	// Variable type counter will be defined by the user and single sized
	// don't need the counter.
	size_t size = messageInfo->blocks.size();
	for(size_t i = 0; i< size ; ++i)
	{
		const NetMessageBlock &block = messageInfo->blocks[i];
		if (block.type == NetBlockMultiple)
		{
			std::cout << block.name << " counter set to " << block.repeatCount << std::endl;
			blockQuantityCounter = block.repeatCount;
		}
	}	

}
