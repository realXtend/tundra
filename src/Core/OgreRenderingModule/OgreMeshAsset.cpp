// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "Win.h"
#include "DebugOperatorNew.h"
#include "OgreMeshAsset.h"
#include "OgreRenderingModule.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "Profiler.h"
#include "Geometry/Ray.h"

#include <QFile>
#include <QFileInfo>
#include <Ogre.h>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

OgreMeshAsset::OgreMeshAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_),
    loadTicket_(0)
{
}

OgreMeshAsset::~OgreMeshAsset()
{
    Unload();
}

bool OgreMeshAsset::LoadFromFile(QString filename)
{
    /// @todo Duplicate allowAsynchronous code in OgreMeshAsset and TextureAsset.
    bool allowAsynchronous = true;
    if ((OGRE_THREAD_SUPPORT == 0) || !assetAPI->Cache() || assetAPI->IsHeadless() ||
        assetAPI->GetFramework()->HasCommandLineParameter("--noAsyncAssetLoad") ||
        assetAPI->GetFramework()->HasCommandLineParameter("--no_async_asset_load")) /**< @todo Remove support for the deprecated underscore version at some point. */
    {
        allowAsynchronous = false;
    }

    QString cacheDiskSource;
    if (allowAsynchronous)
    {
        cacheDiskSource = assetAPI->Cache()->FindInCache(Name());
        if (cacheDiskSource.isEmpty())
            allowAsynchronous = false;
    }
    
    if (allowAsynchronous)
        return DeserializeFromData(0, 0, true);
    else
        return IAsset::LoadFromFile(filename);
}

bool OgreMeshAsset::DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous)
{
    PROFILE(OgreMeshAsset_LoadFromFileInMemory);

    /// Force an unload of this data first.
    Unload();

    /// @todo Duplicate allowAsynchronous code in OgreMeshAsset and TextureAsset.
    if ((OGRE_THREAD_SUPPORT == 0) || !assetAPI->Cache() || assetAPI->IsHeadless() || IsAssimpFileType() ||
        assetAPI->GetFramework()->HasCommandLineParameter("--noAsyncAssetLoad") ||
        assetAPI->GetFramework()->HasCommandLineParameter("--no_async_asset_load")) /**< @todo Remove support for the deprecated underscore version at some point. */
    {
        allowAsynchronous = false;
    }

    QString cacheDiskSource;
    if (allowAsynchronous)
    {
        cacheDiskSource = assetAPI->Cache()->FindInCache(Name());
        if (cacheDiskSource.isEmpty())
            allowAsynchronous = false;
    }
    
    // Asynchronous loading
    // 1. AssetAPI allows a asynch load. This is false when called from LoadFromFile(), LoadFromCache() etc.
    // 2. We have a rendering window for Ogre as Ogre::ResourceBackgroundQueue does not work otherwise. Its not properly initialized without a rendering window.
    // 3. The Ogre we are building against has thread support.
    if (allowAsynchronous)
    {
        // We can only do threaded loading from disk, and not any disk location but only from asset cache.
        // local:// refs will return empty string here and those will fall back to the non-threaded loading.
        // Do not change this to do DiskCache() as that directory for local:// refs will not be a known resource location for ogre.
        QFileInfo fileInfo(cacheDiskSource);
        std::string sanitatedAssetRef = fileInfo.fileName().toStdString();
        //! \todo - Should we set this somewhere for async path?: ogreMesh->setAutoBuildEdgeLists(false);
        loadTicket_ = Ogre::ResourceBackgroundQueue::getSingleton().load(Ogre::MeshManager::getSingleton().getResourceType(),
                          sanitatedAssetRef, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP, false, 0, 0, this);
        return true;
    }

    if (!data_)
    {
        LogError("OgreMeshAsset::DeserializeFromData: Cannot deserialize from null input data");
        return false;
    }
    
    // Synchronous loading
    if (ogreMesh.isNull())
    {   
        ogreMesh = Ogre::MeshManager::getSingleton().createManual(
            AssetAPI::SanitateAssetRef(this->Name()).toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (ogreMesh.isNull())
        {
            LogError("OgreMeshAsset::DeserializeFromData: Failed to create mesh " + Name());
            return false; 
        }
        ogreMesh->setAutoBuildEdgeLists(false);
    }

    // Convert file to Ogre mesh using assimp
    if (IsAssimpFileType())
    {
#ifdef ASSIMP_ENABLED
        emit ExternalConversionRequested(this, data_, numBytes);
        return true;
#else
        LogError(QString("OgreMeshAsset::DeserializeFromData: cannot convert " + Name() + " to Ogre mesh. OpenAssetImport is not enabled."));
        return false;
#endif
    }

    try
    {
        std::vector<u8> tempData(data_, data_ + numBytes);
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)&tempData[0], numBytes, false));
#include "EnableMemoryLeakCheck.h"
        Ogre::MeshSerializer serializer;
        serializer.importMesh(stream, ogreMesh.getPointer()); // Note: importMesh *adds* submeshes to an existing mesh. It doesn't replace old ones.
    }
    catch (Ogre::Exception &e)
    {
        LogError(QString("OgreMeshAsset::DeserializeFromData: Ogre::MeshSerializer::importMesh failed when loading asset '" + Name() + "': ") + e.what());

        if(IsAssimpFileType())
            LogError(QString("OgreMeshAsset::DeserializeFromData: cannot convert " + Name() + " to Ogre mesh. OpenAssetImport is not enabled."));

        return false;
    }

    if (GenerateMeshData())
    {        
        // We did a synchronous load, must call AssetLoadCompleted here.
        assetAPI->AssetLoadCompleted(Name());
        return true;
    }
    else 
        return false;
}

