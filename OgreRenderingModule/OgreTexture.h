// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreTexture_h
#define incl_OgreRenderer_OgreTexture_h

#include "TextureInterface.h"

#include <OgreTexture.h>

namespace OgreRenderer
{
    //! an Ogre texture resource
    class OgreTexture;
    typedef boost::shared_ptr<OgreTexture> OgreTexturePtr;

    class OgreTexture : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id texture id
         */
        OgreTexture(const std::string& id);
        
        //! constructor
        /*! \param id texture id
            \param source source raw texture data
        */
        OgreTexture(const std::string& id, Foundation::TexturePtr source);

        //! destructor
        virtual ~OgreTexture();

        //! returns resource type in text form
        virtual const std::string& GetTypeName() const;

        //! returns Ogre texture
        /*! may be null if no data successfully set yet
         */
        Ogre::TexturePtr GetTexture() const { return ogre_texture_; }

        //! returns quality level
        int GetLevel() const { return level_; }

        //! sets contents from raw source texture
        /*! \param source source raw texture data
        */
        bool SetData(Foundation::TexturePtr source);

    private:
        // ogre texture
        Ogre::TexturePtr ogre_texture_;

        // quality level, 0 = highest, -1 = no content yet
        int level_;
    };
}
#endif
