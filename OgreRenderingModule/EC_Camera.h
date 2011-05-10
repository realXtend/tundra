// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_OgreCamera_h
#define incl_OgreRenderer_EC_OgreCamera_h

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"

#include "Vector3D.h"

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

<b>Attributes</b>:
<ul>
<li>Vector3df: upVector
<div>Up vector that defines the yaw axis.</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"AutoSetPlaceable": sets placeable from the same entity
<li>"SetActive": sets as active camera in the viewport
<li>"SetNearClip": sets near clip distance
        \note EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param nearclip new near clip distance
<li>"SetFarClip": sets far clip distance
        note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param farclip new far clip distance
<li>"SetVerticalFov": sets vertical fov 
        \param fov new vertical fov in radians
<li>"GetInitialRotation": returns initial Euler rotation according to the up vector.
<li>"GetAdjustedRotation": returns an adjusted Euler rotation according to the up vector.
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
class OGRE_MODULE_API EC_Camera : public IComponent
{
    Q_OBJECT

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Camera(Framework *fw);

    virtual ~EC_Camera();

    /// Camera up vector. Defines the yaw axis
    Q_PROPERTY(Vector3df upVector READ getupVector WRITE setupVector);
    DEFINE_QPROPERTY_ATTRIBUTE(Vector3df, upVector);

    /// sets placeable component
    /** set a null placeable to detach the camera, otherwise will attach
        \param placeable placeable component
     */
    void SetPlaceable(ComponentPtr placeable);

    COMPONENT_NAME("EC_Camera", 15)
public slots:

    /// automatically find the placeable and set it
    void AutoSetPlaceable();
    
    /// sets as active camera in the viewport
    void SetActive();
    
    /// Get an initial rotation for the camera (in Euler angles, can be assigned to a Transform) that corresponds to the up vector
    /// Note: the left/right & front/back axes are unspecified
    Vector3df GetInitialRotation() const;
    
    /// Adjust a pitch/yaw/roll Euler rotation vector using the up vector
    Vector3df GetAdjustedRotation(const Vector3df& rotation) const;
    
    /// sets near clip distance
    /** note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param nearclip new near clip distance
     */ 
    void SetNearClip(float nearclip);
    
    /// sets far clip distance
    /** note that EC_OgreEnviroment will override what you set here, based on whether camera is under/over water!
        \param farclip new far clip distance
     */         
    void SetFarClip(float farclip);
    
    /// sets vertical fov 
    /** \param fov new vertical fov in radians 
     */
    void SetVerticalFov(float fov);
    
    /// returns near clip distance
    float GetNearClip() const;
    
    /// returns far clip distance
    float GetFarClip() const;
    
    /// returns vertical fov as radians
    float GetVerticalFov() const;
    
    /// returns whether camera is active in the viewport
    bool IsActive() const;
    
    /// returns the actual Ogre camera.
    /** use with caution. never set the position of the camera directly, use the placeable component for that.
     */
    Ogre::Camera* GetCamera() const { return camera_; }

/* The following functions moved here from RenderServiceInterface. Reimplement them:

    /// take sceenshot to a location
    /// \param filePath File path.
    /// \param fileName File name.
    virtual void TakeScreenshot(const std::string& filePath, const std::string& fileName) = 0;

    /// Render current main window content to texture
    virtual QPixmap RenderImage(bool use_main_camera = true) = 0; */

private slots:
    /// Called when the parent entity has been set.

    void UpdateSignals();
    /// Called when component has been removed from the parent entity. Checks if the component removed was the mesh, and autodissociates it.
    void OnComponentRemoved(IComponent* component, AttributeChange::Type change);

private:
    
    /// attaches camera to placeable
    void AttachCamera();
    
    /// detaches camera from placeable
    void DetachCamera();
    
    /// placeable component 
    ComponentPtr placeable_;

    /// attached to placeable -flag
    bool attached_;
    
    /// renderer ptr
    OgreRenderer::RendererWeakPtr renderer_;
    
    /// Ogre camera
    Ogre::Camera* camera_;
};

#endif
