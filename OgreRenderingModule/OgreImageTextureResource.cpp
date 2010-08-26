// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreImageTextureResource.h"
#include "OgreRenderingModule.h"

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
            ogre_texture_ = Ogre::TextureManager::getSingleton().loadImage(id_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, image);
            
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