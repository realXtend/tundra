// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Geometry/Ray.h"

#include <QImage>
#include <QSize>
#include <set>

#include <OgreImage.h>

namespace Ogre
{
    class PlaneBoundedVolumeListSceneQuery;
}

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
<li>float3: upVector
<div>Up vector that defines the yaw axis.</div>
<li>float: nearPlane
<div>Near clip distance.</div>
<li>float: farPlane
<div>Far clip distance.</div>
<li>float3: verticalFov
<div>Vertical field of view as degrees.</div>
<li>QString: aspectRatio
<div>Aspect ratio is a string of form "<widthProportion>:<heightProportion>", e.g. "4:3".</div>
</ul>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"SetActive": sets as active camera in the viewport
<li>"IsActive": returns whether camera is active in the viewport
<li>"InitialRotation": returns initial Euler rotation according to the up vector.
<li>"AdjustedRotation": returns an adjusted Euler rotation according to the up vector.
<li>"GetMouseRay": Returns a world space ray as cast from the camera through a viewport position.
<li>"AspectRatio": Returns the currently used view aspect ratio (width/height).
<li>"VisibleEntities": Returns visible entities in the camera's frustum.
<li>"VisibleEntityIDs": Returns entity IDs of visible entities in the camera's frustum.
<li>"StartViewTracking": Starts tracking an entity's visibility within the scene using this camera.
<li>"StopViewTracking": Stops tracking an entity's visibility.
<li>"AspectRatio": Returns the currently used view aspect ratio (width/height).
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component Placeable</b>.
</table> */
class OGRE_MODULE_API EC_Camera : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_Camera", 15)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_Camera(Scene* scene);
    virtual ~EC_Camera();

    /// Camera up vector. Defines the yaw axis
    Q_PROPERTY(float3 upVector READ getupVector WRITE setupVector);
    DEFINE_QPROPERTY_ATTRIBUTE(float3, upVector);

    /// Near clip distance.
    ///\todo Rename to nearClipDistance?
    Q_PROPERTY(float nearPlane READ getnearPlane WRITE setnearPlane);
    DEFINE_QPROPERTY_ATTRIBUTE(float, nearPlane);

    /// Far clip distance.
    ///\todo Rename to farClipDistance?
    Q_PROPERTY(float farPlane READ getfarPlane WRITE setfarPlane);
    DEFINE_QPROPERTY_ATTRIBUTE(float, farPlane);

    /// Vertical field of view as degrees.
    Q_PROPERTY(float verticalFov READ getverticalFov WRITE setverticalFov);
    DEFINE_QPROPERTY_ATTRIBUTE(float, verticalFov);

    /// Aspect ratio is a string of form "<widthProportion>:<heightProportion>", e.g. "4:3".
    /** If this string is empty, the aspect ratio of the main window viewport is used (default). */
    Q_PROPERTY(QString aspectRatio READ getaspectRatio WRITE setaspectRatio);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, aspectRatio);

