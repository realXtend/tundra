// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "ZeroCode.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("ZeroCode")

namespace ProtocolUtilities
{

    size_t CountConsecutiveZeroes(const uint8_t *data, size_t i, size_t numBytes)
    {
        size_t count = 0;
        while(i < numBytes && data[i] == 0)
        {
            ++count;
            ++i;
        }
        return count;
    }

    size_t CountZeroEncodedLength(const uint8_t *data, size_t numBytes)
    {
        size_t length = 0;
        
        size_t i = 0;
        while(i < numBytes)
        {
            if (data[i] == 0) // Hit a zero?
            {
                size_t numZeroes = CountConsecutiveZeroes(data, i, numBytes);
                length += 1 + numZeroes;
                i += numZeroes;
            }
            else
            {
                ++length;
                ++i;
            }
        }
        return length;
    }

    size_t CountZeroDecodedLength(const uint8_t *data, size_t numBytes)
    {
        size_t length = 0;

        size_t i = 0;
        while(i < numBytes)
        {
            // If we encounter a zero, the next byte in the stream tells us how many times we duplicate that zero.
            if (data[i] == 0)
            {
                ++i;
                if (i >= numBytes)
                {
                    LogWarning("Oops! We received a stream where the last byte was zero. We should have had a length byte after this.. Malformed packet!");
                    return 0; // return 0 instead of length to signal that this packet is malformed.
                }

                size_t numZeroes = data[i++];
                if (numZeroes == 0) // A run of zero zeroes? The packet is then malformed.
                    return 0; // \todo Have heard of rumors that a sequence '00 00 AA BB' would signal a larger block of zeroes, e.g. using a u16 as the length counter.
                              //       libopenmetaverse's code doesn't do this however, so we conclude this case to result in a corrupted stream. 
                length += numZeroes;
            }
            else // A normal non-zero byte.
            {
                ++length;
                ++i;
            }
        }
        return length;
    }

    bool ZeroDecode(uint8_t *dstData, size_t dstBytes, const uint8_t *srcData, size_t srcBytes)
    {
        size_t dst = 0;
        size_t src = 0;

        while(src < srcBytes)
        {
            if (srcData[src] == 0)
            {
                ++src;
                if (src >= srcBytes)
                {
                    LogWarning("Malformed zero-encoded packet found! (Ends in a zero without run-length!");
                    return false;
                }

                size_t numZeroes = srcData[src++];
                for(size_t i = 0; i < numZeroes; ++i)
                {
                    if (dst >= dstBytes)
                    {
                        LogWarning("Whoops! Caller didn't provide a buffer big enough!");
                        return false;
                    }

                    dstData[dst++] = 0;
                }
            }
            else
            {
                if (dst >= dstBytes)
                {
                    LogWarning("Whoops! Caller didn't provide a buffer big enough!");
                    return false;
                }

                dstData[dst++] = srcData[src++];
            }
        }

        return true;
    }

    bool ZeroEncode(uint8_t *dstData, size_t dstBytes, const uint8_t *srcData, size_t srcBytes)
    {
        size_t dst = 0;
        size_t src = 0;
        while(src < srcBytes)
        {
            if (srcData[src] == 0)
            {
                size_t numZeroes = CountConsecutiveZeroes(srcData, src, srcBytes);
                src += numZeroes;

                ///\bug Fails on a run of 256 or more zeroes.
                if (dst >= dstBytes)
                {
                    LogWarning("Whoops! Caller didn't provide a buffer big enough!");
                    return false;
                }
                dstData[dst++] = 0;
                if (dst >= dstBytes)
                {
                    LogWarning("Whoops! Caller didn't provide a buffer big enough!");
                    return false;
                }
                dstData[dst++] = numZeroes;
            }
            else
            {
                if (dst >= dstBytes)
                {
                    LogWarning("Whoops! Caller didn't provide a buffer big enough!");
                    return false;
                }
                dstData[dst++] = srcData[src++];
            }
        }

        return true;
    }
}
