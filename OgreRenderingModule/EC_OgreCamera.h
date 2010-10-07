// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCamera_h
#define incl_OgreRenderer_EC_OgreCamera_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Declare_EC.h"

/// Ogre camera entity component
/**
<table class="header">
<tr>
<td>
<h2>OgreCamera</h2>
Ogre camera entity component
Needs to be attached to a placeable (aka scene node) to be useful.

Registered by OgreRenderer::OgreRenderingModule.

\ingroup OgreRenderingModuleClient

<b>No Attributes</b>.

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetActive": sets as active camera in the viewport
<li>"SetNearClip": sets near clip distance
        \note EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param nearclip new near clip distance
<li>"SetFarClip": sets far clip distance
        note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param farclip new far clip distance
<li>"SetVerticalFov": sets vertical fov 
        \param fov new vertical fov in radians 
<li>"GetNearClip": returns near clip distance
<li>"GetFarClip": returns far clip distance
<li>"GetVerticalFov": returns vertical fov as radians
<li>"IsActive": returns whether camera is active in the viewport
<li>"GetCamera": returns the actual Ogre camera.
        use with caution. never set the position of the camera directly, use the placeable component for that.
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table>
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
    OgreRenderer::RendererWeakPtr renderer_;
    
    //! Ogre camera
    Ogre::Camera* camera_;
};

#endif
