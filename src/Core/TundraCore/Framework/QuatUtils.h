/// @file QuatUtils.h
/// @brief Additional utility functions not found in Irrlicht package for working with quaternions.
/// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "Math/Quat.h"
#include "Math/float3.h"
#include "CoreTypes.h"
/*
Quaternion UnpackQuaternionFromFloat3(float x, float y, float z);

__inline Quaternion UnpackQuaternionFromFloat3(const float *data) { return UnpackQuaternionFromFloat3(data[0], data[1], data[2]); }
__inline Quaternion UnpackQuaternionFromFloat3(const Vector3D<float> &data) { return UnpackQuaternionFromFloat3(data.x, data.y, data.z); }

Vector3D<float> PackQuaternionToFloat3(float x, float y, float z, float w);

__inline Vector3D<float> PackQuaternionToFloat3(const float *data) { return PackQuaternionToFloat3(data[0], data[1], data[2], data[3]); }
__inline Vector3D<float> PackQuaternionToFloat3(const Quat &data) { return PackQuaternionToFloat3(data.x, data.y, data.z, data.w); }

Quaternion UnpackQuaternionFromU16_4(u16 x,u16 y,u16 z,u16 w);

__inline Quaternion UnpackQuaternionFromU16_4(const u16 *data) { return UnpackQuaternionFromU16_4(data[0], data[1], data[2], data[3]); }
*/