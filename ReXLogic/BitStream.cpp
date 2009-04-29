// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BitStream.h"

namespace RexLogic
{
    /*! \todo It's very odd to have to define this here, but otherwise 
        linking to or loading RexLogicModule fails on Linux because of
        undefined reference
     */
    int BitStream::num_bits_in_elem_ = 8;

    BitStream::BitStream(const void *data, size_t num_bytes)
        :data_(reinterpret_cast<const Core::u8*>(data)), num_elems_((num_bytes*num_bits_in_elem_ + num_bits_in_elem_ - 1) / num_bits_in_elem_), elem_ofs_(0), bit_ofs_(0)
    {
    }

    void BitStream::ResetPosition()
    {
        elem_ofs_ = 0;
        bit_ofs_ = 0;
    }

    Core::u32 BitStream::ReadBits(int count)
    {
        Core::u8 data[4] = { 0 };
        int cur_byte = 0;
        int cur_bit = 0;
        assert(num_bits_in_elem_ == 8);
        int total_bits = std::min(num_bits_in_elem_, count);
        while(count-- > 0)
        {
            if (ReadBit()) ///\note Can optimize here - read several bits at a time instead of looping bit-per-bit.
                data[cur_byte] |= 1 << (total_bits - 1 - cur_bit);
            if (++cur_bit >= num_bits_in_elem_)
            {
                ++cur_byte;
                cur_bit = 0;
                total_bits = std::min(num_bits_in_elem_, count);
            }
        }
        return *reinterpret_cast<Core::u32*>(data);
    }

    bool BitStream::ReadBit()
    {
        if (BitsLeft() == 0)
            return false;

        bool bit = (data_[elem_ofs_] & (1 << (num_bits_in_elem_ - 1 - bit_ofs_))) != 0;
        ++bit_ofs_;
        if (bit_ofs_ >= num_bits_in_elem_)
        {
            bit_ofs_ = 0;
            ++elem_ofs_;
        }
        return bit;
    }

}
