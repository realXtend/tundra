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

    try
    {
        // Convert the data into Ogre's own DataStream format.
        std::vector<u8> tempData(data, data + numBytes);
        Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream(&tempData[0], tempData.size(), false));
        // Load up the image as an Ogre CPU image object.
        Ogre::Image image;
        image.load(stream);

        if (ogreTexture.isNull()) // If we are creating this texture for the first time, create a new Ogre::Texture object.
        {
            ogreAssetName = OgreRenderer::SanitateAssetIdForOgre(this->Name().toStdString()).c_str();
            ogreTexture = Ogre::TextureManager::getSingleton().loadImage(ogreAssetName.toStdString(), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image);
        }
        else // If we're loading on top of an Ogre::Texture we've created before, don't lose the old Ogre::Texture object, but reuse the old.
        {    // This will allow all existing materials to keep referring to this texture, and they'll get the updated texture image immediately.
            ogreTexture->freeInternalResources(); 

            if (image.getWidth() != ogreTexture->getWidth() || image.getHeight() != ogreTexture->getHeight() || image.getFormat() != ogreTexture->getFormat())
            {
                ogreTexture->setWidth(image.getWidth());
                ogreTexture->setHeight(image.getHeight());
                ogreTexture->setFormat(image.getFormat());
            }

            if (ogreTexture->getBuffer().isNull())
            {
                OgreRenderer::OgreRenderingModule::LogError("TextureAsset::DeserializeFromData: Failed to create texture " + this->Name().toStdString() + ": OgreTexture::getBuffer() was null!");
                return false;
            }

            Ogre::PixelBox pixelBox(Ogre::Box(0,0, image.getWidth(), image.getHeight()), image.getFormat(), (void*)image.getData());
            ogreTexture->getBuffer()->blitFromMemory(pixelBox);

            ogreTexture->createInternalResources();
        }

        return true;
    }
    catch (Ogre::Exception &e)
    {
        OgreRenderer::OgreRenderingModule::LogError("TextureAsset::DeserializeFromData: Failed to create texture " + this->Name().toStdString() + ": " + std::string(e.what()));

        return false;
    }
}
/*
void TextureAsset::RegenerateAllMipLevels()
{
    if (ogreTexture.isNull())
        return;

///\todo This function does not quite work, since ogreTexture->getNumMipmaps() will return 0 to denote a "full mipmap chain".

    for(int f = 0; f < ogreTexture->getNumFaces(); ++f)
        for(int i = 1; i < ogreTexture->getNumMipmaps(); ++i)
        {
            Ogre::HardwarePixelBufferSharedPtr src = ogreTexture->getBuffer(f, i-1);
            Ogre::Box srcSize(0, 0, src->getWidth(), src->getHeight());
            Ogre::HardwarePixelBufferSharedPtr dst = ogreTexture->getBuffer(f, i);
            Ogre::Box dstSize(0, 0, dst->getWidth(), dst->getHeight());
            dst->blit(src, srcSize, dstSize);
        }
}
*/
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

void TextureAsset::DoUnload()
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
