// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreTextureResource_h
#define incl_OgreRenderer_OgreTextureResource_h

#include "TextureInterface.h"
#include "AssetInterface.h"
#include "OgreModuleApi.h"
#include "Renderer.h"

#include <OgreTexture.h>

namespace OgreRenderer
{
    class OgreTextureResource;
    typedef boost::shared_ptr<OgreTextureResource> OgreTextureResourcePtr;

    //! An Ogre-specific texture resource
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API OgreTextureResource : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id texture id
            \param texturequality texture quality
         */
        OgreTextureResource(const std::string& id, TextureQuality texturequality);
        
        //! constructor
        /*! \param id texture id
            \param texturequality texture quality
            \param source source raw texture data
        */
        OgreTextureResource(const std::string& id, TextureQuality texturequality, Foundation::TexturePtr source);

        //! destructor
        virtual ~OgreTextureResource();

        //! returns resource type in text form
        virtual const std::string& GetType() const;

        //! returns whether resource valid
        virtual bool IsValid() const;
        
        //! returns Ogre texture
        /*! may be null if no data successfully set yet
         */
        Ogre::TexturePtr GetTexture() const { return ogre_texture_; }

        //! returns quality level
        int GetLevel() const { return level_; }
        
        //! returns whether has alpha channel
        bool HasAlpha() const;
        
        //! sets contents from an image file
        /*! \param source source image asset
            \return true if successful
         */
        bool SetDataFromImage(Foundation::AssetPtr source);
        
        //! sets contents from raw source texture
        /*! \param source source raw texture data
            \return true if successful
         */
        bool SetData(Foundation::TexturePtr source);
        
        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();

    private:
        //! Remove texture
        void RemoveTexture();
        
        //! Ogre texture
        Ogre::TexturePtr ogre_texture_;

        //! quality level, 0 = highest, -1 = no content yet
        int level_;
        
        //! Texture quality
        TextureQuality texturequality_;
    };
}
#endif
