// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CollisionShapeUtils.h"
#include "ConvexHull.h"
#include "PhysicsUtils.h"
#include "btBulletDynamicsCommon.h"

#include "ConvexDecomposition/ConvexBuilder.h"
#include "ConvexDecomposition/ConvexDecomposition.h"

#include <Ogre.h>

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
    class ConvexResultReceiver : public ConvexDecomposition::ConvexDecompInterface
    {
    public:
        ConvexResultReceiver(ConvexHullSet* dest) : dest_(dest)
        {
        }
        
        virtual void ConvexDecompResult(ConvexDecomposition::ConvexResult &result)
        {
            ConvexHull hull;
            hull.position_ = Vector3df(0,0,0);
            
            btAlignedObjectArray<btVector3> vertices;
            
            for (uint i = 0; i < result.mHullVcount; ++i)
            {
                btVector3 vertex(result.mHullVertices[i*3],result.mHullVertices[i*3+1],result.mHullVertices[i*3+2]);
                Vector3df vertexVec = ToVector3(vertex);
                hull.position_ += vertexVec;
                vertices.push_back(vertex);
            }
            
            hull.position_ /= (float)result.mHullVcount;
            
            btVector3 positionBtVec(hull.position_.x, hull.position_.y, hull.position_.z);
            
            for (uint i = 0; i < result.mHullVcount; ++i)
                vertices[i] -= positionBtVec;
            
            hull.hull_ = boost::shared_ptr<btConvexHullShape>(new btConvexHullShape((const btScalar*)&vertices[0], vertices.size(), sizeof(btVector3)));
            dest_->hulls_.push_back(hull);
        }
        
        ConvexHullSet* dest_;
    };
    
    std::vector<Vector3df> triangles;
    GetTrianglesFromMesh(mesh, triangles, flipAxes);

    std::vector<float> vertexData;
    std::vector<uint> indexData;
    
    // Add all triangles as individual vertices, make up index data as we go along
    for (uint i = 0; i < triangles.size(); ++i)
    {
        vertexData.push_back(triangles[i].x);
        vertexData.push_back(triangles[i].y);
        vertexData.push_back(triangles[i].z);
        indexData.push_back(i);
    }
    
    ConvexResultReceiver crr(ptr);
    ConvexDecomposition::DecompDesc desc;
    desc.mVcount = vertexData.size() / 3;
    desc.mVertices = &vertexData[0];
    desc.mTcount = indexData.size() / 3;
    desc.mIndices = &indexData[0];
    // Fixed parameters
    desc.mDepth = 0;
    desc.mCpercent = 5.0f;
    desc.mPpercent = 15.0f;
    desc.mMaxVertices = 100;
    desc.mSkinWidth = 0.01f;
    desc.mCallback = &crr;
    
    ConvexBuilder cb(desc.mCallback);
    cb.process(desc);
    
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
