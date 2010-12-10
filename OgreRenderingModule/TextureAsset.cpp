#include "StableHeaders.h"

#include "TextureAsset.h"
#include "OgreConversionUtils.h"

#include "OgreRenderingModule.h"
#include <Ogre.h>

TextureAsset::~TextureAsset()
{
    Unload();
}

bool TextureAsset::DeserializeFromData(const u8 *data, size_t numBytes)
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
        OgreRenderer::OgreRenderingModule::LogError("TextureAsset::DeserializeFromData: Failed to create texture " + this->Name().toStdString() + ": " + std::string(e.what()));
        Unload();

        return false;
    }
}

bool TextureAsset::SerializeTo(std::vector<u8> &data, const QString &serializationParameters)
{
    if (ogreTexture.isNull())
    {
        OgreRenderer::OgreRenderingModule::LogWarning("TextureAsset::SerializeTo called on an unloaded texture \"" + Name().toStdString() + "\".");
        return false;
    }

    try
    {
        Ogre::Image new_image;
                
        // From Ogre 1.7 Texture::convertToImage()
        size_t numMips = 1;
        size_t dataSize = Ogre::Image::calculateSize(numMips,
            ogreTexture->getNumFaces(), ogreTexture->getWidth(), ogreTexture->getHeight(), ogreTexture->getDepth(), ogreTexture->getFormat());
        void* pixData = OGRE_MALLOC(dataSize, Ogre::MEMCATEGORY_GENERAL);
        // if there are multiple faces and mipmaps we must pack them into the data
        // faces, then mips
        void* currentPixData = pixData;
        for (size_t face = 0; face < ogreTexture->getNumFaces(); ++face)
        {
            for (size_t mip = 0; mip < numMips; ++mip)
            {
                size_t mipDataSize = Ogre::PixelUtil::getMemorySize(ogreTexture->getWidth(), ogreTexture->getHeight(), ogreTexture->getDepth(), ogreTexture->getFormat());
                Ogre::PixelBox pixBox(ogreTexture->getWidth(), ogreTexture->getHeight(), ogreTexture->getDepth(), ogreTexture->getFormat(), currentPixData);
                ogreTexture->getBuffer(face, mip)->blitToMemory(pixBox);
                currentPixData = (void*)((char*)currentPixData + mipDataSize);
            }
        }
        // load, and tell Image to delete the memory when it's done.
        new_image.loadDynamicImage((Ogre::uchar*)pixData, ogreTexture->getWidth(), ogreTexture->getHeight(), ogreTexture->getDepth(), ogreTexture->getFormat(), true, 
            ogreTexture->getNumFaces(), numMips - 1);

        Ogre::DataStreamPtr imageStream = new_image.encode(serializationParameters.toStdString());
        if (imageStream.get() && imageStream->size() > 0)
        {
            data.resize(imageStream->size());
            imageStream->read(&data[0], data.size());
        }
    } catch (std::exception &e)
    {
        OgreRenderer::OgreRenderingModule::LogError("Failed to export Ogre texture " + Name().toStdString() + ":");
        if (e.what())
            OgreRenderer::OgreRenderingModule::LogError(e.what());
        return false;
    }
    return true;
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
