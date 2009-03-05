// For conditions of distribution and use, see copyright notice in license.txt
#include <iostream>
#include "NetInMessage.h"
#include "ZeroCode.h"

NetInMessage::NetInMessage(const NetMessageInfo *info, const uint8_t *data, size_t numBytes, bool zeroCoded)
:messageInfo(info)
{
	assert(info != 0);

	if (zeroCoded)
	{
		size_t decodedLength = CountZeroDecodedLength(data, numBytes);
		messageData.resize(decodedLength, 0); ///\todo Can optimize here for extra-extra bit of performance if profiling shows the need for so..
		ZeroDecode(&messageData[0], decodedLength, data, numBytes);
	}
	else
	{
		messageData.resize(numBytes, 0); ///\todo Can optimize here for extra-extra bit of performance if profiling shows the need for so..
		for(size_t i = 0; i < numBytes; ++i)
			messageData[i] = data[i];
	}

	ResetReading();
}

NetInMessage::~NetInMessage()
{
}

void NetInMessage::ResetReading()
{
	currentBlock = 0;
	currentBlockInstanceNumber = 0;
	currentVariable = 0;
	currentVariableSize = 0;
	bytesRead = 0;

	StartReadingNextBlock();
	ReadNextVariableSize();
}

uint8_t NetInMessage::ReadU8()
{
	if (CheckNextVariableType() == NetVarU8)
	{
		uint8_t *data = (uint8_t*)ReadBytesUnchecked(sizeof(uint8_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

uint16_t NetInMessage::ReadU16()
{
	if (CheckNextVariableType() == NetVarU16)
	{
		uint16_t *data = (uint16_t*)ReadBytesUnchecked(sizeof(uint16_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

uint32_t NetInMessage::ReadU32()
{
	if (CheckNextVariableType() == NetVarU32)
	{
		uint32_t *data = (uint32_t*)ReadBytesUnchecked(sizeof(uint32_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

uint64_t NetInMessage::ReadU64()
{
	if (CheckNextVariableType() == NetVarU64)
	{
		uint64_t *data = (uint64_t*)ReadBytesUnchecked(sizeof(uint64_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

int8_t NetInMessage::ReadS8()
{
	if (CheckNextVariableType() == NetVarS8)
	{
		uint8_t *data = (uint8_t*)ReadBytesUnchecked(sizeof(int8_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

int16_t NetInMessage::ReadS16()
{
	if (CheckNextVariableType() == NetVarS16)
	{
		int16_t *data = (int16_t*)ReadBytesUnchecked(sizeof(int16_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

int32_t NetInMessage::ReadS32()
{
	if (CheckNextVariableType() == NetVarS32)
	{
		int32_t *data = (int32_t*)ReadBytesUnchecked(sizeof(int32_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

int64_t NetInMessage::ReadS64()
{
	if (CheckNextVariableType() == NetVarS64)
	{
		int64_t *data = (int64_t*)ReadBytesUnchecked(sizeof(int64_t));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

float NetInMessage::ReadF32()
{
	if (CheckNextVariableType() == NetVarF32)
	{
		float *data = (float*)ReadBytesUnchecked(sizeof(float));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

double NetInMessage::ReadF64()
{
	if (CheckNextVariableType() == NetVarF64)
	{
		double *data = (double*)ReadBytesUnchecked(sizeof(double));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

bool NetInMessage::ReadBool()
{
	if (CheckNextVariableType() == NetVarBufferByte)
	{
		bool *data = (bool*)ReadBytesUnchecked(sizeof(bool));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}

/*Vector3 NetInMessage::ReadVector3()
{
	if (CheckNextVariableType() == NetVarVector3)
	{
		Vector3 *data = (Vector3*)ReadBytesUnchecked(sizeof(Vector3));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}*/

/*Vector3d NetInMessage::ReadVector3d()
{
	if (CheckNextVariableType() == NetVarVector3d)
	{
		Vector3d *data = (Vector3d*)ReadBytesUnchecked(sizeof(Vector3d));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}*/

/*Vector4 NetInMessage::ReadVector4()
{
	if (CheckNextVariableType() == NetVarVector4)
	{
		Vector4 *data = (Vector4*)ReadBytesUnchecked(sizeof(Vector4));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}*/

/*Quaternion NetInMessage::ReadQuaternion()
{
	if (CheckNextVariableType() == NetVarQuaternion)
	{
		Quaternion *data = (Quaternion*)ReadBytesUnchecked(sizeof(Quaternion));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Error: Tried to read wrong variable type." << std::endl;
		return 0;
	}
}*/

UUID NetInMessage::ReadUUID()
{
	if (CheckNextVariableType() == NetVarUUID)
	{
		UUID *data = (UUID*)ReadBytesUnchecked(sizeof(UUID));
		AdvanceToNextVariable();
		return *data;
	}
	else 
	{	
		std::cout << "Invalid variable type. Current type is " << CheckNextVariableType() << std::endl;
		return UUID();
	}
}

///\ todo Add the rest of the reading functions (IPPORT, IPADDR).

const uint8_t *NetInMessage::ReadBuffer(size_t *bytesRead)
{
	if (CheckNextVariableType() == NetVarBufferByte ||
		CheckNextVariableType() == NetVarBuffer2Bytes)
	{
		if (bytesRead)
			*bytesRead = currentVariableSize;
		const uint8_t *data = (const uint8_t *)ReadBytesUnchecked(currentVariableSize);
		AdvanceToNextVariable();
		return data;
	}
	else 
	{	
		std::cout << "Invalid variable type. Current type is " << CheckNextVariableType() << std::endl;
		return 0;
	}
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

void NetInMessage::SkipToNextVariable(bool bytesAlreadyRead)
{
	if (currentBlock >= messageInfo->blocks.size()) // We're finished reading if currentBlock points past all the blocks.
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

void NetInMessage::SkipToFirstVariableByName(std::string variableName)
{
	assert(messageInfo);
	
	size_t skip_count = 0;

	// Check out that the variable really exists.
	bool bFound = false;
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
	{
		std::cout << "Variable \"" << variableName << "\" not found in the message info!" << std::endl;
		return;
	}

	// Skip to the desired variable.
	for(size_t it = 0; it < skip_count; ++it)
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
	if (bytesRead >= messageData.size() && count == 0)
		return 0;

	if (bytesRead + count > messageData.size())
	{
		std::cout << "Error: Size of the message exceeded. Can't read bytes anymore." << std::endl;
		return 0;
	}

	void *data = &messageData[bytesRead];
	bytesRead += count;

	return data;
}

void NetInMessage::SkipToPacketEnd()
{
	currentBlock = messageInfo->blocks.size();
	currentBlockInstanceCount = 0;
	currentVariable = 0;
	currentVariableSize = 0;
	bytesRead = messageData.size();
}
