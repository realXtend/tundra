// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingSystem_EC_OgreSky_h
#define incl_OgreRenderingSystem_EC_OgreSky_h

#include "ComponentInterface.h"
#include "Foundation.h"

namespace Ogre
{
    class SceneNode;
}

namespace OgreRenderer
{
    class Renderer;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    
    //! Ogre sky component
    /*! \todo other sky methods, like plane or dome
     */
    class MODULE_API EC_OgreSky : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OgreSky);
    public:
        virtual ~EC_OgreSky();
        
        //! enables a skybox
        /*! other sky methods will be disabled
            \param material_name material name
            \distance distance of skybox from camera
            \return true if successful
            \todo use material/texture asset reference when asset system exists
         */
        bool SetSkyBox(const std::string& material_name, Core::Real distance);
        
        //! disables sky methods
        void DisableSky();
        
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreSky(Foundation::ModuleInterface* module);

        //! renderer
        RendererPtr renderer_;
        
        //! whether sky enabled by this component
        bool sky_enabled_;
    };
}

#endif