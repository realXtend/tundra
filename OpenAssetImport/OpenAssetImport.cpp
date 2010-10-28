// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenAssetImport.h"
#include "Renderer.h"

#include <assimp.hpp>      // C++ importer interface
#include <aiScene.h>       // Output data structure
#include <aiPostProcess.h> // Post processing flags
#include <Logger.h>
#include <DefaultLogger.h>
#include <Ogre.h>

using namespace Assimp;
using namespace Ogre;

namespace AssImp
{
    OpenAssetImport::OpenAssetImport() : 
        importer_(new Importer()), 
        logstream_(new AssImpLogStream()), 
        loglevels_(Logger::DEBUGGING | Logger::INFO | Logger::ERR | Logger::WARN)
    {
        // set up importer
        importer_->SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);	// limit bone weights to 4 vertices
		
        importer_->SetPropertyInteger(			// ignore vertex colours, textures, lights and cameras (for now)
            AI_CONFIG_PP_RVC_FLAGS, 
            aiComponent_COLORS		|
            aiComponent_TEXTURES	|
            aiComponent_LIGHTS		|
            aiComponent_CAMERAS
            );

        importer_->SetPropertyInteger(			// ignore point and line primitives (for now)
            AI_CONFIG_PP_SBP_REMOVE, 
            aiPrimitiveType_POINT		|
            aiPrimitiveType_LINE
            );

