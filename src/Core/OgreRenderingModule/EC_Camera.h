// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "OgreModuleApi.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Geometry/Frustum.h"
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
/** <table class="header">
    <tr>
    <td>
    <h2>Camera</h2>
    Ogre camera entity component
    Needs to be attached to a placeable (aka scene node) to be useful.

    Registered by OgreRenderer::OgreRenderingModule.

    \ingroup OgreRenderingModuleClient

    <b>Attributes</b>:
    <ul>
    <li>float3: upVector
    <div> @copydoc .</div>
    <li>float: nearPlane
    <div> @copydoc </div>
    <li>float: farPlane
    <div> @copydoc </div>
    <li>float3: verticalFov
    <div> @copydoc </div>
    <li>QString: aspectRatio
    <div> @copydoc </div>
    </ul>

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"SetActive": @copydoc SetActive
    <li>"IsActive": @copydoc IsActive
    <li>"InitialRotation": @copydoc InitialRotation
    <li>"AdjustedRotation": @copydoc AdjustedRotation
    <li>"ViewportPointToRay": @copydoc ViewportPointToRay
    <li>"ScreenPointToRay": @copydoc ScreenPointToRay
    <li>"AspectRatio": @copydoc AspectRatio
    <li>"VisibleEntities": @copydoc VisibleEntities
    <li>"VisibleEntityIDs": @copydoc VisibleEntityIDs
    <li>"StartViewTracking": @copydoc StartViewTracking
    <li>"StopViewTracking": @copydoc StopViewTracking
    <li>"AspectRatio": @copydoc AspectRatio
    <li>"SetAspectRatio": @copydoc SetAspectRatio
    <li>"ViewMatrix": @copydoc ViewMatrix
    <li>"ProjectionMatrix": @copydoc ProjectionMatrix
    <li>"SetFromFrustum": @copydoc ViewMatrix
    <li>"ToFrustum": @copydoc ProjectionMatrix
    </ul>

    Does not react on any actions.

    Does not emit any actions.

    <b>Depends on the component @ref EC_ Placeable "Placeable".</b>
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
    /**  Alternatively float can be used too, e.g. "1.33". If this string is empty, the aspect ratio of the main window viewport is used (default). */
    Q_PROPERTY(QString aspectRatio READ getaspectRatio WRITE setaspectRatio);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, aspectRatio);

    /// Returns the actual Ogre camera.
    /** @note Use with caution. Never set the position of the camera directly, use the placeable component for that. */
    Ogre::Camera* OgreCamera() const { return camera_; }

    /// Render current view to a Ogre::Image. Returns null Ogre::Image if operation fails.
    /** Tundra rendering viewport size is used as the image size.
        @param renderUi If the image should have the user interface included.
        @return The render result image. */
    Ogre::Image ToOgreImage(bool renderUi = true);

    /// Returns entity IDs of visible entities in the camera's frustum.
    const std::set<entity_id_t>& VisibleEntityIDs();

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

    /// Returns the currently used view aspect ratio (width/height).
    float AspectRatio() const;

    /// Sets the currently used view aspect ratio (width/height).
    /** @note Does not change the value of the aspectRatio attribute.
        @note The aspectRatio attribute must be != "" (i.e. we're not using automatic aspect ratio) otherwise this functions has not effect. */
    void SetAspectRatio(float ratio);

    /// Returns whether an entity is visible in the camera's frustum
    bool IsEntityVisible(Entity* entity);

    /// Returns visible entities in the camera's frustum.
    QList<Entity*> VisibleEntities();

    /// Takes a screen shot to hard drive.
    /** Store location will be users app data directory to make the function script safe. The name will have a timestamp identifier. 
        You can rename/remove etc. the file after this function returns to make it suitable for you usage.
        Tundra rendering viewport size is used as the image size. If you want to a spesific sized image use ToQImage()
        function and resize the returned image to suit your needs.
        @return Absolute filepath to the image file. Empty string if operation fails.
        @note The timestamp format is yyyy-MM-dd-hh:mm. Image format is PNG. */
    QString SaveScreenshot(bool renderUi = true);

    /// Renders current view to a QImage.
    /** Tundra rendering viewport size is used as the image size.
        @param renderUi If the image should have the user interface included.
        @return The render result image, null QImage if operation fails. */
    QImage ToQImage(bool renderUi = true);

    /// Returns a world space ray as cast from the camera through a viewport position.
    /** @param x The x position at which the ray should intersect the viewport, in normalized screen coordinates [0,1].
        @param y The y position at which the ray should intersect the viewport, in normalized screen coordinates [0,1].
        @sa ScreenPointToRay */
    Ray ViewportPointToRay(float x, float y) const;

    /// Returns a world space ray as cast from the camera through a screen (graphics scene) position.
    /** @param x The x screen position.
        @param y The y screen position.
        @sa ViewportPointToRay */
    Ray ScreenPointToRay(uint x, uint y) const;

    /// Starts tracking an entity's visibility within the scene using this camera
    /** After this, connect either to the camera's EntityEnterView and EntityLeaveView signals,
        or the entity's EnterView & LeaveView signals, to be notified of the visibility change(s) */
    void StartViewTracking(Entity* entity);

    /// Stops tracking an entity's visibility
    void StopViewTracking(Entity* entity);

    /// Returns the view matrix for this camera, float4x4::nan if not applicable.
    float4x4 ViewMatrix() const;

    /// Returns the projection matrix for this camera, float4x4::nan if not applicable.
    float4x4 ProjectionMatrix() const;

    /// Returns the camera's perspective viewing frustum, Frustum::type == InvalidFrustum if not applicable.
    Frustum ToFrustum() const;

    /// Sets the camera's view from a perspective viewing frustum.
    void SetFromFrustum(const Frustum &f);

    // DEPRECATED
    Ray GetMouseRay(float x, float y) const { return ViewportPointToRay(x, y); } /**< @deprecated use ViewportPointToRay @todo Add warning and remove at some point. */
    void SetNearClip(float nearclip); /**< @deprecated use attribute nearPlane @todo Remove */
    float NearClip() const; /**< @deprecated use attribute nearPlane @todo Remove */
    void SetFarClip(float farclip); ///< @deprecated use attribute farPlane @todo Remove */
    float FarClip() const; /**< @deprecated use attribute farPlane @todo Remove */
    void SetVerticalFov(float fov); /**< @deprecated use attribute verticalFov @note fov as degrees @todo Remove */
    float VerticalFov() const; /**< @deprecated use attribute verticalFov @note returns radians @todo Remove */
    Ogre::Camera* GetCamera() const { return OgreCamera(); } /**< @deprecated use OgreCamera @todo Add warning and remove at some point. */

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

    /// Handle frame update. Used for entity visibility tracking
    void OnUpdated(float timeStep);

private:

    void AttributesChanged();

    /// Sets placeable component
    /** set a null placeable to detach the camera, otherwise will attach
        @param placeable placeable component */
    void SetPlaceable(const ComponentPtr &placeable);

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
    Ogre::TexturePtr UpdateRenderTexture(bool renderUi);

    void SetNearClipDistance(float distance);
    void SetFarClipDistance(float distance);
    void SetFovY(float fov);

    /// placeable component 
    ComponentPtr placeable_;

    /// Attached to placeable -flag. Not ideal but needed due to Ogre crappiness (Ogre::SceneNode::getAttachedObject throws an expection if no object found)...
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
