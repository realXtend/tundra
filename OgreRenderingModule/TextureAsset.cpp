#include "StableHeaders.h"

#include "TextureAsset.h"
#include "OgreConversionUtils.h"

#include "OgreRenderingModule.h"
#include <Ogre.h>

bool TextureAsset::LoadFromFileInMemory(const u8 *data, size_t numBytes)
{
    if (!data)
        return false; ///\todo Log out error.
    if (numBytes == 0)
        return false; ///\todo Log out error.

    // If we have any previous texture loaded, unload it first.
    Unload();

    try
    {
        std::vector<u8> tempData(data, data + numBytes);
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&tempData[0], tempData.size(), false));
        Ogre::Image image;
        image.load(stream);
        ogreAssetName = OgreRenderer::SanitateAssetIdForOgre(this->Name().toStdString()).c_str();
        ogreTexture = Ogre::TextureManager::getSingleton().loadImage(ogreAssetName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image);

        return true;
    }
    catch (Ogre::Exception &e)
    {
        OgreRenderer::OgreRenderingModule::LogError("TextureAsset::LoadFromFileInMemory: Failed to create texture " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();

        return false;
    }
}

void TextureAsset::Unload()
{
    if (!ogreTexture.isNull())
        ogreAssetName = ogreTexture->getName().c_str();

    ogreTexture = Ogre::TexturePtr();
    try
    {
        Ogre::TextureManager::getSingleton().remove(ogreAssetName.toStdString());
    }
    catch (...) {}
}
