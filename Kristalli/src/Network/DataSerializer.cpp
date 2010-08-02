/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
//#include "clb/Core/Logging.h"
#include "clb/Network/DataSerializer.h"
#include "clb/Math/Util/BitOps.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

using namespace clb;

DataSerializer::DataSerializer(size_t maxBytes_)
{
	assert(maxBytes_ > 0);

	maxBytes = maxBytes_;
	messageData = new SerializedMessage();
	messageData->data.resize(maxBytes);
	data = &messageData->data[0];

	ResetFill();
}

DataSerializer::DataSerializer(size_t maxBytes_, const SerializedMessageDesc *msgTemplate)
{
	assert(maxBytes_ > 0);
	assert(msgTemplate != 0);

	iter = new SerializedDataIterator(*msgTemplate);

	maxBytes = maxBytes_;
	messageData = new SerializedMessage();
	messageData->data.resize(maxBytes);
	data = &messageData->data[0];

	ResetFill();
}

DataSerializer::DataSerializer(char *data_, size_t maxBytes_)
:data(data_), maxBytes(maxBytes_)
{
	ResetFill();
}

DataSerializer::DataSerializer(char *data_, size_t maxBytes_, const SerializedMessageDesc *msgTemplate)
:data(data_), maxBytes(maxBytes_)
{
	assert(msgTemplate != 0);

	iter = new SerializedDataIterator(*msgTemplate);
	ResetFill();
}

DataSerializer::DataSerializer(std::vector<char> &data_, size_t maxBytes_)
{
	if (data_.size() < maxBytes_)
		data_.resize(maxBytes_);
	assert(data_.size() > 0);
	data = &data_[0];
	maxBytes = maxBytes_;

	ResetFill();
}

DataSerializer::DataSerializer(std::vector<char> &data_, size_t maxBytes_, const SerializedMessageDesc *msgTemplate)
{
	if (data_.size() < maxBytes_)
		data_.resize(maxBytes_);
	assert(data_.size() > 0);
	data = &data_[0];
	maxBytes = maxBytes_;

	assert(msgTemplate != 0);
	iter = new SerializedDataIterator(*msgTemplate);

	ResetFill();
}

void DataSerializer::AppendByte(u8 byte)
{
	if (bitOfs == 0)
		AppendAlignedByte(byte);
	else
		AppendUnalignedByte(byte);
}

void DataSerializer::AppendUnalignedByte(u8 byte)
{
	// The current partial byte can hold (8-bitOfs) bits.
	data[elemOfs] = (data[elemOfs] & LSB(bitOfs)) | ((byte & LSB(8-bitOfs)) << bitOfs);
	// The next byte can hold full 8 bits, but we only need to add bitOfs bits.
	data[++elemOfs] = byte >> (8-bitOfs);
}

void DataSerializer::AppendAlignedByte(u8 byte)
{
	assert(bitOfs == 0);

	assert(elemOfs < maxBytes);
	data[elemOfs++] = byte;
}

void DataSerializer::AppendBits(u32 value, int amount)
{
	const u8 *bytes = reinterpret_cast<const u8*>(&value);
	while(amount >= 8)
	{
		AppendByte(*bytes);
		++bytes;
		amount -= 8;
	}

	u8 remainder = *bytes & LSB(amount);

	data[elemOfs] = (data[elemOfs] & LSB(bitOfs)) | ((remainder & LSB(8-bitOfs)) << bitOfs);
	if (amount > 8-bitOfs)
		data[++elemOfs] = remainder >> (8-bitOfs);

	bitOfs = (bitOfs + amount) & 7;
}

void DataSerializer::ResetFill()
{
	if (iter)
		iter->ResetTraversal();
	elemOfs = 0;
	bitOfs = 0;
}

/// Requires a template to be present to use this.
void DataSerializer::SetVaryingElemSize(u32 count)
{
	assert(iter.ptr() != 0);
	assert(iter->NextElementDesc() != 0);

	AppendBits(count, iter->NextElementDesc()->count);

	iter->SetVaryingElemSize(count);
}

template<>
void DataSerializer::Add<bit>(const bit &value)
{
	u8 val = (value != 0) ? 1 : 0;
	AppendBits(val, 1);
}

void DataSerializer::AddString(const char *str)
{
	size_t len = strlen(str);
	if (iter)
		SetVaryingElemSize(len);
	else
		Add<u8>(len);

	AddArray<s8>((const s8*)str, len);
}

void DataSerializer::SkipNumBytes(size_t numBytes)
{
	elemOfs += numBytes;
	///\todo Check out-of-bounds.

}