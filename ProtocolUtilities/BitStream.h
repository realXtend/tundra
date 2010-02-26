// For conditions of distribution and use, see copyright notice in license.txt

#ifndef RexLogic_BitStream_h
#define RexLogic_BitStream_h

#include "CoreTypes.h"

namespace ProtocolUtilities
{
    /// A stream reader utility for reading a byte array bit-by-bit.
    class BitStream
    {
    private:
        /// The memory is now accessed per-byte, but for optimization, one could use u32 reads.
        static const int num_bits_in_elem_;

    public:
        /** Constructs a BitStream reader to the given memory area.
            \param data A pointer to the data to read. \note The memory will not be copied, but the 
            data will be read from the original buffer, so keep the data alive at least as long as 
            you're using BitStream to read from it.
            \param numBytes The number of bytes in the stream. */
        BitStream(const void *data, size_t num_bytes);

        /** Reads the given amount of bits from the stream and advances the position inside the stream.
            The bits are read from the byte array most-significant-bit first, i.e. big endian for bits, but
            if you read u16 or u32 variables, the bytes are reconstructed as LSB -first. This convention 
            comes directly from the way existing data is stored in the SLUDP protocol.
            \param count The number of bits to read, 0 <= count <= 32.
            \return The desired amount of bits packed in an u32, populating bits from the 
            least-significant-bits-end of the u32. The bits are filled in most-significant-bit first. */
        u32 ReadBits(int count);

        /// Reads a single bit from the stream and advances the current stream position.
        /// \return The next bit in the stream, or 0 if there are no bits left in the stream.
        bool ReadBit();

        /// Resets the current stream position to the beginning of the stream.
        void ResetPosition();

        /// \return The current bit position in the stream, [0, num_elems_*num_bits_in_elem_].
        size_t BitPos() const { return elem_ofs_ * num_bits_in_elem_ + bit_ofs_; }

        /// \return The number of bits left in this stream, [0, num_elems_*num_bits_in_elem_].
        size_t BitsLeft() const { return num_elems_*num_bits_in_elem_ - (elem_ofs_ * num_bits_in_elem_ + bit_ofs_); }

        /// \return num_elems_*num_bits_in_elem_ - the number of total bits in this stream.
        size_t Size() const { return num_elems_*num_bits_in_elem_; }

    private:
        /// The actual data buffer, not owned by BitStream.
        const u8 *data_;

        /// The number of total elements in the buffer.
        size_t num_elems_;

        /// The element offset the read pointer is currently at.
        int elem_ofs_;

        /// The bit index of the current element the read pointer is at. Runs up from 0 to num_bits_in_elem_-1.
        int bit_ofs_;
    };
}

#endif
