// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreDefines.h"
#include "PhysicsModuleApi.h"
#include "Math/float3.h"

class btConvexHullShape;
class btTriangleMesh;

namespace Ogre
{
    class Mesh;
};

namespace Physics
{
    struct ConvexHullSet;

    void GenerateTriangleMesh(Ogre::Mesh* mesh, btTriangleMesh* ptr);
    void GetTrianglesFromMesh(Ogre::Mesh* mesh, std::vector<float3>& dest);
    void GenerateConvexHullSet(Ogre::Mesh* mesh, ConvexHullSet* ptr);
}


