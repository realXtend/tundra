// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCamera_h
#define incl_OgreRenderer_EC_OgreCamera_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "Declare_EC.h"

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
    class OGRE_MODULE_API EC_OgreCamera : public IComponent
    {
        Q_OBJECT
        
        DECLARE_EC(EC_OgreCamera);
    public:
        virtual ~EC_OgreCamera();
        
        //! sets placeable component
        /*! set a null placeable to detach the camera, otherwise will attach
            \param placeable placeable component
         */
        void SetPlaceable(ComponentPtr placeable);
    public slots:
 
        //! sets as active camera in the viewport
        void SetActive();
        
        //! sets near clip distance
        /*! note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
            \param nearclip new near clip distance
         */ 
        void SetNearClip(float nearclip);
        
        //! sets far clip distance
        /*! note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
            \param farclip new far clip distance
         */         
        void SetFarClip(float farclip);
        
        //! sets vertical fov 
        /*! \param fov new vertical fov in radians 
         */
        void SetVerticalFov(float fov);
        
        //! returns near clip distance
        float GetNearClip() const;
        
        //! returns far clip distance
        float GetFarClip() const;
        
        //! returns vertical fov as radians
        float GetVerticalFov() const;
        
        //! returns whether camera is active in the viewport
        bool IsActive() const;
        
        //! returns the actual Ogre camera.
        /*! use with caution. never set the position of the camera directly, use the placeable component for that.
         */
        Ogre::Camera* GetCamera() { return camera_; }
                
    private:
        //! constructor
        /*! \param module renderer module
         */
        explicit EC_OgreCamera(IModule* module);
        
        //! attaches camera to placeable
        void AttachCamera();
        
        //! detaches camera from placeable
        void DetachCamera();
        
        //! placeable component 
        ComponentPtr placeable_;
 
        //! attached to placeable -flag
        bool attached_;
        
        //! renderer ptr
        RendererWeakPtr renderer_;
        
        //! Ogre camera
        Ogre::Camera* camera_;
    };
}

#endif
