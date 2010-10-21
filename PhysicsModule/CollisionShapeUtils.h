// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Physics_CollisionShapeUtils_h
#define incl_Physics_CollisionShapeUtils_h

#include "Core.h"
#include "PhysicsModuleApi.h"

class btConvexHullShape;
class btTriangleMesh;

namespace Ogre
{
    class Mesh;
};

namespace Physics
{
    struct ConvexHullSet;

    void GenerateTriangleMesh(Ogre::Mesh* mesh, btTriangleMesh* ptr, bool flipAxes);
    void GetTrianglesFromMesh(Ogre::Mesh* mesh, std::vector<Vector3df>& dest, bool flipAxes);
    void GenerateConvexHullSet(Ogre::Mesh* mesh, ConvexHullSet* ptr, bool flipAxes);
}


#endif