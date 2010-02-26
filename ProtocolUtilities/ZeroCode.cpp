// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "ZeroCode.h"

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
            if (data[i] == 0)
            {
                ++i;
                if (i >= numBytes)
                {
                    ///\todo Warning log out.
                    return 0; // return 0 instead of length to signal that this packet is malformed.
                }
                size_t numZeroes = data[i++];
                
                //! \todo Additional zeroes indicate more than 256 (one byte) zeros -cm
                if (numZeroes == 0) // A run of zero zeroes? The packet is then malformed. (Actually no, see above todo -cm)
                    bool lol = true; //! REMOVE //LogWarning("Warning! A run of zero zeroes ('00 00') detected on a zeroencoded packet!");
                //return 0; ///\todo Warning log out.

                length += numZeroes;
            }
            else
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
                    ///\todo Warning log out.
                    // assert(false && "Malformed zero-encoded packet found! (Ends in a zero without run-length!)"); 
                    return false;
                }
                size_t numZeroes = srcData[src++];
                for(size_t i = 0; i < numZeroes; ++i)
                {
                    ///\todo Warning log out.
                    if (dst >= dstBytes)
                        return false; // Whoops! Caller didn't provide a buffer big enough!

                    dstData[dst++] = 0;
                }
            }
            else
            {
                ///\todo Warning log out.
                if (dst >= dstBytes)
                    return false; // Whoops! Caller didn't provide a buffer big enough!

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

                ///\todo Warning log out.
                if (dst >= dstBytes)
                    return false; // Whoops! Caller didn't provide a buffer big enough!
                dstData[dst++] = 0;
                ///\todo Warning log out.
                if (dst >= dstBytes)
                    return false; // Whoops! Caller didn't provide a buffer big enough!
                dstData[dst++] = numZeroes;
            }
            else
            {
                ///\todo Warning log out.
                if (dst >= dstBytes)
                    return false; // Whoops! Caller didn't provide a buffer big enough!
                dstData[dst++] = srcData[src++];
            }
        }

        return true;
    }

}