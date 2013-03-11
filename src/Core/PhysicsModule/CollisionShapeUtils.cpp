// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_BULLET_INTEROP
#include "DebugOperatorNew.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "PhysicsUtils.h"
#include "LoggingFunctions.h"
#include "hull.h"

#include <Ogre.h>

// Disable unreferenced formal parameter coming from Bullet
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#endif
#include <btBulletDynamicsCommon.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "MemoryLeakCheck.h"

namespace Physics
{

void GenerateTriangleMesh(Ogre::Mesh* mesh, btTriangleMesh* ptr)
{
    std::vector<float3> triangles;
    GetTrianglesFromMesh(mesh, triangles);
    
    for(uint i = 0; i < triangles.size(); i += 3)
        ptr->addTriangle(triangles[i], triangles[i+1], triangles[i+2]);
}

void GenerateConvexHullSet(Ogre::Mesh* mesh, ConvexHullSet* ptr)
{
    std::vector<float3> vertices;
    GetTrianglesFromMesh(mesh, vertices);
    if (!vertices.size())
    {
        LogError("Mesh had no triangles; aborting convex hull generation");
        return;
    }
    
    StanHull::HullDesc desc;
    desc.SetHullFlag(StanHull::QF_TRIANGLES);
    desc.mVcount = vertices.size();
    desc.mVertices = &vertices[0].x;
    desc.mVertexStride = sizeof(float3);
    desc.mSkinWidth = 0.01f; // Hardcoded skin width
    
    StanHull::HullLibrary lib;
    StanHull::HullResult result;
    lib.CreateConvexHull(desc, result);

    if (!result.mNumOutputVertices)
    {
        LogError("No vertices were generated; aborting convex hull generation");
        return;
    }
    
    ConvexHull hull;
    hull.position_ = float3(0,0,0);
    /// \todo StanHull always produces only 1 hull. Therefore using a hull set is unnecessary and could be optimized away
    hull.hull_ = shared_ptr<btConvexHullShape>(new btConvexHullShape((const btScalar*)&result.mOutputVertices[0], result.mNumOutputVertices, 3 * sizeof(float)));
    ptr->hulls_.push_back(hull);
    
    lib.ReleaseResult(result);
}

void GetTrianglesFromMesh(Ogre::Mesh* mesh, std::vector<float3>& dest)
{
    dest.clear();
    
    try
    {

    for(uint i = 0; i < mesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh* submesh = mesh->getSubMesh(i);
        
        Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;
        const Ogre::VertexElement* posElem = vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        Ogre::HardwareVertexBufferSharedPtr vbuf = vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());
        unsigned char* vertices = static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        uint vertexSize = vbuf->getVertexSize();
        float* pReal = 0;
        
        Ogre::IndexData* index_data = submesh->indexData;
        size_t numTris = index_data->indexCount / 3;
        Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;
        u32*  pLong = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        u16* pShort = reinterpret_cast<u16*>(pLong);
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        
        if (use32bitindexes)
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pLong[k];
                uint i2 = pLong[k+1];
                uint i3 = pLong[k+2];
                
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
                

            }
        }
        else
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pShort[k];
                uint i2 = pShort[k+1];
                uint i3 = pShort[k+2];
                
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                dest.push_back(float3(pReal[0], pReal[1], pReal[2]));
            }
        }
        
        vbuf->unlock();
        ibuf->unlock();
    }

    } catch(Ogre::Exception &e)
    {
        ///\todo Fix Ogre to not allow meshes like this to be successfully created.
        LogError("GetTrianglesFromMesh failed for mesh! Ogre threw an exception: " + QString(e.what()));
        dest.clear();
    }
}

}
