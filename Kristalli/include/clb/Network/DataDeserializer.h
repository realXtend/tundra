/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef DataDeserializer_h
#define DataDeserializer_h

#include "clb/Core/Types.h"

#include "BasicSerializedDataTypes.h"
#include "SerializedDataIterator.h"

/// DataDeserializer is an utility class that walks through and deserializes data in a stream of raw bytes. The stream
/// itself does not contain information about what types of data is contained within, but the user of DataDeserializer must
/// know the contents of the data.
class DataDeserializer
{
	/// The data pointer to read from.
	const char *data;
	/// The length of the read buffer in bytes.
	size_t size;
	/// The element we're reading next from in the data buffer.
	size_t elemOfs;
	/// The current bit index of the byte we're reading, [0, 7].
	int bitOfs;

	Ptr(SerializedDataIterator) iter;
    
	u32 ReadBitsToU32(int count);

	DataDeserializer(const DataDeserializer &);
	void operator =(const DataDeserializer &);

public:
	DataDeserializer(const char *data_, size_t size_)
	:data(data_), size(size_)
	{
		assert(data);
		assert(size > 0);
		ResetTraversal();
	}

	DataDeserializer(const char *data_, size_t size_, const SerializedMessageDesc *msgTemplate)
	:data(data_), size(size_)
	{
		assert(data);
		assert(size > 0);
		assert(msgTemplate);

		iter = new SerializedDataIterator(*msgTemplate);

		ResetTraversal();
	}

	void ResetTraversal();

	template<typename T>
	T Read();

	static const u32 VLEReadError = 0xFFFFFFFF;

	template<typename VLEType>
	u32 ReadVLE();

	template<typename T>
	void ReadArray(T *dst, size_t numElems);

	/// Reads a string from the stream. Need to use a template to use this function.
	/// Use the 's8' data type with dynamicCount set in the message template description for a string variable.
	std::string ReadString();

	/// Reads the given amount of bits and packs them into a u32, which is returned.
	/// @param numBits the number of bits to read, [1, 32].
	u32 ReadBits(int numBits);

	u32 GetDynamicElemCount();

	/// @return The number of bytes left in the stream to read.
	u32 BytesLeft() const { return (u32)((elemOfs >= size) ? 0 : (size - elemOfs)); }

	/// @return The number of bits left in the stream to read.
	u32 BitsLeft() const { return (u32)((elemOfs >= size) ? 0 : ((size - elemOfs)*8 - bitOfs)); }

	/// @return The current byte of the stream that will be read next.
	u32 BytePos() const { return (u32)elemOfs; }

	/// @return The bit index, [0, 7] of the byte that will be read next. The reading will proceed from LSBit to MSBit, i.e. 0 -> 7.
	u32 BitPos() const { return bitOfs; }

	/// Advances the read pointer with the given amount of bits. Can only be used in nontemplate read mode.
	void SkipBits(size_t numBits);

	/// Advances the read pointer with the given amount of bytes. Can only be used in nontemplate read mode.
	void SkipBytes(size_t numBytes) { SkipBits(numBytes * 8); }
};

template<typename T>
T DataDeserializer::Read()
{
	assert(!iter || iter->NextElementType() == SerializedDataTypeTraits<T>::type);
	T value;
	u8 *data = reinterpret_cast<u8*>(&value);

	for(int i = 0; i < sizeof(value); ++i)
		data[i] = (u8)ReadBitsToU32(8);

	if (iter)
		iter->ProceedToNextVariable();

	return value;
}

template<typename VLEType>
u32 DataDeserializer::ReadVLE()
{
	const u32 cStreamEndError = 0xFFFFFFFF;

	if (BitsLeft() < VLEType::numBits1 + 1)
		return cStreamEndError;

	u32 sml = ReadBits(VLEType::numBits1 + 1);
	if ((sml & (1 << VLEType::numBits1)) == 0)
		return sml;
	sml &= (1 << VLEType::numBits1) - 1;

	if (BitsLeft() < VLEType::numBits2 + 1)
		return cStreamEndError;

	u32 med = ReadBits(VLEType::numBits2 + 1);
	if ((med & (1 << VLEType::numBits2)) == 0)
		return sml | (med << VLEType::numBits1);

	if (BitsLeft() < VLEType::numBits3)
		return cStreamEndError;

	med &= (1 << VLEType::numBits2) - 1;
	u32 large = ReadBits(VLEType::numBits3);
	return sml | (med << VLEType::numBits1) | (large << (VLEType::numBits1 + VLEType::numBits2));
}

template<typename T>
void DataDeserializer::ReadArray(T *dst, size_t numElems)
{
	for(size_t i = 0; i < numElems; ++i)
		dst[i] = Read<T>();

	// The above will move on to the next variable in the template accordingly, but if the number
	// of elements to read is zero, it needs to be assumed that this was a dynamic block and 
	// the size was 0.
	if (numElems == 0 && iter)
		iter->ProceedToNextVariable();
}

template<>
bool DataDeserializer::Read<bit>();

template<>
void DataDeserializer::ReadArray(bit *dst, size_t numElems);

#endif
