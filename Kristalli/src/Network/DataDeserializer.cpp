/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#include <cassert>

#include "clb/Network/DataDeserializer.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

void DataDeserializer::ResetTraversal()
{
	elemOfs = 0;
	bitOfs = 0;
	if (iter)
		iter->ResetTraversal();
}

u32 DataDeserializer::ReadBitsToU32(int count)
{
	if (BitsLeft() < (u32)count)
		throw std::exception("Not enough bits left in DataDeserializer::ReadBits!"); ///\todo Own exception class.

	u32 var = 0;
	int bitsFilled = 0;
	while(count > 0)
	{
		int bitsToRead = std::min(std::min(8, count), 8 - bitOfs);
		u32 readMask = ((1 << bitsToRead) - 1) << bitOfs;
		assert(elemOfs < size);
		if (elemOfs >= size)
			throw std::exception("Ran off past the array in DataDeserializer::ReadBits!"); ///\todo Own exception class.

//		var |= ((u32)data[elemOfs] & readMask) << bitsFilled;
		var |= (((u32)data[elemOfs] & readMask) >> bitOfs) << bitsFilled;
		bitsFilled += bitsToRead;
		bitOfs += bitsToRead;
		assert(bitOfs <= 8);

		if (bitOfs == 8)
		{
			bitOfs = 0;
			++elemOfs;
		}
		count -= bitsToRead;
	}
	return var;
}

/// Need to have a message template to use this function.
u32 DataDeserializer::GetDynamicElemCount()
{
	const SerializedElementDesc *desc = iter->NextElementDesc();
	assert(desc);

	assert(desc->varyingCount == true);

	if (BitsLeft() < (u32)desc->count)
		throw std::exception("Not enough bits left in DataDeserializer::GetDynamicElemCount!"); ///\todo Own exception class.

	u32 count = ReadBitsToU32(desc->count);
	iter->SetVaryingElemSize(count);
	return count;
}

template<>
bool DataDeserializer::Read<bit>()
{
	assert(!iter || iter->NextElementType() == SerialBit);
	bool value = (ReadBitsToU32(1) != 0);

	if (iter)
		iter->ProceedToNextVariable();

	return value;
}
/*
template<>
void DataDeserializer::ReadArray<bit>(clb::u8 *dst, size_t numElems)
{
	for(int i = 0; i < numElems; ++i)
	{
		dst[i] = (clb::u8)ReadBits(8);
		iter.ProceedToNextVariable();
	}
}
*/
u32 DataDeserializer::ReadBits(int numBits)
{
	assert(!iter || iter->NextElementType() == SerialBit);

	u32 val = ReadBitsToU32(numBits);

	if (iter)
		iter->ProceedNVariables(numBits);

	return val;
}

void DataDeserializer::SkipBits(size_t numBits)
{
	assert(!iter);

	elemOfs += numBits / 8;
	bitOfs += numBits % 8;
	if (bitOfs >= 8)
	{
		bitOfs -= 8;
		++elemOfs;
	}
	assert(elemOfs <= size);
	assert(elemOfs < size || bitOfs == 0);

	if (elemOfs > size)
		throw std::exception("Not enough bits left in DataDeserializer::SkipBits!"); ///\todo. Own exception class.
	if (elemOfs >= size && bitOfs != 0)
		throw std::exception("Not enough bits left in DataDeserializer::SkipBits(2)!"); ///\todo. Own exception class.
}

std::string DataDeserializer::ReadString()
{
	u32 length = (iter ? GetDynamicElemCount() : Read<u8>());
	if (BitsLeft() < length*8)
		throw std::exception("Not enough bytes left in DataDeserializer::ReadString!"); ///\todo. Own exception class.

	std::string str;
	if (bitOfs == 0)
	{
		str.append(data + elemOfs, length);
		elemOfs += length;
	}
	else
	{
		std::vector<s8> bytes(length+1);
		ReadArray<s8>(&bytes[0], length);
		str.append((char*)&bytes[0], length);
	}

	if (iter)
		for(u32 i = 0; i < length; ++i)
			iter->ProceedToNextVariable();

	return str;
}
