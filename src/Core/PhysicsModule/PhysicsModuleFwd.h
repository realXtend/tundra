/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   PhysicsModuleFwd.h
    @brief  Forward declarations and type defines for commonly used PhysicsModule plugin classes. */

#pragma once

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
