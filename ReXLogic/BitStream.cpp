// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BitStream.h"

namespace RexLogic
{
    BitStream::BitStream(const void *data, size_t numBits)
        :data_(reinterpret_cast<const Core::u8*>(data)), numBits_(numBits), numElems_((numBits + cNumBitsInElem - 1) / cNumBitsInElem), elemOfs_(0), bitOfs_(0)
    {
    }

    void BitStream::ResetPosition()
    {
        elemOfs_ = 0;
        bitOfs_ = 0;
    }

    /** The bits in consecutive bytes are read MSB-first, i.e. the MSB of a byte is read first, and going down to the LSB. */
    Core::u32 BitStream::ReadBits(int count)
    {
        Core::u32 val = 0;
        while(count-- > 0)
            val = (val << 1) | (ReadBit() ? 1 : 0);

        return val;
    }

    bool BitStream::ReadBit()
    {
        if (BitsLeft() == 0)
            return false;

        bool bit = (data_[elemOfs_] & (1 << bitOfs_++)) != 0;
        if (bitOfs_ >= cNumBitsInElem)
        {
            bitOfs_ = 0;
            ++elemOfs_;
        }
        return bit;
    }
}
