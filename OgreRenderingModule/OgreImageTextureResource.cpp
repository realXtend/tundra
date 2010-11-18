// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreImageTextureResource.h"
#include "OgreRenderingModule.h"
#include "OgreConversionUtils.h"

#include <Ogre.h>

namespace OgreRenderer
{
    OgreImageTextureResource::OgreImageTextureResource(const std::string& id, TextureQuality texturequality) : 
        ResourceInterface(id),
        texturequality_(texturequality)
    {
    }

    OgreImageTextureResource::OgreImageTextureResource(const std::string& id, TextureQuality texturequality, Foundation::AssetPtr source) : 
        ResourceInterface(id),
        texturequality_(texturequality)
    {
        SetData(source);
    }

    OgreImageTextureResource::~OgreImageTextureResource()
    {
        RemoveTexture();
    }
    
    bool OgreImageTextureResource::SetData(Foundation::AssetPtr source)
    {
        if (!source)
        {
            OgreRenderingModule::LogError("Null source image data pointer");
            return false;
        }
        if (!source->GetSize())
        {
            OgreRenderingModule::LogError("Zero sized image asset");     
            return false;
        }
        try
        {
            RemoveTexture();

            Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream((void*)source->GetData(), source->GetSize(), false));
            Ogre::Image image;
            image.load(stream);
            if (texturequality_ == Texture_Low)
                image.resize(image.getWidth() / 2, image.getHeight() / 2);
            ogre_texture_ = Ogre::TextureManager::getSingleton().loadImage(SanitateAssetIdForOgre(id_), Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image);
            internal_name_ = SanitateAssetIdForOgre(id_);
        }
        catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogError("Failed to create image texture " + id_ + ": " + std::string(e.what()));
            RemoveTexture();
            return false;
        }

        OgreRenderingModule::LogDebug("Ogre image texture " + id_ + " created");
        return true;
    }

    bool OgreImageTextureResource::Export(const std::string &filename) const
    {
        if (ogre_texture_.isNull())
        {
            OgreRenderingModule::LogWarning("Tried to export non-existing Ogre texture " + id_ + ".");
            return false;
        }

        try
        {
            //Ogre::Image image;
            //ogre_texture_->convertToImage(image);
            Ogre::Image new_image;
                    
            // From Ogre 1.7 Texture::convertToImage()
            size_t numMips = 1;
            size_t dataSize = Ogre::Image::calculateSize(numMips,
                ogre_texture_->getNumFaces(), ogre_texture_->getWidth(), ogre_texture_->getHeight(), ogre_texture_->getDepth(), ogre_texture_->getFormat());
            void* pixData = OGRE_MALLOC(dataSize, Ogre::MEMCATEGORY_GENERAL);
            // if there are multiple faces and mipmaps we must pack them into the data
            // faces, then mips
            void* currentPixData = pixData;
            for (size_t face = 0; face < ogre_texture_->getNumFaces(); ++face)
            {
                for (size_t mip = 0; mip < numMips; ++mip)
                {
                    size_t mipDataSize = Ogre::PixelUtil::getMemorySize(ogre_texture_->getWidth(), ogre_texture_->getHeight(), ogre_texture_->getDepth(), ogre_texture_->getFormat());
                    Ogre::PixelBox pixBox(ogre_texture_->getWidth(), ogre_texture_->getHeight(), ogre_texture_->getDepth(), ogre_texture_->getFormat(), currentPixData);
                    ogre_texture_->getBuffer(face, mip)->blitToMemory(pixBox);
                    currentPixData = (void*)((char*)currentPixData + mipDataSize);
                }
            }
            // load, and tell Image to delete the memory when it's done.
            new_image.loadDynamicImage((Ogre::uchar*)pixData, ogre_texture_->getWidth(), ogre_texture_->getHeight(), ogre_texture_->getDepth(), ogre_texture_->getFormat(), true, 
                ogre_texture_->getNumFaces(), numMips - 1);

            new_image.save(filename);
        } catch (std::exception &e)
        {
            OgreRenderingModule::LogError("Failed to export Ogre texture " + id_ + ":");
            if (e.what())
                OgreRenderingModule::LogError(e.what());
            return false;
        }
        return true;
    }

    bool OgreImageTextureResource::HasAlpha() const
    {
        if (ogre_texture_.get())
        {
            // As far as the legacy materials are concerned, DXT1 is not alpha
            if (ogre_texture_->getFormat() == Ogre::PF_DXT1)
                return false;
            if (Ogre::PixelUtil::hasAlpha(ogre_texture_->getFormat()))
                return true;
        }
        
        return false;
    }

    static const std::string type_name("OgreImageTexture");
        
    const std::string& OgreImageTextureResource::GetType() const
    {
        return type_name;
    }
    
    const std::string& OgreImageTextureResource::GetTypeStatic()
    {
        return type_name;
    }    
    
    bool OgreImageTextureResource::IsValid() const
    {
        return (!ogre_texture_.isNull());
    }
    
    void OgreImageTextureResource::RemoveTexture()
    {
        if (!ogre_texture_.isNull())
        {
            std::string tex_name = ogre_texture_->getName();
            ogre_texture_.setNull(); 
            
            try
            {
                Ogre::TextureManager::getSingleton().remove(tex_name);
            }
            catch (...) {}
        }
    }
}