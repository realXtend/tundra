// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_ZeroCode_h
#define incl_ZeroCode_h

#include "RexTypes.h"

namespace ProtocolUtilities
{

/// @return The number of bytes the given data block will take if it is zero-encoded.
size_t CountZeroEncodedLength(const uint8_t *data, size_t numBytes);

/// @return The number of bytes the given zeroencoded data block will take when it is zero-decoded
///  or 0 if the data block is malformed and can't be decoded.
size_t CountZeroDecodedLength(const uint8_t *zeroEncodedData, size_t numBytes);

/// Zero-encodes the given data block.
/// @param dstData [out] The resulting zero-encoded block will be written here.
/// @param dstBytes The maximum number of bytes that can be written to dstData.
/// @param srcData The source buffer to encode.
/// @param srcBytes The number of bytes to encode.
/// @return True if compression was successful, false if there wasn't enough space in the
///  destination buffer or if some other error occurred.
bool ZeroEncode(uint8_t *dstData, size_t dstBytes, const uint8_t *srcData, size_t srcBytes);

/// Zero-decodes the given data block.
/// @param dstData [out] The resulting zero-decoded block will be written here.
/// @param dstBytes The maximum number of bytes that can be written to dstData.
/// @param srcData The zero-encoded source buffer to decode.
/// @param srcBytes The number of bytes to decode.
/// @return True if decompression was successful, false if there wasn't enough space in the
///  destination buffer or if some other error occurred.
bool ZeroDecode(uint8_t *dstData, size_t dstBytes, const uint8_t *srcData, size_t srcBytes);

}

#endif
