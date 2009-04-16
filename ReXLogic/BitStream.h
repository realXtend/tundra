// For conditions of distribution and use, see copyright notice in license.txt

#ifndef BitStream_h
#define BitStream_h

namespace RexLogic
{

    /// A nice interface for reading individual bits in a stream
    class BitStream
    {
    private:
        static const int cNumBitsInElem = 8;
    public:

        /// Constructs a BitStream reader. The memory will not be copied, but the data will be read from the original buffer,
        /// so keep the data alive at least as long as you're using BitStream to read from it.
        /// \bug If numBits % 8 != 0, reading the last byte from a bit fails (returns MSB's that are bogus, instead of the correct LSB's).
        BitStream(const void *data, size_t numBytes);

        /// Reads the given amount of bits off the stream (and advances the position inside the stream).
        /// @param count The number of bits to read, 1 <= count <= 32.
        /// @return The bits organized in a variable. The bit ordering will be reversed, i.e. the first bit in the stream
        ///         will be set to be the MSB in the returned variable, and the last bit will be the LSB.
        Core::u32 ReadBits(int count);

        /// @return The next bit in the stream. Advances the current stream position. If no bits left in the stream, returns 0.
        bool ReadBit();

        /// Resets the current stream position to the beginning of the stream.
        void ResetPosition();

        /// @return The current bit position in the stream, [0, Size[.
        size_t BitPos() const { return elemOfs_ * cNumBitsInElem + bitOfs_; }

        /// @return The number of bits left in this stream.
        size_t BitsLeft() const { return numBits_ - (elemOfs_ * cNumBitsInElem + bitOfs_); }

        /// @return The number of total bits in this stream.
        size_t Size() const { return numBits_; }

    private:
        const Core::u8 *data_;
        size_t numBits_;
        size_t numElems_;
        int elemOfs_;
        int bitOfs_;
    };
}

#endif
