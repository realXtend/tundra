// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "Math/float3.h"

#include <LinearMath/btQuaternion.h>
#include <LinearMath/btVector3.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/CollisionDispatch/btCollisionWorld.h>

/// Simple raycast against single rigid body
/** @param rayFrom origin of ray
    @param rayTo ray destination
    @param body rigid body to test against
    @return true if ray hit the rigid body, false otherwise */
inline bool RayTestSingle(const float3& rayFrom, const float3& rayTo, btRigidBody* body)
{
    assert (body);

    btTransform rayFromTrans, rayToTrans;
    btVector3 btRayFrom(rayFrom.x, rayFrom.y, rayFrom.z);
    btVector3 btRayTo(rayTo.x, rayTo.y, rayTo.z);

    rayFromTrans.setIdentity();
    rayFromTrans.setOrigin(btRayFrom);
    rayToTrans.setIdentity();
    rayToTrans.setOrigin(btRayTo);

    btCollisionWorld::ClosestRayResultCallback resultCallback(btRayFrom,btRayTo);

    btCollisionWorld::rayTestSingle(rayFromTrans, rayToTrans, body,
        body->getCollisionShape(), body->getWorldTransform(), resultCallback);

    return resultCallback.hasHit();
}

