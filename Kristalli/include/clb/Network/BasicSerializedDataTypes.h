/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef BasicSerializedDataTypes_h
#define BasicSerializedDataTypes_h

#include <list>
#include <string>

#include "clb/Core/Types.h"

/// A single 0|1 value that is serialized as just one bit, *without padding*.
//using clb::u8; ///< a single byte: 0-255.
//using clb::s8; ///< a single byte: -128 - 127.
//using clb::u16; ///< 2 bytes: 0 - 65535.
//using clb::s16; ///< 2 bytes: -32768 - 32767.

#ifndef NAALI_CLIENT
//using clb::u32; ///< 4 bytes: 0 - 4,294,967,295 ~ 4000 million or 4e9.
//using clb::s32; ///< 4 bytes signed: max 2,147,483,647 ~ 2000 million or 2e9.
#endif

//using clb::u64; ///< 8 bytes: 18,446,744,073,709,551,615 ~1.8e19.
//using clb::s64; ///< 8 bytes signed. 9,223,372,036,854,775,807 ~ 9e18.

/// Defines the basic data types that are the building blocks of every other serializable data type.
enum BasicSerializedDataType
{
	/// Denotes an invalid value of type BasicSerializedDataType.
	SerialInvalid = 0, 

	SerialBit, ///< Associated with the type 'bit'.
	SerialU8,  ///< Associated with the type 'u8'.
	SerialS8,  ///< Associated with the type 's8'.
	SerialU16, ///< Associated with the type 'u16'.
	SerialS16, ///< Associated with the type 's16'.
	SerialU32, ///< Associated with the type 'u32'.
	SerialS32, ///< Associated with the type 's32'.
	SerialU64, ///< Associated with the type 'u64'.
	SerialS64, ///< Associated with the type 's64'.
	SerialFloat,   ///< Associated with the type 'float'.
	SerialDouble,  ///< Associated with the type 'double'.
	SerialStruct,  ///< Not associated with a basic type, but defines that the given node contains an aggregate of multiple other types.
	SerialDynamicCount, ///< Not a data type, but an identifier for the SerializedMessageIterator to return when a dynamic count of a block is the next field to fill.

	/// The maximum number of different values for BasicSerializedDataType.
	NumSerialTypes
};

/// Converts a give BasicSerializedDataType to a readable string representation.
const char *SerialTypeToString(BasicSerializedDataType type);
/// Tries to parse a BasicSerializedDataType out of a string, or returns SerialInvalid if no match.
BasicSerializedDataType StringToSerialType(const char *type);
/// Returns the number of bytes the passed BasicSerializedDataType requires for serialization.
size_t SerialTypeSize(BasicSerializedDataType type);

/// A trait class used to match basic data types to their associated enum values.
template<typename T>
struct SerializedDataTypeTraits
{
	// The specializations each have to contain the following member: (The generic template version deliberately doesn't).
	// static const BasicSerializedDataType type = x;
};

template<> struct SerializedDataTypeTraits<bit> { static const BasicSerializedDataType type = SerialBit; };
template<> struct SerializedDataTypeTraits<u8>  { static const BasicSerializedDataType type = SerialU8; };
template<> struct SerializedDataTypeTraits<s8>  { static const BasicSerializedDataType type = SerialS8; };
template<> struct SerializedDataTypeTraits<u16> { static const BasicSerializedDataType type = SerialU16; };
template<> struct SerializedDataTypeTraits<s16> { static const BasicSerializedDataType type = SerialS16; };
template<> struct SerializedDataTypeTraits<u32> { static const BasicSerializedDataType type = SerialU32; };
template<> struct SerializedDataTypeTraits<s32> { static const BasicSerializedDataType type = SerialS32; };
template<> struct SerializedDataTypeTraits<u64> { static const BasicSerializedDataType type = SerialU64; };
template<> struct SerializedDataTypeTraits<s64> { static const BasicSerializedDataType type = SerialS64; };
template<> struct SerializedDataTypeTraits<float> { static const BasicSerializedDataType type = SerialFloat; };
template<> struct SerializedDataTypeTraits<double> { static const BasicSerializedDataType type = SerialDouble; };

#endif
