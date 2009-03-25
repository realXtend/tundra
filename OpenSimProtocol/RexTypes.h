// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexTypes_h
#define incl_RexTypes_h

#include "CoreDefines.h"

#include "boost/cstdint.hpp"

///\todo Move all these to core framework headers.
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;
using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

///\todo Create own classes/use core classes.
struct Vector3
{
	float x;
	float y;
	float z;
};

struct Vector3d
{
	double x;
	double y;
	double z;
};

struct Vector4
{
	float x;
	float y;
	float z;
	float w;
};

struct Quaternion
{
	float x;
	float y;
	float z;
	float w;
};


#endif
