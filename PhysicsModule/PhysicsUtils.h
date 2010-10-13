// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsUtils_h
#define incl_Physics_PhysicsUtils_h

#include "Core.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"

inline btVector3 ToBtVector3(const Vector3df& vector)
{
    return btVector3(vector.x, vector.y, vector.z);
}

inline Vector3df ToVector3(const btVector3& btVector)
{
    return Vector3df(btVector.x(), btVector.y(), btVector.z());
}

inline btQuaternion ToBtQuaternion(const Quaternion& quat)
{
    return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

inline Quaternion ToQuaternion(const btQuaternion& btQuat)
{
    return Quaternion(btQuat.x(), btQuat.y(), btQuat.z(), btQuat.w());
}

#endif
