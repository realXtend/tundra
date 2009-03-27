// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexTypes_h
#define incl_RexTypes_h

#include "CoreStableHeaders.h"
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

namespace RexTypes
{   

typedef Core::Vector3D<float> Vector3;
typedef Core::Vector3D<double> Vector3d;

///\todo Is this needed? Only one message uses Vector4 (CameraConstraint).
struct Vector4
{
    void Null() { x = 0; y = 0; z = 0; x = w;}
	float x;
	float y;
	float z;
	float w;
};

}

#endif