        Assimp::DefaultLogger::get()->attachStream(logstream_, loglevels_);
    }

    OpenAssetImport::~OpenAssetImport()
    {
        Assimp::DefaultLogger::get()->detatchStream(logstream_, loglevels_);
        delete logstream_;
    }

    bool OpenAssetImport::IsSupportedExtension(const QString& extension)
    {
        return importer_->IsExtensionSupported(extension.toStdString());
    }

    void OpenAssetImport::Import(Foundation::Framework *framework, const QString& file, std::vector<std::string> &outMeshNames)
    {
        const aiScene *scene = importer_->ReadFile(
            file.toStdString(), 
            aiProcess_JoinIdenticalVertices		|
            aiProcess_Triangulate				|
            aiProcess_RemoveComponent			|
            aiProcess_GenNormals				|	// ignored if model already has normals
            aiProcess_LimitBoneWeights			|
            aiProcess_SortByPType				|	// remove point and line primitive types
            aiProcess_ValidateDataStructure         // makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct...
            );

        if (scene)
        {
            
            ImportScene(framework, scene, file, outMeshNames);
        } else
        {       
            // report error
            Foundation::RootLogError(importer_->GetErrorString());
            //return QString(importer_->GetErrorString());
        }
    }

    void OpenAssetImport::ImportScene(Foundation::Framework *framework, const struct aiScene *scene, const QString& file, std::vector<std::string> &outMeshNames)
    {
        const struct aiNode *rootNode = scene->mRootNode;
        
        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework->GetServiceManager()->
            GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        
        if (renderer)
        {
            ImportNode(renderer, scene, rootNode, file, outMeshNames);
        }
	}

    void OpenAssetImport::ImportNode(const boost::shared_ptr<OgreRenderer::Renderer> &renderer, const aiScene *scene, 
        const aiNode *node, const QString& file, std::vector<std::string> &outMeshNames)
    {
        try
        {
            boost::filesystem::path path(file.toStdString());
            outMeshNames.push_back(path.filename());
            std::string meshname = std::string("mesh___") + path.filename();


            aiMatrix4x4 transform = node->mTransformation;
            Ogre::MeshPtr ogreMesh = Ogre::MeshManager::getSingleton().createManual(meshname, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
            ogreMesh->setAutoBuildEdgeLists(false);

            Ogre::Vector3 vmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            Ogre::Vector3 vmax(std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min());

            for (unsigned int i=0 ; i<node->mNumMeshes ; ++i)
            {
                const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                
                SubMesh *ogreSubmesh = ogreMesh->createSubMesh();
                
                ogreSubmesh->useSharedVertices = false;
                ogreSubmesh->vertexData = new Ogre::VertexData();
                ogreSubmesh->vertexData->vertexCount = mesh->mNumVertices;
                Ogre::VertexData *data = ogreSubmesh->vertexData;
                
                // Vertex declarations
                size_t offset = 0;
                Ogre::VertexDeclaration* decl = data->vertexDeclaration;
                decl->addElement(0, offset, VET_FLOAT3, VES_POSITION);
                offset += VertexElement::getTypeSize(VET_FLOAT3);
                decl->addElement(0, offset, VET_FLOAT3, VES_NORMAL);

                offset = 0;
                for (int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
                {
                    if (mesh->mTextureCoords[tn])
                    {
                        if (mesh->mNumUVComponents[tn] == 3)
                        {
                            decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, tn);
                            offset += VertexElement::getTypeSize(VET_FLOAT3);
                        } else
                        {
                            decl->addElement(1, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, tn);
                            offset += VertexElement::getTypeSize(VET_FLOAT2);
                        }
                    }
                }
                if (mesh->HasTangentsAndBitangents())
                {
                    decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
                    offset += VertexElement::getTypeSize(VET_FLOAT3);
                    decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_BINORMAL);
                }

                // Write vertex data to buffer
                Ogre::HardwareVertexBufferSharedPtr vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(0),                     // This value is the size of a vertex in memory
                    data->vertexCount,                          // The number of vertices you'll put into this buffer
                    Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
                    );
                Ogre::Real *vbData = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                
                offset = 0;
                for (unsigned int n=0 ; n<data->vertexCount ; ++n)
                {
                    vbData[offset++] = mesh->mVertices[n].x;
                    vbData[offset++] = mesh->mVertices[n].y;
                    vbData[offset++] = mesh->mVertices[n].z;

                    vbData[offset++] = mesh->mNormals[n].x;
                    vbData[offset++] = mesh->mNormals[n].y;
                    vbData[offset++] = mesh->mNormals[n].z;

                    vmin.x = std::min(vmin.x, mesh->mVertices[n].x);
                    vmin.y = std::min(vmin.y, mesh->mVertices[n].y);
                    vmin.z = std::min(vmin.z, mesh->mVertices[n].z);

                    vmax.x = std::max(vmax.x, mesh->mVertices[n].x);
                    vmax.y = std::max(vmax.y, mesh->mVertices[n].y);
                    vmax.z = std::max(vmax.z, mesh->mVertices[n].z);
                }
                vbuf->unlock();
                data->vertexBufferBinding->setBinding(0, vbuf);
                

                vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                    decl->getVertexSize(1),                     // This value is the size of a vertex in memory
                    data->vertexCount,                          // The number of vertices you'll put into this buffer
                    Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
                    );
                vbData = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                
                offset = 0;
                for (unsigned int n=0 ; n<data->vertexCount ; ++n)
                {
                    for (int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
                    {
                        if (mesh->mTextureCoords[tn])
                        {
                            if (mesh->mNumUVComponents[tn] == 3)
                            {
                                vbData[offset++] = mesh->mTextureCoords[tn][n].x;
                                vbData[offset++] = mesh->mTextureCoords[tn][n].y;
                                vbData[offset++] = mesh->mTextureCoords[tn][n].z;
                            } else
                            {
                                vbData[offset++] = mesh->mTextureCoords[tn][n].x;
                                vbData[offset++] = mesh->mTextureCoords[tn][n].y;
                            }
                        }
                    }

                    if (mesh->HasTangentsAndBitangents())
                    {
                        vbData[offset++] = mesh->mTangents[n].x;
                        vbData[offset++] = mesh->mTangents[n].y;
                        vbData[offset++] = mesh->mTangents[n].z;

                        vbData[offset++] = mesh->mBitangents[n].x;
                        vbData[offset++] = mesh->mBitangents[n].y;
                        vbData[offset++] = mesh->mBitangents[n].z;
                    }
                }
                vbuf->unlock();
                data->vertexBufferBinding->setBinding(1, vbuf);

                // indices
                size_t numIndices = mesh->mNumFaces * 3;            // support only triangles, so 3 indices per face
                
                Ogre::HardwareIndexBuffer::IndexType idxType = Ogre::HardwareIndexBuffer::IT_16BIT;
                if (numIndices > 65535)
                    idxType = Ogre::HardwareIndexBuffer::IT_32BIT;

                Ogre::HardwareIndexBufferSharedPtr ibuf = Ogre::HardwareBufferManager::getSingleton().createIndexBuffer(
                    idxType,                                        // You can use several different value types here
                    numIndices,                                     // The number of indices you'll put in that buffer
                    Ogre::HardwareBuffer::HBU_DYNAMIC     // Properties
                    );
                
                if (idxType == Ogre::HardwareIndexBuffer::IT_16BIT)
                {
                    u16 *idxData = static_cast<u16*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                    offset = 0;
                    for (int n=0 ; n<mesh->mNumFaces ; ++n)
                    {
                        idxData[offset++] = mesh->mFaces[n].mIndices[0];
                        idxData[offset++] = mesh->mFaces[n].mIndices[1];
                        idxData[offset++] = mesh->mFaces[n].mIndices[2];
                    }
                } else
                {
                    u32 *idxData = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                    offset = 0;
                    for (int n=0 ; n<mesh->mNumFaces ; ++n)
                    {
                        idxData[offset++] = mesh->mFaces[n].mIndices[0];
                        idxData[offset++] = mesh->mFaces[n].mIndices[1];
                        idxData[offset++] = mesh->mFaces[n].mIndices[2];
                    }
                }
                ibuf->unlock();

                ogreSubmesh->indexData->indexBuffer = ibuf;         // The pointer to the index buffer
                ogreSubmesh->indexData->indexCount = numIndices;    // The number of indices we'll use
                ogreSubmesh->indexData->indexStart = 0;
            }

            ogreMesh->_setBounds(Ogre::AxisAlignedBox(vmin, vmax));
            Ogre::Real maxvertex = std::max(abs(vmax.x), std::max(abs(vmin.x), std::max(abs(vmax.y), std::max(abs(vmin.y), std::max(vmax.z, vmin.z)))));
            ogreMesh->_setBoundingSphereRadius(maxvertex / 2.f);
            ogreMesh->load();
        } catch (Ogre::Exception &e)
        {
            // error
        }

        // import children
        for (int i=0 ; i<node->mNumChildren ; ++i)
            ImportNode(renderer, scene, node->mChildren[i], file, outMeshNames);
    }
    
    void OpenAssetImport::AssImpLogStream::write(const char* message)
    {
        Foundation::RootLogInfo(message);
    }
}
