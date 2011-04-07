// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OpenAssetImport.h"
#include "SceneDesc.h"
#include "LoggingFunctions.h"
//DEFINE_POCO_LOGGING_FUNCTIONS("OpenAssetImport")

#include <assimp.hpp>      // C++ importer interface
#include <aiScene.h>       // Output data structure
#include <aiPostProcess.h> // Post processing flags
#include <Logger.h>
#include <DefaultLogger.h>
#include <Ogre.h>

#include "MemoryLeakCheck.h"

using namespace Assimp;

namespace AssImp
{
    OpenAssetImport::OpenAssetImport() :
        importer_(new Importer()),
#include "DisableMemoryLeakCheck.h"
        logstream_(new AssImpLogStream()),
#include "EnableMemoryLeakCheck.h"
#ifdef _DEBUG
        loglevels_(Logger::DEBUGGING | Logger::INFO | Logger::ERR | Logger::WARN),
#else
        loglevels_(Logger::INFO | Logger::ERR | Logger::WARN),
#endif
        default_flags_( aiProcess_JoinIdenticalVertices     |
                        aiProcess_Triangulate               |
                        aiProcess_RemoveComponent           |
                        aiProcess_GenSmoothNormals          |   // ignored if model already has normals
                        aiProcess_LimitBoneWeights          |
                        aiProcess_FlipUVs                   |   // UVs are upside down
                        aiProcess_GenUVCoords               |   // for formats with pre-defined UV mappings (sphere, cylindrical...), convert to proper UV channels
                        aiProcess_SortByPType               |   // remove point and line primitive types
                        aiProcess_ValidateDataStructure         // makes sure that all indices are valid, all animations and bones are linked correctly, all material references are correct...
                      )
    {
        // set up importer
        importer_->SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 4);    // limit bone weights to 4 vertices
        
        importer_->SetPropertyInteger(  // ignore vertex colours, textures, lights and cameras (for now)
            AI_CONFIG_PP_RVC_FLAGS,
            aiComponent_COLORS      |
            aiComponent_TEXTURES    |
            aiComponent_LIGHTS      |
            aiComponent_CAMERAS
            );