public slots:
    /// Sets this camera as the active main window camera.
    /// Calling this function is equivalent to calling Renderer::SetMainCamera(this).
    void SetActive();

    /// Returns whether camera is active in the viewport
    bool IsActive() const;

    /// Get an initial rotation for the camera (in Euler angles, can be assigned to a Transform) that corresponds to the up vector
    /// @note the left/right & front/back axes are unspecified
    float3 InitialRotation() const;

    /// Adjust a pitch/yaw/roll Euler rotation vector using the up vector
    float3 AdjustedRotation(const float3& rotation) const;

    /// Sets near clip distance
    /** @param nearclip new near clip distance */
    ///\todo Remove, exposed as Attribute nearPlane
    void SetNearClip(float nearclip);
    /// returns near clip distance
    ///\todo Remove, exposed as Attribute nearPlane
    float NearClip() const;

    /// sets far clip distance
    /** @param farclip new far clip distance */
    ///\todo Remove, exposed as Attribute farPlane
    void SetFarClip(float farclip);
    /// returns far clip distance
    ///\todo Remove, exposed as Attribute farPlane
    float FarClip() const;

    /// Sets vertical fov 
    /** @param fov new vertical fov in radians  */
    ///\todo Remove, exposed as Attribute verticalFov
    void SetVerticalFov(float fov);
    /// returns vertical fov as radians
    ///\todo Remove, exposed as Attribute verticalFov
    float VerticalFov() const;

    /// Returns the currently used view aspect ratio (width/height).
    float AspectRatio() const;

    /// Returns the actual Ogre camera.
    /** @note Use with caution. Never set the position of the camera directly, use the placeable component for that. */
    Ogre::Camera* GetCamera() const { return camera_; }

    /// Returns whether an entity is visible in the camera's frustum
    bool IsEntityVisible(Entity* entity);

    /// Returns visible entities in the camera's frustum.
    QList<Entity*> VisibleEntities();

    /// Returns entity IDs of visible entities in the camera's frustum.
    const std::set<entity_id_t>& VisibleEntityIDs();

    /// Takes a screen shot to hard drive. Store location will be users app data directory 
    /// to make the function script safe. The name will have a timestamp identifier. 
    /// You can rename/remove etc. the file after this function returns to make it suitable for you usage.
    /// Tundra rendering viewport size is used as the image size. If you want to a spesific sized image use ToQImage()
    /// function and resize the returned image to suit your needs.
    /// @return Absolute filepath to the image file. Empty string if operation fails.
    /// @note The timestamp format is yyyy-MM-dd-hh:mm. Image format is PNG.
    QString SaveScreenshot(bool renderUi = true);

    /// Render current view to a QImage. Returns null QImage if operation fails. 
    /// Tundra rendering viewport size is used as the image size.
    /// @param renderUi If the image should have the user interface included.
    /// @return The render result image.
    QImage ToQImage(bool renderUi = true);
   
    /// Render current view to a Ogre::Image. Returns null Ogre::Image if operation fails.
    /// Tundra rendering viewport size is used as the image size.
    /// @param renderUi If the image should have the user interface included.
    /// @return The render result image.
    Ogre::Image ToOgreImage(bool renderUi = true);

    /// Returns a world space ray as cast from the camera through a viewport position.
    /** @param The x position at which the ray should intersect the viewport, in normalized screen coordinates [0,1].
        @param The y position at which the ray should intersect the viewport, in normalized screen coordinates [0,1]. */
    Ray GetMouseRay(float x, float y) const;

    /// Starts tracking an entity's visibility within the scene using this camera
    /** After this, connect either to the camera's EntityEnterView and EntityLeaveView signals,
        or the entity's EnterView & LeaveView signals, to be notified of the visibility change(s) */
    void StartViewTracking(Entity* entity);

    /// Stops tracking an entity's visibility
    void StopViewTracking(Entity* entity);

signals:
    /// An entity has entered the view
    void EntityEnterView(Entity* entity);

    /// An entity has left the view
    void EntityLeaveView(Entity* entity);

private slots:
    /// Called when the parent entity has been set.
    void UpdateSignals();

    /// Called when component has been added or removed from the parent entity. Checks the existence of the EC_Placeable component, and attaches this camera to it.
    void OnComponentStructureChanged();

    void OnAttributeUpdated(IAttribute *attribute);

    /// Handle frame update. Used for entity visibility tracking
    void OnUpdated(float timeStep);

private:
    /// Sets placeable component
    /** set a null placeable to detach the camera, otherwise will attach
        @param placeable placeable component */
    void SetPlaceable(ComponentPtr placeable);

    /// attaches camera to placeable
    void AttachCamera();

    /// detaches camera from placeable
    void DetachCamera();

    /// Deletes the Ogre camera associated with this EC_Camera from the Ogre scene.
    /// After calling this function, the internal camera_ pointer is null.
    void DestroyOgreCamera();

    /// Perform a frustum query for visible entities
    void QueryVisibleEntities();

    /// Update the render texture with the current view.
    bool UpdateRenderTexture(QSize textureSize, bool renderUi);
    

    void SetNearClipDistance(float distance);
    void SetFarClipDistance(float distance);
    void SetFovY(float fov);

    /// placeable component 
    ComponentPtr placeable_;

    /// attached to placeable -flag
    bool attached_;

    /// Ogre world ptr
    OgreWorldWeakPtr world_;

    /// Ogre camera
    Ogre::Camera* camera_;

    /// Frame number on which a full frustum query was last performed
    int queryFrameNumber_;

    /// Visible entity ID's during this frame
    std::set<entity_id_t> visibleEntities_;

    /// Visible entity ID's during last frame
    std::set<entity_id_t> lastVisibleEntities_;

    /// Entities being tracked for visibility changes
    std::vector<EntityWeakPtr> visibilityTrackedEntities_;

    /// Frustum query
    Ogre::PlaneBoundedVolumeListSceneQuery *query_;

    /// Render texture name used in SaveScreenshot(), ToQImage() and ToOgreImage() functions.
    std::string renderTextureName_;
};
