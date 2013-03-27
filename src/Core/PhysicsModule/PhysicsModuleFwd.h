/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   PhysicsModuleFwd.h
    @brief  Forward declarations and type defines for commonly used PhysicsModule plugin classes. */

#pragma once

#include "CoreTypes.h"

/// @todo Remove the Physics namespace.
namespace Physics
{
    class PhysicsModule;
    class PhysicsWorld;
    struct ConvexHull;
    struct ConvexHullSet;
}

class PhysicsRaycastResult;
class EC_RigidBody;
class EC_VolumeTrigger;

typedef shared_ptr<Physics::PhysicsWorld> PhysicsWorldPtr;
typedef weak_ptr<Physics::PhysicsWorld> PhysicsWorldWeakPtr;

// From Bullet:
class btTriangleMesh;
class btCollisionConfiguration;
class btBroadphaseInterface;
class btConstraintSolver;
class btDiscreteDynamicsWorld;
class btDispatcher;
class btCollisionObject;
class btConvexHullShape;
class btRigidBody;
class btCollisionShape;
class btHeightfieldTerrainShape;
