// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BitStream.h"

namespace RexLogic
{
    BitStream::BitStream(const void *data, size_t numBytes)
        :data_(reinterpret_cast<const Core::u8*>(data)), numBits_(numBytes*8), numElems_((numBytes*8 + cNumBitsInElem - 1) / cNumBitsInElem), elemOfs_(0), bitOfs_(0)
    {
    }

    void BitStream::ResetPosition()
    {
        elemOfs_ = 0;
        bitOfs_ = 0;
    }

    /** The bits in consecutive bytes are read MSB-first, i.e. the MSB of a byte is read first, and going down to the LSB.
        But the bytes are read little-endian. */
    Core::u32 BitStream::ReadBits(int count)
    {
        Core::u8 data[4] = { 0 };
        int curByte = 0;
        int curBit = 0;
        assert(cNumBitsInElem == 8);
        int totalBits = std::min(cNumBitsInElem, count);
        while(count-- > 0)
        {
            if (ReadBit()) ///\note Can optimize here - read several bits at a time instead of looping bit-per-bit.
                data[curByte] |= 1 << (totalBits - 1 - curBit);
            if (++curBit >= cNumBitsInElem)
            {
                ++curByte;
                curBit = 0;
                totalBits = std::min(cNumBitsInElem, count);
            }
        }
        return *reinterpret_cast<Core::u32*>(data);
    }

    bool BitStream::ReadBit()
    {
        if (BitsLeft() == 0)
            return false;

        bool bit = (data_[elemOfs_] & (1 << (cNumBitsInElem - 1 - bitOfs_))) != 0;
        ++bitOfs_;
        if (bitOfs_ >= cNumBitsInElem)
        {
            bitOfs_ = 0;
            ++elemOfs_;
        }
        return bit;
    }

}
