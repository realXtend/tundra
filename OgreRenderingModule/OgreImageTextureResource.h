// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreImageTextureResource_h
#define incl_OgreRenderer_OgreImageTextureResource_h

#include "AssetInterface.h"
#include "TextureInterface.h"
#include "OgreModuleApi.h"

#include <OgreTexture.h>

namespace OgreRenderer
{
    class OgreImageTextureResource;
    typedef boost::shared_ptr<OgreImageTextureResource> OgreImageTextureResourcePtr;

    //! An Ogre-specific texture resource, created from image not J2K stream
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API OgreImageTextureResource : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id texture id
         */
        OgreImageTextureResource(const std::string& id);
        
        //! constructor
        /*! \param id texture id
            \param source source image
        */
        OgreImageTextureResource(const std::string& id, Foundation::AssetPtr source);

        //! destructor
        virtual ~OgreImageTextureResource();

        //! returns resource type in text form
        virtual const std::string& GetType() const;

        //! returns whether resource valid
        virtual bool IsValid() const;
        
        //! returns Ogre texture
        /*! may be null if no data successfully set yet
         */
        Ogre::TexturePtr GetTexture() const { return ogre_texture_; }
        
        //! returns whether has alpha channel
        bool HasAlpha() const;
        
        //! sets contents from raw source texture
        /*! \param source source raw texture data
            \return true if successful
         */
        bool SetData(Foundation::AssetPtr source);
        
        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();

    private:
        //! Remove texture
        void RemoveTexture();
        
        //! Ogre texture
        Ogre::TexturePtr ogre_texture_;
    };
}
#endif
