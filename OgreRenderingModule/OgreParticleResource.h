// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_OgreParticleResource_h
#define incl_OgreRenderer_OgreParticleResource_h

#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "OgreModuleApi.h"

namespace OgreRenderer
{
    class OgreMeshResource;
    typedef boost::shared_ptr<OgreMeshResource> OgreMeshResourcePtr;

    //! An Ogre-specific particle system template resource. One resource may contain multiple templates.
    /*! \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API OgreParticleResource : public Foundation::ResourceInterface
    {
    public:
        //! constructor
        /*! \param id particle system template id
         */
        OgreParticleResource(const std::string& id);
        
        //! constructor
        /*! \param id particle system template id
            \param source asset data to construct templates from
        */
        OgreParticleResource(const std::string& id, Foundation::AssetPtr source);

        //! destructor
        virtual ~OgreParticleResource();

        //! returns whether resource valid
        virtual bool IsValid() const;

        //! sets contents from asset data
        /*! \param source asset data to construct templates from
            \return true if successful
        */
        bool SetData(Foundation::AssetPtr source);
        
        //! returns resource type in text form
        virtual const std::string& GetType() const;
        
        //! returns number of templates
        uint GetNumTemplates() const;
        
        //! returns template name by index
        const std::string& GetTemplateName(uint index) const;

        //! returns resource type in text form (static)
        static const std::string& GetTypeStatic();
    
    private:
        //! removes particle system templates
        void RemoveTemplates();
        
        //! template names
        StringVector templates_;
    };
}

#endif
