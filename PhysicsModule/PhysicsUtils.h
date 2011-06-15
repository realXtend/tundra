// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreDefines.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "LinearMath/btQuaternion.h"
#include "LinearMath/btVector3.h"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

inline btVector3 ToBtVector3(const float3& vector)
{
    return btVector3(vector.x, vector.y, vector.z);
}

inline float3 ToVector3(const btVector3& btVector)
{
    return float3(btVector.x(), btVector.y(), btVector.z());
}

/// Simple raycast against single rigid body
/** 
    \param rayFrom origin or ray
    \param rayTo ray destination
    \param body rigid body to test against
    \return true if ray hit the rigid body, false otherwise
*/
inline bool RayTestSingle(const float3& rayFrom, const float3& rayTo, btRigidBody* body)
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

