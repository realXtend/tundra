// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OgreMeshAsset.h"
#include "OgreConversionUtils.h"
#include "OgreRenderingModule.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "Profiler.h"

#include <QFile>
#include <QFileInfo>
#include <Ogre.h>

#include "LoggingFunctions.h"
#include "MemoryLeakCheck.h"

OgreMeshAsset::~OgreMeshAsset()
{
    Unload();
}

bool OgreMeshAsset::DeserializeFromData(const u8 *data_, size_t numBytes, const bool allowAsynchronous)
{
    PROFILE(OgreMeshAsset_LoadFromFileInMemory);
    assert(data_);
    if (!data_)
        return false;
    
    /// Force an unload of this data first.
    Unload();

    // Asynchronous loading
    // 1. AssetAPI allows a asynch load. This is false when called from LoadFromFile(), LoadFromCache() etc.
    // 2. We have a rendering window for Ogre as Ogre::ResourceBackgroundQueue does not work otherwise. Its not properly initialized without a rendering window.
    // 3. The Ogre we are building against has thread support.
    if (allowAsynchronous && !assetAPI->IsHeadless() && (OGRE_THREAD_SUPPORT != 0))
    {
        // We can only do threaded loading from disk, and not any disk location but only from asset cache.
        // local:// refs will return empty string here and those will fall back to the non-threaded loading.
        // Do not change this to do DiskCache() as that directory for local:// refs will not be a known resource location for ogre.
        QString cacheDiskSource = assetAPI->GetAssetCache()->GetDiskSourceByRef(Name());
        if (!cacheDiskSource.isEmpty())
        {
            QFileInfo fileInfo(cacheDiskSource);
            std::string sanitatedAssetRef = fileInfo.fileName().toStdString();
            loadTicket_ = Ogre::ResourceBackgroundQueue::getSingleton().load(Ogre::MeshManager::getSingleton().getResourceType(),
                              sanitatedAssetRef, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP, false, 0, 0, this);
            return true;
        }
    }

    // Synchronous loading
    if (ogreMesh.isNull())
    {   
        ogreMesh = Ogre::MeshManager::getSingleton().createManual(
            OgreRenderer::SanitateAssetIdForOgre(this->Name().toStdString()), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
        if (ogreMesh.isNull())
        {
            LogError("Failed to create mesh " + Name().toStdString());
            return false; 
        }
        ogreMesh->setAutoBuildEdgeLists(false);
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
        return false;
    }
    
    // Generate tangents to mesh
    try
    {
        unsigned short src, dest;
        ///\bug Crashes if called for a mesh that has null or zero vertices in the vertex buffer, or null or zero indices in the index buffer.
        if (!ogreMesh->suggestTangentVectorBuildParams(Ogre::VES_TANGENT, src, dest))
            ogreMesh->buildTangentVectors(Ogre::VES_TANGENT, src, dest);
    }
    catch(...) {}
    
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
    catch(...) {}
        
    try
    {
        // Assign default materials that won't complain
        SetDefaultMaterial();
        // Set asset references the mesh has
        //ResetReferences();
    }
    catch(Ogre::Exception &e)
    {
        ::LogError("Failed to create mesh " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();
        return false;
    }

    //internal_name_ = SanitateAssetIdForOgre(id_);
    //LogDebug("Ogre mesh " + this->Name().toStdString() + " created");

    // We did a synchronous load, must call AssetLoadCompleted here.
    assetAPI->AssetLoadCompleted(Name());
    return true;
}

void OgreMeshAsset::operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result)
{
    if (ticket != loadTicket_)
        return;

    const QString assetRef = Name();
    if (!result.error)
    {
        /*! \todo Verify if we need to do
            - ogreMesh->setAutoBuildEdgeLists(false);
            - ogreMesh->buildTangentVectors(...);
            - smesh->generateExtremes(n);
            for non-manual created meshes via thread loading.
        */

        ogreMesh = Ogre::MeshManager::getSingleton().getByName(OgreRenderer::SanitateAssetIdForOgre(assetRef), 
                                                               OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP);
        if (!ogreMesh.isNull())
        {        
            try
            {
                SetDefaultMaterial();
                assetAPI->AssetLoadCompleted(assetRef);
                return;
            }
            catch (Ogre::Exception &e)
            {
                LogError("OgreMeshAsset asynch load: Failed to set default materials to " + assetRef.toStdString() + ": " + std::string(e.what()));
            }
        }
        else
            LogError("OgreMeshAsset asynch load: Ogre::Mesh was null after threaded loading: " + assetRef);
    }
    else
        LogError("OgreMeshAsset asynch load: Ogre failed to do threaded loading: " + result.message);

    DoUnload();
    assetAPI->AssetLoadFailed(assetRef);
}

void OgreMeshAsset::DoUnload()
{
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
    if (ogreMesh.isNull())
        return;

//    originalMaterials.clear();
    for(unsigned short i = 0; i < ogreMesh->getNumSubMeshes(); ++i)
    {
        Ogre::SubMesh *submesh = ogreMesh->getSubMesh(i);
        if (submesh)
        {
//            originalMaterials.push_back(submesh->getMaterialName().c_str());
            submesh->setMaterialName("LitTextured");
        }
    }
}

bool OgreMeshAsset::IsLoaded() const
{
    return ogreMesh.get() != 0;
}

bool OgreMeshAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    if (ogreMesh.isNull())
    {
        ::LogWarning("Tried to export non-existing Ogre mesh " + Name().toStdString() + ".");
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
        if (!success)
            return false;
    } catch(std::exception &e)
    {
        ::LogError("Failed to export Ogre mesh " + Name().toStdString() + ":");
        if (e.what())
            ::LogError(e.what());
        return false;
    }
    return true;
}
