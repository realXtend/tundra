#include "StableHeaders.h"
#include "OgreSkeletonAsset.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"

using namespace OgreRenderer;

bool OgreSkeletonAsset::DeserializeFromData(const u8 *data_, size_t numBytes)
{
    if (!data_)
    {
        OgreRenderingModule::LogError("Null source asset data pointer");
        return false;
    }
    if (numBytes == 0)
    {
        OgreRenderingModule::LogError("Zero sized skeleton asset");
        return false;
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
                return false; 
            }
        }

        std::vector<u8> tempData(data_, data_ + numBytes);
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&tempData[0], numBytes, false));
        Ogre::SkeletonSerializer serializer;
        serializer.importSkeleton(stream, ogreSkeleton.getPointer());
    }
    catch (Ogre::Exception &e)
    {
        OgreRenderingModule::LogError("Failed to create skeleton " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();
        return false;
    }

    internal_name_ = SanitateAssetIdForOgre(this->Name().toStdString());
    OgreRenderingModule::LogDebug("Ogre skeleton " + this->Name().toStdString() + " created");
    return true;
}

void OgreSkeletonAsset::Unload()
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
