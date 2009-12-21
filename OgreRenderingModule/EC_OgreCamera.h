// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCamera_h
#define incl_OgreRenderer_EC_OgreCamera_h

#include "Foundation.h"
#include "ComponentInterface.h"
#include "OgreModuleApi.h"

namespace Ogre
{
    class Camera;
}

namespace OgreRenderer
{
    class Renderer;
    class EC_OgrePlaceable;
    
    typedef boost::shared_ptr<Renderer> RendererPtr;
    typedef boost::weak_ptr<Renderer> RendererWeakPtr;
    
    //! Ogre camera entity component
    /*! Needs to be attached to a placeable (aka scene node) to be useful.
        \ingroup OgreRenderingModuleClient
     */
    class OGRE_MODULE_API EC_OgreCamera : public Foundation::ComponentInterface
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreCamera);
    public:
        virtual ~EC_OgreCamera();
        
        //! sets placeable component
        /*! set a null placeable to detach the camera, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(Foundation::ComponentPtr placeable);
 
    private:
        //! constructor
        /*! \param module renderer module
         */
        EC_OgreCamera(Foundation::ModuleInterface* module);
        
        //! attaches camera to placeable
        void AttachCamera();
        
        //! detaches camera from placeable
        void DetachCamera();
        
        //! placeable component 
        Foundation::ComponentPtr placeable_;
 
        //! attached to placeable -flag
        bool attached_;
        
        //! renderer ptr
        RendererWeakPtr renderer_;
        
        //! Ogre camera
        Ogre::Camera* camera_;
    };
}

#endif