struct KdTreeRayQueryFirstHitVisitor
{
    RayQueryResult result;

    KdTreeRayQueryFirstHitVisitor()
    {
        result.entity = 0;
        result.component = 0;
        result.t = std::numeric_limits<float>::infinity();
        result.triangleIndex = KdTree<Triangle>::BUCKET_SENTINEL;
        result.submeshIndex = (u32)-1;
    }
    bool operator()(KdTree<Triangle> &tree, const KdTreeNode &leaf, const Ray &ray, float tNear, float tFar)
    {
        u32 *bucket = tree.Bucket(leaf.bucketIndex);
        assert(bucket);
        while(*bucket != KdTree<Triangle>::BUCKET_SENTINEL)
        {
            const Triangle &tri = tree.Object(*bucket);
            float u, v, t;
            t = Triangle::IntersectLineTri(ray.pos, ray.dir, tri.a, tri.b, tri.c, u, v);
            bool intersects = t < std::numeric_limits<float>::infinity();
            if (intersects && t >= tNear && t <= tFar && t < result.t)
            {
                result.t = t;
                result.pos = ray.GetPoint(t);
                result.triangleIndex = *bucket;
                result.submeshIndex = (u32)-1;
                result.barycentricUV = float2(u,v);
            }
            ++bucket;
            
        }
        return result.t < std::numeric_limits<float>::infinity(); // If we hit a triangle, no need to visit any farther nodes, since we are only interested in the nearest hit.
    }
};

RayQueryResult OgreMeshAsset::Raycast(const Ray &ray)
{
    if (!ogreMesh.get())
        return RayQueryResult();
    if (meshData.NumObjects() == 0)
        CreateKdTree();
    KdTreeRayQueryFirstHitVisitor visitor;
    meshData.RayQuery(ray, visitor);
    if (visitor.result.triangleIndex != KdTree<Triangle>::BUCKET_SENTINEL)
    {
        visitor.result.normal = normals[visitor.result.triangleIndex];
        float2 uv = (uvs.size() > visitor.result.triangleIndex*3+2) ?
                       (1.f - visitor.result.barycentricUV.x - visitor.result.barycentricUV.y) * uvs[visitor.result.triangleIndex*3]
                       + visitor.result.barycentricUV.x * uvs[visitor.result.triangleIndex*3+1]
                       + visitor.result.barycentricUV.y * uvs[visitor.result.triangleIndex*3+2]
                    : float2(-1, -1);
        visitor.result.uv = uv;
        int triangleIndex = visitor.result.triangleIndex;
        for(size_t i = 0; i < subMeshTriangleCounts.size(); ++i)
        {
            if (triangleIndex < subMeshTriangleCounts[i])
            {
                visitor.result.submeshIndex = (unsigned)i;
                break;
            }
            else
                triangleIndex -= subMeshTriangleCounts[i];
        }
    }
    return visitor.result;
}

