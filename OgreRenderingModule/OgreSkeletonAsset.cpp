#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "OgreSkeletonAsset.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"
#include "AssetAPI.h"
#include "AssetCache.h"

#include <QFile>
#include <QFileInfo>

using namespace OgreRenderer;

OgreSkeletonAsset::OgreSkeletonAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_)
{
}

OgreSkeletonAsset::~OgreSkeletonAsset()
{
    Unload();
}

AssetLoadState OgreSkeletonAsset::DeserializeFromData(const u8 *data_, size_t numBytes)
{
    if (!data_)
    {
        OgreRenderingModule::LogError("Null source asset data pointer");
        return ASSET_LOAD_FAILED;
    }
    if (numBytes == 0)
    {
        OgreRenderingModule::LogError("Zero sized skeleton asset");
        return ASSET_LOAD_FAILED;
    }

    if (OGRE_THREAD_SUPPORT != 0)
    {
        // We can only do threaded loading from disk, and not any disk location but only from asset cache.
        // local:// refs will return empty string here and those will fall back to the non-threaded loading.
        // Do not change this to do DiskCache() as that directory for local:// refs will not be a known resource location for ogre.
        QString cacheDiskSource = assetAPI->GetAssetCache()->GetDiskSource(QUrl(Name()));
        if (!cacheDiskSource.isEmpty())
        {
            QFileInfo fileInfo(cacheDiskSource);
            std::string sanitatedAssetRef = fileInfo.fileName().toStdString(); 
            loadTicket_ = Ogre::ResourceBackgroundQueue::getSingleton().load(Ogre::SkeletonManager::getSingleton().getResourceType(),
                                                                             sanitatedAssetRef, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP,
                                                                             false, 0, 0, this);
            return ASSET_LOAD_PROCESSING;
        }
    }

    try
    {
        if (ogreSkeleton.isNull())
        {
            ogreSkeleton = Ogre::SkeletonManager::getSingleton().create(
                SanitateAssetIdForOgre(this->Name().toStdString()), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            if (ogreSkeleton.isNull())
            {
                OgreRenderingModule::LogError("Failed to create skeleton " + this->Name().toStdString());
                return ASSET_LOAD_FAILED; 
            }
        }

        std::vector<u8> tempData(data_, data_ + numBytes);
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&tempData[0], numBytes, false));
#include "EnableMemoryLeakCheck.h"
        Ogre::SkeletonSerializer serializer;
        serializer.importSkeleton(stream, ogreSkeleton.getPointer());
    }
    catch (Ogre::Exception &e)
    {
        OgreRenderingModule::LogError("Failed to create skeleton " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();
        return ASSET_LOAD_FAILED;
    }

    internal_name_ = SanitateAssetIdForOgre(this->Name().toStdString());
    OgreRenderingModule::LogDebug("Ogre skeleton " + this->Name().toStdString() + " created");
    return ASSET_LOAD_SUCCESFULL;
}

void OgreSkeletonAsset::operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result)
{
    if (ticket != loadTicket_)
        return;

    if (result.error)
        OgreRenderingModule::LogError("OgreSkeletonAsset: Ogre failed to do threaded loading: " + result.message);

    assetAPI->OnTransferAssetLoadCompleted(Name(), (result.error ? ASSET_LOAD_FAILED : ASSET_LOAD_SUCCESFULL));
}

bool OgreSkeletonAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    if (ogreSkeleton.isNull())
    {
        OgreRenderingModule::LogWarning("Tried to export non-existing Ogre skeleton " + Name().toStdString() + ".");
        return false;
    }
    try
    {
        Ogre::SkeletonSerializer serializer;
        QString tempFilename = assetAPI->GenerateTemporaryNonexistingAssetFilename(Name());
        // Ogre has a limitation of not supporting serialization to a file in memory, so serialize directly to a temporary file,
        // load it up and delete the temporary file.
        serializer.exportSkeleton(ogreSkeleton.get(), tempFilename.toStdString());
        bool success = LoadFileToVector(tempFilename.toStdString().c_str(), data);
        QFile::remove(tempFilename); // Delete the temporary file we used for serialization.
        if (!success)
            return false;
    } catch (std::exception &e)
    {
        OgreRenderingModule::LogError("Failed to export Ogre skeleton " + Name().toStdString() + ":");
        if (e.what())
            OgreRenderingModule::LogError(e.what());
        return false;
    }
    return true;
}

void OgreSkeletonAsset::DoUnload()
{
    if (ogreSkeleton.isNull())
        return;

    std::string skeleton_name = ogreSkeleton->getName();
    ogreSkeleton.setNull();

    try
    {
        Ogre::SkeletonManager::getSingleton().remove(skeleton_name);
    }
    catch (...) {}
}

bool OgreSkeletonAsset::IsLoaded() const
{
    return ogreSkeleton.get() != 0;
}
