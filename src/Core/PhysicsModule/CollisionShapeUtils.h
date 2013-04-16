// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "PhysicsModuleApi.h"
#include "PhysicsModuleFwd.h"
#include "Math/float3.h"

namespace Ogre { class Mesh; };

namespace Physics
{
    void PHYSICS_MODULE_API GenerateTriangleMesh(Ogre::Mesh* mesh, btTriangleMesh* ptr);
    void PHYSICS_MODULE_API GetTrianglesFromMesh(Ogre::Mesh* mesh, std::vector<float3>& dest);
    void PHYSICS_MODULE_API GenerateConvexHullSet(Ogre::Mesh* mesh, ConvexHullSet* ptr);
}
