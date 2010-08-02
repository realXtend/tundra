/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef VLEPacker_h
#define VLEPacker_h

#include "clb/Math/Util/BitOps.h"

/// VLEPacker performs variable-length encoding of unsigned integer values by omitting leading
/// zeroes from big numbers.
template<int bits1, int bits2, int bits3 = 0>
class VLEType
{
public:
	static const int numBits1 = bits1;
	static const int numBits2 = bits2;
	static const int numBits3 = bits3;

	static const u32 maxValue1 = clb::LSBT<numBits1>::val;
	static const u32 maxValue2 = clb::LSBT<numBits1+numBits2>::val;
	static const u32 maxValue3 = clb::LSBT<numBits1+numBits2+numBits3>::val;

	static const u32 maxValue = maxValue3;

	static const u32 bitsValue1 = numBits1 + 1;
	static const u32 bitsValue2 = numBits1 + 1 + numBits2 + (numBits3 != 0 ? 1 : 0);
	static const u32 bitsValue3 = (numBits3 == 0) ? 0 : numBits1 + 1 + numBits2 + 1 + numBits3;

	static const u32 maxBits = (numBits3 == 0) ? bitsValue2 : bitsValue3;

	static const u32 bitMask1 = clb::BitMaskT<0, numBits1>::val;
	static const u32 bitMask2 = clb::BitMaskT<bitsValue1, numBits2>::val;//((1 << numBits2) - 1) << bitsValue1;
	static const u32 bitMask3 = clb::BitMaskT<bitsValue1+bitsValue2, numBits3>::val;//((1 << numBits2) - 1) << bitsValue1;
//	static const u32 bitMask3 = ((1 << numBits3) - 1) << bitsValue2;

	static int GetEncodedBitLength(u32 value)
	{
		if (value <= maxValue1)
			return bitsValue1;
		else if (value <= maxValue2)
			return bitsValue2;
		else
			return bitsValue3;
	}

	static u32 Encode(u32 value)
	{
		if (value > maxValue)
			value = maxValue;

		if (value <= maxValue1)
			return value;
		else if (value <= maxValue2)
		{
			u32 lowPart = value & clb::BitMaskT<0, numBits1>::val;
			u32 medPart = value & clb::BitMaskT<numBits1, numBits2>::val;
			assert(medPart != 0);
			return lowPart | (1 << numBits1) | (medPart << 1);
		}
		else
		{
			u32 lowPart = value & clb::BitMaskT<0, numBits1>::val;
			u32 medPart = value & clb::BitMaskT<numBits1, numBits2>::val;
			u32 highPart = value & clb::BitMaskT<numBits1+numBits2, numBits3>::val;
			assert(highPart != 0);
			return lowPart | (1 << numBits1) | (medPart << 1) | (1 << (bitsValue1 + numBits2)) | (highPart << 2);
		}
	}
	
	static u32 Decode(u32 value)
	{
		if ((value & (1 << numBits1)) == 0)
			return value;
		else if ((value & (1 << (bitsValue1 + numBits2))) == 0)
		{
			u32 lowPart = value & clb::BitMaskT<0, numBits1>::val;
			u32 medPart = value & clb::BitMaskT<numBits1+1, numBits2>::val;
			return lowPart | (medPart >> 1);
		}
		else
		{
			u32 lowPart = value & clb::BitMaskT<0, numBits1>::val;
			u32 medPart = (value & clb::BitMaskT<numBits1+1, numBits2>::val);
			u32 highPart = (value & clb::BitMaskT<numBits1+1+numBits2+1, numBits3>::val);
			return lowPart | (medPart >> 1) | (highPart >> 2);
		}
	}
/*	
	static u32 Decode(const char *data, int bitOfs)
	{
		u32 value = 
	}

	/// @return The number of bits taken by the encoding.
	int EncodeAndPack(u32 value, char *dst)
	{
		u32 encoded = Encode(value);
		char *data = reinterpret_cast<char*>(&encoded);
		int bitLength = GetEncodedBitLength(value);
		for(int i = 0; i < (bitLength + 7) / 8; ++i)
			dst[i] = data[i];

		return 
		if (value <= maxValue1)
			return value;
	}
*/
};

typedef VLEType<7, 7, 16> VLE8_16_32;
typedef VLEType<7, 8> VLE8_16;
typedef VLEType<7, 24> VLE8_32;
typedef VLEType<15, 16> VLE16_32;

#endif
