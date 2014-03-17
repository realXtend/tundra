// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreSkeletonAsset.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"

#include "Framework.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "LoggingFunctions.h"

#include <QFile>
#include <QFileInfo>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

OgreSkeletonAsset::OgreSkeletonAsset(AssetAPI *owner, const QString &type_, const QString &name_) :
    IAsset(owner, type_, name_), loadTicket_(0)
{
}

OgreSkeletonAsset::~OgreSkeletonAsset()
{
    Unload();
}

bool OgreSkeletonAsset::AllowAsynchronousLoading() const
{
    OgreRenderer::OgreRenderingModule *renderingModule = assetAPI->GetFramework()->Module<OgreRenderer::OgreRenderingModule>();
    OgreRenderer::Renderer *renderer = (renderingModule ? renderingModule->Renderer().get() : 0);
    bool allow = (renderer ? renderer->AllowAsynchronousLoading() : false);

    // Asset cache must have this asset.
    if (allow && assetAPI->Cache()->FindInCache(Name()).isEmpty())
        allow = false;
    return allow;
}

bool OgreSkeletonAsset::DeserializeFromData(const u8 *data_, size_t numBytes, bool allowAsynchronous)
{
    if (!data_)
    {
        LogError("OgreSkeletonAsset::DeserializeFromData: Null source asset data pointer");
        return false;
    }
    if (numBytes == 0)
    {
        LogError("OgreSkeletonAsset::DeserializeFromData: Zero sized skeleton asset");
        return false;
    }
    
    /// Force an unload of this data first.
    Unload();

    if (allowAsynchronous && !AllowAsynchronousLoading())
        allowAsynchronous = false;

    // Async loading.
    if (allowAsynchronous)
    {
        QString cacheDiskSource = assetAPI->Cache()->FindInCache(Name());
        if (!cacheDiskSource.isEmpty())
        {
            QFileInfo fileInfo(cacheDiskSource);
            std::string sanitatedAssetRef = fileInfo.fileName().toStdString(); 
            loadTicket_ = Ogre::ResourceBackgroundQueue::getSingleton().load(Ogre::SkeletonManager::getSingleton().getResourceType(),
                              sanitatedAssetRef, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP, false, 0, 0, this);
        }
        return true;
    }

    // Synchronous loading
    try
    {
        if (ogreSkeleton.isNull())
        {
            ogreSkeleton = Ogre::SkeletonManager::getSingleton().create(
                AssetAPI::SanitateAssetRef(this->Name().toStdString()), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

            if (ogreSkeleton.isNull())
            {
                LogError("OgreSkeletonAsset::DeserializeFromData: Failed to create skeleton " + this->Name());
                return false; 
            }
        }

        std::vector<u8> tempData(data_, data_ + numBytes);
#include "DisableMemoryLeakCheck.h"
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&tempData[0], numBytes, false));
#include "EnableMemoryLeakCheck.h"
        Ogre::SkeletonSerializer serializer;
        serializer.importSkeleton(stream, ogreSkeleton.getPointer());
    }
    catch(Ogre::Exception &e)
    {
        LogError("OgreSkeletonAsset::DeserializeFromData: Failed to create skeleton " + this->Name() + ": " + QString(e.what()));
        Unload();
        return false;
    }

    internal_name_ = AssetAPI::SanitateAssetRef(this->Name().toStdString());
    //LogDebug("Ogre skeleton " + this->Name().toStdString() + " created");

    // We did a synchronous load, must call AssetLoadCompleted here.
    assetAPI->AssetLoadCompleted(Name());
    return true;
}

void OgreSkeletonAsset::operationCompleted(Ogre::BackgroundProcessTicket ticket, const Ogre::BackgroundProcessResult &result)
{
    if (ticket != loadTicket_)
        return;

    // Reset to 0 to mark the asynch request is not active anymore. Aborted in Unload() if it is.
    loadTicket_ = 0;
    
    const QString assetRef = Name();
    internal_name_ = AssetAPI::SanitateAssetRef(assetRef).toStdString();
    if (!result.error)
    {
        ogreSkeleton = Ogre::SkeletonManager::getSingleton().getByName(internal_name_, OgreRenderer::OgreRenderingModule::CACHE_RESOURCE_GROUP);
        if (!ogreSkeleton.isNull())
        {
            assetAPI->AssetLoadCompleted(assetRef);
            return;
        }
        else
            LogError("OgreSkeletonAsset asynch load: Could not get Skeleton from SkeletonManager after threaded loading: " + assetRef);
    }
    else
        LogError("OgreSkeletonAsset asynch load: Ogre failed to do threaded loading: " + result.message);

    DoUnload();
    assetAPI->AssetLoadFailed(assetRef);
}

bool OgreSkeletonAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters) const
{
    if (ogreSkeleton.isNull())
    {
        LogWarning("Tried to export non-existing Ogre skeleton " + Name() + ".");
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
    } catch(std::exception &e)
    {
        LogError("Failed to export Ogre skeleton " + Name() + ":");
        if (e.what())
            LogError(e.what());
        return false;
    }
    return true;
}

void OgreSkeletonAsset::DoUnload()
{
    // If a ongoing asynchronous asset load requested has been made to ogre, we need to abort it.
    // Otherwise Ogre will crash to our raw pointer that was passed if we get deleted. A ongoing ticket id cannot be 0.
    if (loadTicket_ != 0)
    {
        Ogre::ResourceBackgroundQueue::getSingleton().abortRequest(loadTicket_);
        loadTicket_ = 0;
    }
        
    if (ogreSkeleton.isNull())
        return;

    std::string skeleton_name = ogreSkeleton->getName();
    ogreSkeleton.setNull();

    try
    {
        Ogre::SkeletonManager::getSingleton().remove(skeleton_name);
    }
    catch(...) {}
}

bool OgreSkeletonAsset::IsLoaded() const
{
    return ogreSkeleton.get() != 0;
}
