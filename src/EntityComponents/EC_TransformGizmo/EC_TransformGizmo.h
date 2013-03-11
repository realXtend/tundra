/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   EC_TransformGizmo.h
    @brief  Enables visual manipulators (gizmos) for Transform attributes. */

#pragma once

#include "IComponent.h"
#include "InputFwd.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"
#include "Geometry/Ray.h"
#include "Math/float3.h"
#include "Math/float3x4.h"
#include "Math/Quat.h"

/// Enables visual manipulators (gizmos) for Transform attributes.
/** <table class="header">
    <tr>
    <td>
    <h2>TransformGizmo</h2>
    Enables visual manipulators (gizmos) for Transform attributes.

    Doesn't have any attributes.

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"SetPosition": @copydoc SetPosition
    <li>"CurrentGizmoType": @copydoc CurrentGizmoType
    <li>"SetCurrentGizmoType": @copydoc SetCurrentGizmoType
    <li>"Show": @copydoc Show
    <li>"Hide": @copydoc Hide
    <li>"SetVisible": @copydoc SetVisible
    <li>"IsVisible":  @copydoc IsVisible
    </ul>

    Doesn't react on any actions.

    Does not emit any actions.

    <b>Depends on EC_Placeable and EC_Mesh</b>.

    </table> */
class EC_TransformGizmo : public IComponent
{
    Q_OBJECT
    Q_ENUMS(GizmoType)
    COMPONENT_NAME("EC_TransformGizmo", 30)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    EC_TransformGizmo(Scene* scene);
    ~EC_TransformGizmo();

    /// Possible types of a gizmo.
    enum GizmoType
    {
        Translate, ///< Translates, i.e. moves, an object.
        Rotate, ///< Rotates an object.
        Scale ///< Scales an object.
    };

    /// Internal states of gizmo.
    enum GizmoState
    {
        Inactive, ///< Mouse is not above gizmo.
        Hovering, ///< We're hovering mouse above gizmo.
        Active, ///< Mouse was pressed above gizmo, and is currently held down (might or might not be above the gizmo).
    };
    
public slots:
    ///\todo implement SetPivot
    //void SetPivot(float3x4 tm)

    /// Sets position of the gizmo.
    /** @param pos New position. */
    void SetPosition(const float3 &pos);

    /// Sets orientation of the gizmo.
    /** @param rot New orientation. */
    void SetOrientation(const Quat &rot);
    
    /// Returns current type of the gizmo.
    GizmoType CurrentGizmoType() const { return gizmoType; }

    /// Sets the type of the gizmo.
    /** @type Type of the gizmo. */
    void SetCurrentGizmoType(GizmoType type);

    /// Sets visiblity of the gizmo.
    /** @param visible Visibility of the gizmo. */
    void SetVisible(bool visible);

    /// Returns visibility of the gizmo.
    bool IsVisible() const;

    /// Return state of gizmo
    GizmoState State() const { return state; }

signals:
    /// Emitted when gizmo is active in Translate mode.
    /** @param offset New offset. */
    void Translated(const float3 &offset);

    /// Emitted when the gizmo is active in Rotate mode.
    /** @param delta Change in rotation. */
    void Rotated(const Quat &delta);

    /// Emitted when the gizmo is active in Translate mode.
    /** @param offset New offset. */
    void Scaled(const float3 &offset);

private:
    /// Represents axis of the gizmo.
    struct GizmoAxis
    {
        ///\todo Gizmo mesh must be re-authored so that x=0, y=1, z=2. Currently x=1, y=0, z=2.
        static const uint X = 1; ///< Submesh index of the gizmo's x axis.
        static const uint Y = 0; ///< Submesh index of the gizmo's y axis.
        static const uint Z = 2; ///< Submesh index of the gizmo's z axis.
        uint axis; ///< Represented coordinate axis.
        Ray ray; ///< Corresponding ray for the represented coordinate axis.
        AssetReference material; ///< Currently used material for the axis' submesh.
    };

    InputContextPtr input; ///< Input context for the gizmo.
    shared_ptr<EC_Placeable> placeable; ///< Placeable component.
    shared_ptr<EC_Mesh> mesh; ///< Mesh component.
    GizmoType gizmoType; ///< Current gizmo type.
    float3x4 worldTM; ///< Transform used for the gizmo rays. Based on the placeable; updated in inactive & hovering states, but not in active state
    float3 worldPos; ///< Gizmo's remembered world position. Based on the placeable; updated in inactive & hovering states, but not in active state
    OgreWorldWeakPtr ogreWorld; ///< OgreWorld.
    float3 prevPoint; ///< Previous nearest projected point on the gizmo's coordinate axes.
    float3 curPoint; ///< Current nearest projected point on the gizmo's coordinate axes.
    GizmoState state; ///< Current state of the gizmo.
    QList<GizmoAxis> activeAxes; ///< Currently active axes.

    /// Computes the scale we want to apply to the transform gizmo to make it constant-sized in the view.
    float DesiredGizmoScale();

private slots:
    /// Initializes the gizmo when parent entity is set.
    void Initialize();

    /// Handles mouse events.
    /** @param e Mouse event. */
    void HandleMouseEvent(MouseEvent *e);

    void OnFrameUpdate(float dt);
};
