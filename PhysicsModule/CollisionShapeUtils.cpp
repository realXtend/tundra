// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "PhysicsUtils.h"
#include "btBulletDynamicsCommon.h"
#include "LoggingFunctions.h"

#include "hull.h"

#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("CollisionShapeUtils")

namespace Physics
{

void GenerateTriangleMesh(Ogre::Mesh* mesh, btTriangleMesh* ptr, bool flipAxes)
{
    std::vector<Vector3df> triangles;
    GetTrianglesFromMesh(mesh, triangles, flipAxes);
    
    for (uint i = 0; i < triangles.size(); i += 3)
        ptr->addTriangle(ToBtVector3(triangles[i]), ToBtVector3(triangles[i+1]), ToBtVector3(triangles[i+2]));
}

void GenerateConvexHullSet(Ogre::Mesh* mesh, ConvexHullSet* ptr, bool flipAxes)
{
    std::vector<Vector3df> vertices;
    GetTrianglesFromMesh(mesh, vertices, flipAxes);
    if (!vertices.size())
    {
        LogError("Mesh had no triangles; aborting convex hull generation");
        return;
    }
    
    StanHull::HullDesc desc;
    desc.SetHullFlag(StanHull::QF_TRIANGLES);
    desc.mVcount = vertices.size();
    desc.mVertices = &vertices[0].x;
    desc.mVertexStride = sizeof(Vector3df);
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
    hull.position_ = Vector3df(0,0,0);
    /// \todo StanHull always produces only 1 hull. Therefore using a hull set is unnecessary and could be optimized away
    hull.hull_ = boost::shared_ptr<btConvexHullShape>(new btConvexHullShape((const btScalar*)&result.mOutputVertices[0], result.mNumOutputVertices, 3 * sizeof(float)));
    ptr->hulls_.push_back(hull);
    
    lib.ReleaseResult(result);
}

void GetTrianglesFromMesh(Ogre::Mesh* mesh, std::vector<Vector3df>& dest, bool flipAxes)
{
    dest.clear();
    
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
        unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
        unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);
        bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        
        if (use32bitindexes)
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pLong[k];
                uint i2 = pLong[k+1];
                uint i3 = pLong[k+2];
                
                //! Haxor the collision mesh for the Ogre->Opensim coordinate space adjust
                /*! \todo Hopefully the need for this is eliminated soon
                 */
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
                

            }
        }
        else
        {
            for(size_t k = 0; k < numTris * 3; k += 3)
            {
                uint i1 = pShort[k];
                uint i2 = pShort[k+1];
                uint i3 = pShort[k+2];
                
                //! Haxor the collision mesh for the Ogre->Opensim coordinate space adjust
                /*! \todo Hopefully the need for this is eliminated soon
                 */
                posElem->baseVertexPointerToElement(vertices + i1 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i2 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
                    
                posElem->baseVertexPointerToElement(vertices + i3 * vertexSize, &pReal);
                if (flipAxes)
                    dest.push_back(Vector3df(-pReal[0], pReal[2], pReal[1]));
                else
                    dest.push_back(Vector3df(pReal[0], pReal[1], pReal[2]));
            }
        }
        
        vbuf->unlock();
        ibuf->unlock();
    }
}

}
