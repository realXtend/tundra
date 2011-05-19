// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_PhysicsUtils_h
#define incl_Physics_PhysicsUtils_h

#include "CoreDefines.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

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

/// Simple raycast against single rigid body
/** 
    \param rayFrom origin or ray
    \param rayTo ray destination
    \param body rigid body to test against
    \return true if ray hit the rigid body, false otherwise
*/
inline bool RayTestSingle(const Vector3df& rayFrom, const Vector3df& rayTo, btRigidBody* body)
{
    assert (body);

    btTransform rayFromTrans, rayToTrans;
    btVector3 btRayFrom(ToBtVector3(rayFrom));
    btVector3 btRayTo(ToBtVector3(rayTo));

    rayFromTrans.setIdentity();
    rayFromTrans.setOrigin(btRayFrom);
    rayToTrans.setIdentity();
    rayToTrans.setOrigin(btRayTo);

    btCollisionWorld::ClosestRayResultCallback resultCallback(btRayFrom,btRayTo);

    btCollisionWorld::rayTestSingle(rayFromTrans, rayToTrans, body, 
                    body->getCollisionShape(), body->getWorldTransform(), resultCallback);

    return resultCallback.hasHit();
}

#endif
