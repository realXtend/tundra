/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   PhysicsModuleFwd.h
    @brief  Forward declarations and type defines for commonly used PhysicsModule plugin classes. */

#pragma once

#include "CoreTypes.h"

/// @todo Remove the Physics namespace.
namespace Physics
{
    struct ConvexHull;
    struct ConvexHullSet;
}

using Physics::ConvexHull;
using Physics::ConvexHullSet;

class PhysicsModule;
class PhysicsWorld;
class PhysicsRaycastResult;
class EC_RigidBody;
class EC_VolumeTrigger;

typedef shared_ptr<PhysicsWorld> PhysicsWorldPtr;
typedef weak_ptr<PhysicsWorld> PhysicsWorldWeakPtr;

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