Triangle OgreMeshAsset::Tri(int submeshIndex, int triangleIndex)
{
    if (subMeshTriangleCounts.size() == 0)
        CreateKdTree();

    if (triangleIndex < 0 || NumTris(submeshIndex) < triangleIndex)
    {
        LogError("Invalid triangle index to call to OgreMeshAsset::Tri(submeshIndex=" + QString::number(submeshIndex) + ", triangleIndex=" + QString::number(triangleIndex) + "), the specified submesh has only " + NumTris(submeshIndex) + " triangles!");
        return Triangle();
    }
    // Shift to index in proper location of the submesh triangles array.
    for(int i = 0; i < submeshIndex; ++i)
        triangleIndex += subMeshTriangleCounts[i];
    return meshData.Object(triangleIndex);
}

size_t OgreMeshAsset::NumSubmeshes()
{
    if (subMeshTriangleCounts.size() == 0)
        CreateKdTree();

    return subMeshTriangleCounts.size();
}

int OgreMeshAsset::NumTris(int submeshIndex)
{
    if (subMeshTriangleCounts.size() == 0)
        CreateKdTree();

    if (submeshIndex >= 0 && (size_t)submeshIndex < subMeshTriangleCounts.size())
        return subMeshTriangleCounts[submeshIndex];

    LogError("Ogre mesh " + Name() + " does not contain " + QString::number(submeshIndex+1) + " submeshes! (has only " + QString::number(subMeshTriangleCounts.size()) + ")");
    return 0;
}

void OgreMeshAsset::CreateKdTree()
{
    meshData.Clear();
    normals.clear();
    uvs.clear();
    subMeshTriangleCounts.clear();
    for(unsigned short i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh *submesh = ogreMesh->getSubMesh(i);
        assert(submesh);
        
        Ogre::VertexData *vertexData = submesh->useSharedVertices ? ogreMesh->sharedVertexData : submesh->vertexData;
        assert(vertexData);
        
        const Ogre::VertexElement *posElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
        if (!posElem)
        {
            subMeshTriangleCounts.push_back(0);
            continue; // No position element. Ignore this submesh.
        }
        
        Ogre::HardwareVertexBufferSharedPtr vbufPos = vertexData->vertexBufferBinding->getBuffer(posElem->getSource());

        unsigned char *pos = (unsigned char*)vbufPos->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
        assert(pos);
        size_t posOffset = posElem->getOffset();
        size_t posSize = vbufPos->getVertexSize();
        
        // Texcoord element is not mandatory
        unsigned char *texCoord = 0;
        size_t texOffset = 0;
        size_t texSize = 0;
        Ogre::HardwareVertexBufferSharedPtr vbufTex;
        const Ogre::VertexElement *texElem = vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);
        if (texElem)
        {
            vbufTex = vertexData->vertexBufferBinding->getBuffer(texElem->getSource());
            // Check if the texcoord buffer is different than the position buffer, in that case lock it separately
            if (vbufTex != vbufPos)
                texCoord = static_cast<unsigned char*>(vbufTex->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
            else
                texCoord = pos;
            texOffset = texElem->getOffset();
            texSize = vbufTex->getVertexSize();
        }
        
        Ogre::IndexData *indexData = submesh->indexData;
        Ogre::HardwareIndexBufferSharedPtr ibuf = indexData->indexBuffer;

        u32 *pLong = (u32*)ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY);
        u16 *pShort = (u16*)pLong;
        const bool use32BitIndices = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);
        
        for(unsigned j = 0; j+2 < indexData->indexCount; j += 3)
        {
            unsigned i0, i1, i2;
            if (use32BitIndices)
            {
                i0 = pLong[j];
                i1 = pLong[j+1];
                i2 = pLong[j+2];
            }
            else
            {
                i0 = pShort[j];
                i1 = pShort[j+1];
                i2 = pShort[j+2];
            }
            
            float3 v0 = *(float3*)(pos + posOffset + i0 * posSize);
            float3 v1 = *(float3*)(pos + posOffset + i1 * posSize);
            float3 v2 = *(float3*)(pos + posOffset + i2 * posSize);
            Triangle t(v0, v1, v2);
            meshData.AddObjects(&t, 1);

            if (texElem)
            {
                uvs.push_back(*((float2*)(texCoord + texOffset + i0 * texSize)));
                uvs.push_back(*((float2*)(texCoord + texOffset + i1 * texSize)));
                uvs.push_back(*((float2*)(texCoord + texOffset + i2 * texSize)));
            }

            float3 edge1 = v1 - v0;
            float3 edge2 = v2 - v0;
            float3 normal = edge1.Cross(edge2);
            normal.Normalize();
            normals.push_back(normal);
        }
        subMeshTriangleCounts.push_back((int)(indexData->indexCount / 3));
        
        vbufPos->unlock();
        if (!vbufTex.isNull() && vbufTex != vbufPos)
            vbufTex->unlock();
        ibuf->unlock();
    }

    {
        PROFILE(OgreMeshAsset_KdTree_Build);
        meshData.Build();
    }
}