        importer_->SetPropertyInteger(  // ignore point and line primitives (for now)
            AI_CONFIG_PP_SBP_REMOVE,
            aiPrimitiveType_POINT   |
            aiPrimitiveType_LINE
            );
#ifdef _DEBUG
        DefaultLogger::create("", Logger::VERBOSE); // enable debug log messages
#else
        DefaultLogger::create("", Logger::NORMAL); // enable normal log messages
#endif
        Assimp::DefaultLogger::get()->attachStream(logstream_, loglevels_);
    }

    OpenAssetImport::~OpenAssetImport()
    {
        Assimp::DefaultLogger::get()->detatchStream(logstream_, loglevels_);
        delete logstream_;
        DefaultLogger::kill();
    }

    void OpenAssetImport::StripMeshnameFromAssetId(const QString& id, QString &outfile, QString &outMeshname)
    {
        int lastSlash = id.lastIndexOf('/');

        if (lastSlash != -1 && lastSlash > 6)
        {
            outfile = id.left(lastSlash);
            outMeshname = id.mid(lastSlash + 1);
        }
    }

    bool OpenAssetImport::IsSupportedExtension(const QString& filename)
    {
        boost::filesystem::path path(filename.toStdString());
        QString extension = QString(path.extension().c_str()).toLower();

        return importer_->IsExtensionSupported(extension.toStdString());
    }

    void OpenAssetImport::GetMeshData(const QString& file, std::vector<MeshData> &outMeshData) const
    {
        const aiScene *scene = importer_->ReadFile(file.toStdString(), default_flags_);

        if (scene)
        {
            const struct aiNode *rootNode = scene->mRootNode;
            
            aiMatrix4x4 transform;
            transform.FromEulerAnglesXYZ(90 * DEGTORAD, 0, 180 * DEGTORAD);
            GetNodeData(scene, rootNode, file, transform, outMeshData);
        } else
        {
            // report error
            LogError(importer_->GetErrorString());
        }
    }

    void OpenAssetImport::Import(const void *data, size_t length, const QString &name, const char* hint, const QString &nodeName, std::vector<std::string> &outMeshNames)
    {
        const aiScene *scene = importer_->ReadFileFromMemory(data, length, default_flags_, hint);

        if (scene)
        {
            const struct aiNode *rootNode = scene->mRootNode;
            ImportNode(scene, rootNode, name, nodeName, outMeshNames);
        } else
        {
            // report error
            LogError(importer_->GetErrorString());
        }
    }

    SceneDesc OpenAssetImport::GetSceneDescription(const QString &filename) const
    {
        ///\todo Implement
        /*
        std::vector<MeshData> meshNames;
        GetMeshData(filename, meshNames);
        foreach(MeshData mdata, meshNames)
        {
        }
        */
        return SceneDesc();
    }

    void OpenAssetImport::GetNodeData(const aiScene *scene, const aiNode *node, const QString& file,
        const aiMatrix4x4 &parentTransform, std::vector<MeshData> &outMeshNames) const
    {
        aiMatrix4x4 aiTransform = node->mTransformation;

        aiMatrix4x4 worldTransform =  parentTransform * aiTransform;
        
        aiVector3D pos;
        aiVector3D scale;
        Vector3df rote;
        aiQuaternion rotq;
        worldTransform.Decompose(scale, rotq, pos);
        Quaternion quat(rotq.x, rotq.y, rotq.z, rotq.w);
        
        quat.toEuler(rote);
        rote *= RADTODEG;

        if (node->mNumMeshes > 0)
        {
            MeshData data = { file, QString(node->mName.data), Transform(Vector3df(pos.x, pos.y, pos.z), rote, Vector3df(scale.x, scale.y, scale.z)) };
            outMeshNames.push_back(data);     
        }

        // import children
        for(int i=0 ; i<node->mNumChildren ; ++i)
            GetNodeData(scene, node->mChildren[i], file, worldTransform, outMeshNames);
    }

    void OpenAssetImport::ImportNode(const aiScene *scene, const aiNode *node, const QString& file,
        const QString &nodeName, std::vector<std::string> &outMeshNames)
    {
        try
        {
            std::string ogreMeshName = file.toStdString();

            /*if (scene->mNumMaterials > 0)
            {
                // testing getting Ogre material.xml names
                aiString name;
                scene->mMaterials[0]->Get(AI_MATKEY_NAME,name);
                LogInfo(std::string("Found material ") + std::string(name.data));
            }*/


            //aiMatrix4x4 transform = node->mTransformation;
            if (node->mNumMeshes > 0 && (nodeName.isEmpty() || nodeName.compare(QString(node->mName.data)) == 0) && !Ogre::MeshManager::getSingleton().resourceExists(ogreMeshName))
            {
                Ogre::MeshPtr ogreMesh = Ogre::MeshManager::getSingleton().createManual(ogreMeshName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
                ogreMesh->setAutoBuildEdgeLists(false);

                Ogre::Vector3 vmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
                Ogre::Vector3 vmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

                for(unsigned int i=0 ; i<node->mNumMeshes ; ++i)
                {
                    const aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
                    
                    Ogre::SubMesh *ogreSubmesh = ogreMesh->createSubMesh();
                    
                    ogreSubmesh->useSharedVertices = false;
#include "DisableMemoryLeakCheck.h"
                    ogreSubmesh->vertexData = new Ogre::VertexData();
#include "EnableMemoryLeakCheck.h"
                    ogreSubmesh->vertexData->vertexCount = mesh->mNumVertices;
                    Ogre::VertexData *data = ogreSubmesh->vertexData;
                    
                    // Vertex declarations
                    size_t offset = 0;
                    Ogre::VertexDeclaration* decl = data->vertexDeclaration;
                    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_POSITION);
                    offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
                    decl->addElement(0, offset, Ogre::VET_FLOAT3, Ogre::VES_NORMAL);

                    offset = 0;
                    for(int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
                    {
                        if (mesh->mTextureCoords[tn])
                        {
                            if (mesh->mNumUVComponents[tn] == 3)
                            {
                                decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TEXTURE_COORDINATES, tn);
                                offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
                            } else
                            {
                                decl->addElement(1, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, tn);
                                offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
                            }
                        }
                    }
                    if (mesh->HasTangentsAndBitangents())
                    {
                        decl->addElement(1, offset, Ogre::VET_FLOAT3, Ogre::VES_TANGENT);
                        offset += Ogre::VertexElement::getTypeSize(Ogre::VET_FLOAT3);
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
                    for(unsigned int n=0 ; n<data->vertexCount ; ++n)
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
                    
                    if (mesh->HasTextureCoords(0))
                    {
                    vbuf = Ogre::HardwareBufferManager::getSingleton().createVertexBuffer(
                        decl->getVertexSize(1),                     // This value is the size of a vertex in memory
                        data->vertexCount,                          // The number of vertices you'll put into this buffer
                        Ogre::HardwareBuffer::HBU_DYNAMIC // Properties
                        );
                    vbData = static_cast<Ogre::Real*>(vbuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                    
                    offset = 0;
                    for(unsigned int n=0 ; n<data->vertexCount ; ++n)
                    {
                        for(int tn=0 ; tn<AI_MAX_NUMBER_OF_TEXTURECOORDS ; ++tn)
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
                    }

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
                        for(int n=0 ; n<mesh->mNumFaces ; ++n)
                        {
                            idxData[offset++] = mesh->mFaces[n].mIndices[0];
                            idxData[offset++] = mesh->mFaces[n].mIndices[1];
                            idxData[offset++] = mesh->mFaces[n].mIndices[2];
                        }
                    } else
                    {
                        u32 *idxData = static_cast<u32*>(ibuf->lock(Ogre::HardwareBuffer::HBL_NORMAL));
                        offset = 0;
                        for(int n=0 ; n<mesh->mNumFaces ; ++n)
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

                if (vmin.x <= vmax.x && vmin.y <= vmax.y && vmin.z <= vmax.z)
                {
                    ogreMesh->_setBounds(Ogre::AxisAlignedBox(vmin, vmax));
                    Ogre::Real maxvertex = std::max(abs(vmax.x), std::max(abs(vmin.x), std::max(abs(vmax.y), std::max(abs(vmin.y), std::max(abs(vmax.z), abs(vmin.z))))));
                    ogreMesh->_setBoundingSphereRadius(maxvertex / 2.f);
                    ogreMesh->load();
                    outMeshNames.push_back(ogreMeshName);
                }
            }
        } catch(Ogre::Exception &e)
        {
            // error
            LogError("Failed to create Ogre mesh. Reason: ");
            LogError(e.what());
        }

        // import children
        for(int i=0 ; i<node->mNumChildren ; ++i)
            ImportNode(scene, node->mChildren[i], file, nodeName, outMeshNames);
    }
    
    void OpenAssetImport::AssImpLogStream::write(const char* message)
    {
#ifdef _DEBUG
        LogDebug(message);
#else
        LogInfo(message);
#endif
    }
}
