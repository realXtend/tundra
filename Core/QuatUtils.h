/// @file QuatUtils.h
/// @brief Additional utility functions not found in Irrlicht package for working with quaternions.
/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef Core_QuatUtils_h
#define Core_QuatUtils_h

#include "Quaternion.h"
#include "Vector3D.h"

namespace Core
{
    Quaternion UnpackQuaternionFromFloat3(float x, float y, float z);

    __inline Quaternion UnpackQuaternionFromFloat3(const float *data) { return UnpackQuaternionFromFloat3(data[0], data[1], data[2]); }
    __inline Quaternion UnpackQuaternionFromFloat3(const Vector3D<float> &data) { return UnpackQuaternionFromFloat3(data.x, data.y, data.z); }


}

#endif