bool OgreMeshAsset::GenerateMeshData()
{
    if (ogreMesh.isNull())
        return false;
    /* NOTE: only the last error handler here returns false - first are ignored.
       This is to keep the behaviour identical to the original version which had these checks inside 
       DeserializeFromData - see https://github.com/realXtend/naali/blob/1806ea04057d447263dbd7cf66d5731c36f4d4a3/src/Core/OgreRenderingModule/OgreMeshAsset.cpp#L89
    */
    
    // Generate tangents to mesh
    try
    {
        unsigned short src, dest;
        ///\bug Crashes if called for a mesh that has null or zero vertices in the vertex buffer, or null or zero indices in the index buffer.
        if (!ogreMesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
            ogreMesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
    }
    catch(const Ogre::Exception &e)
    {
        QString what(e.what());
        // "Cannot locate an appropriate 2D texture coordinate set" is benign, see OgreLogListener::messageLogged
        const bool hideBenignOgreMessages = ( assetAPI->GetFramework()->HasCommandLineParameter("--hideBenignOgreMessages") ||
            assetAPI->GetFramework()->HasCommandLineParameter("--hide_benign_ogre_messages")); /**< @todo Remove support for the deprecated underscore version at some point. */
        if (!hideBenignOgreMessages || (hideBenignOgreMessages && !what.contains("Cannot locate an appropriate 2D texture coordinate set")))
            LogError("OgreMeshAsset::GenerateMeshData: Failed to build tangents for mesh " + this->Name() + ": " + what);
    }

    // Generate extremity points to submeshes, 1 should be enough
    try
    {
        for(unsigned short i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
        {
            Ogre::SubMesh *smesh = ogreMesh->getSubMesh(i);
            if (smesh)
                smesh->generateExtremes(1);
        }
    }
    catch(const Ogre::Exception &e)
    {
        LogError("OgreMeshAsset::GenerateMeshData: Failed to generate extremity points to submeshes for mesh " + this->Name() + ": " + QString(e.what()));
    }

    try
    {
        // Assign default materials that won't complain
        if (!IsAssimpFileType())
            SetDefaultMaterial();
        // Set asset references the mesh has
        //ResetReferences();
    }
    catch(const Ogre::Exception &e)
    {
        LogError("OgreMeshAsset::GenerateMeshData: Failed to set default materials to " + this->Name() + ": " + QString(e.what()));
        Unload();
        return false;
    }

    //internal_name_ = AssetAPI::SanitateAssetRef(id_);
    //LogDebug("Ogre mesh " + this->Name().toStdString() + " created");

    return true;
}

void OgreMeshAsset::operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result)
{
    if (ticket != loadTicket_)
        return;
    
    // Reset to 0 to mark the asynch request is not active anymore. Aborted in Unload() if it is.
    loadTicket_ = 0;
    
    const QString assetRef = Name();
    if (!result.error)
    {
        ogreMesh = Ogre::MeshManager::getSingleton().getByName(AssetAPI::SanitateAssetRef(assetRef).toStdString(), 
                                                               OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP);
        if (!ogreMesh.isNull())
        {
            if (GenerateMeshData())
            {
                assetAPI->AssetLoadCompleted(assetRef);
                return;
            }
            else
            {
                LogError("OgreMeshAsset::operationCompleted: asynch load failed in GenerateMeshData - see log above for details.");
            }
        }
        else
            LogError("OgreMeshAsset::operationCompleted: Ogre::Mesh was null after threaded loading: " + assetRef);
    }
    else
        LogError("OgreMeshAsset::operationCompleted: Ogre failed to do threaded loading: " + result.message);

    DoUnload();
    assetAPI->AssetLoadFailed(assetRef);
}

void OgreMeshAsset::DoUnload()
{
    // If a ongoing asynchronous asset load requested has been made to ogre, we need to abort it.
    // Otherwise Ogre will crash to our raw pointer that was passed if we get deleted. A ongoing ticket id cannot be 0.
    if (loadTicket_ != 0)
    {
        Ogre::ResourceBackgroundQueue::getSingleton().abortRequest(loadTicket_);
        loadTicket_ = 0;
    }
    
    if (ogreMesh.isNull())
        return;

    std::string meshName = ogreMesh->getName();
    ogreMesh.setNull();
    try
    {
        Ogre::MeshManager::getSingleton().remove(meshName);
    }
    catch(...) {}
}

void OgreMeshAsset::SetDefaultMaterial()
{
    if (!ogreMesh.get())
        return;

    for(unsigned short i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh *submesh = ogreMesh->getSubMesh(i);
        if (submesh)
            submesh->setMaterialName("LitTextured");
    }
}

bool OgreMeshAsset::IsLoaded() const
{
    return (ogreMesh.get() != 0);
}

QString OgreMeshAsset::OgreMeshName() const
{
    return (ogreMesh.get() != 0 ? QString::fromStdString(ogreMesh->getName()) : "");
}

bool OgreMeshAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    UNREFERENCED_PARAM(serializationParameters);

    if (ogreMesh.isNull())
    {
        LogWarning("OgreMeshAsset::SerializeTo: Tried to export non-existing Ogre mesh " + Name() + ".");
        return false;
    }
    try
    {
        Ogre::MeshSerializer serializer;
        QString tempFilename = assetAPI->GenerateTemporaryNonexistingAssetFilename(Name());
        // Ogre has a limitation of not supporting serialization to a file in memory, so serialize directly to a temporary file,
        // load it up and delete the temporary file.
        serializer.exportMesh(ogreMesh.get(), tempFilename.toStdString());
        bool success = LoadFileToVector(tempFilename.toStdString().c_str(), data);
        QFile::remove(tempFilename); // Delete the temporary file we used for serialization.
        return success;
    }
    catch(const std::exception &e)
    {
        LogError("OgreMeshAsset::SerializeTo: Failed to export Ogre mesh " + Name() + ": " + (e.what() ? QString(e.what()) : ""));
    }
    return false;
}

bool OgreMeshAsset::IsAssimpFileType() const
{
    const char * const openAssImpFileTypes[] = { ".3d", ".b3d", ".blend", ".dae", ".bvh", ".3ds", ".ase", ".obj", ".ply", ".dxf",
        ".nff", ".smd", ".vta", ".mdl", ".md2", ".md3", ".mdc", ".md5mesh", ".x", ".q3o", ".q3s", ".raw", ".ac",
        ".stl", ".irrmesh", ".irr", ".off", ".ter", ".mdl", ".hmp", ".ms3d", ".lwo", ".lws", ".lxo", ".csm",
        ".ply", ".cob", ".scn" };

    for(size_t i = 0; i < NUMELEMS(openAssImpFileTypes); ++i)
        if (this->Name().endsWith(openAssImpFileTypes[i], Qt::CaseInsensitive))
            return true;

    return false;
}

#ifdef ASSIMP_ENABLED
void OgreMeshAsset::OnAssimpConversionDone(bool success)
{
    if (!IsAssimpFileType())
        return;

    if(success)
    {
        if (GenerateMeshData())
            assetAPI->AssetLoadCompleted(Name());
    }
    else
    {
        assetAPI->AssetLoadFailed(Name());
        LogError("OgreMeshAsset::DeserializeFromData: Failed to to covert " + Name() + " to Ogre mesh.");
    }
}
#endif
