/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TransformGizmo.h
 *  @brief  Enables visual manipulators (gizmos) for Transform attributes.
 */

#pragma once

#include "IComponent.h"
#include "InputFwd.h"
#include "Vector3D.h"
#include "OgreModuleFwd.h"
#include "AssetReference.h"

#include <OgreRay.h>

class Quaternion;

/// Enables visual manipulators (gizmos) for Transform attributes.
/** <table class="header">
    <tr>
    <td>
    <h2>EC_TransformGizmo</h2>
    Enables visual manipulators (gizmos) for Transform attributes.

    Doesn't have any attributes.

    <b>Exposes the following scriptable functions:</b>
    <ul>
    <li>"Foo": 
    </ul>

    Doesn't react on any actions.

    Does not emit any actions.

    <b>Depends on EC_Placeable and EC_Mesh</b>.

    </table>
*/
class EC_TransformGizmo : public IComponent
{
    Q_OBJECT
    Q_ENUMS(GizmoType)
    COMPONENT_NAME("EC_TransformGizmo", 30)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    EC_TransformGizmo(Framework *fw);
    ~EC_TransformGizmo();

    ///
    enum GizmoType
    {
        Translate, ///<
        Rotate, ///<
        Scale ///<
    };

public slots:
    ///
    //void SetPivot(float3x4 tm)

    void SetPosition(const Vector3df &pos);

    /// Returns current type of the gizmo.
    GizmoType CurrentGizmoType() const { return gizmoType; }

    /// Sets the type of the gizmo
    /** @type Type of the gizmo. */
    void SetCurrentGizmoType(GizmoType type);

    /// Shows the gizmo.
    void Show();

    /// Hides the gizmo.
    void Hide();

    /// Sets visiblity of the gizmo.
    /** @param visible Visibility of the gizmo. */
    void SetVisible(bool visibile);

    ///
//  bool IsVisible() const;

signals:
    /// Emitted when 
    /** @param offset New offset. */
    void Translated(const Vector3df &offset);

    /// Emitted when 
    /** @param delta Change in rotation. */
    void Rotated(const Quaternion &delta);

    /// Emitted when 
    /** @param offset New offset. */
    void Scaled(const Vector3df &offset);

private:
    InputContextPtr input; ///< Input context for the gizmo.
    boost::shared_ptr<EC_Placeable> placeable; ///< Placeable component.
    boost::shared_ptr<EC_Mesh> mesh; ///< Mesh component.
    GizmoType gizmoType; ///< Current gizmo type.
    OgreRenderer::RendererWeakPtr renderer; ///< Renderer.
    Ogre::Vector3 prevPoint;
    Ogre::Vector3 curPoint;
    // Rays currently as memvars for debugging purposes only.
    Ogre::Ray mouseRay;
    Ogre::Ray xRay, yRay, zRay;
    Ogre::Ray prevMouseRay;

    /// 
    struct GizmoAxis
    {
        static const uint X = 1; ///< Submesh index of the gizmo's x axis.
        static const uint Y = 0; ///< Submesh index of the gizmo's y axis.
        static const uint Z = 2; ///< Submesh index of the gizmo's z axis.
        uint axis; ///< \todo Gizmo mesh must be re-authored so that x=0, y=1, z=2. Currently x=1, y=0, z=2.
        Ogre::Ray ray; ///< 
        AssetReference material; ///<
    };

    QList<GizmoAxis> activeAxes;

    ///
    enum GizmoState
    {
        Inactive, ///< Mouse is not above gizmo.
        Hovering, ///< We're hovering mouse above gizmo.
        Active, ///< Mouse was pressed above gizmo, but currently it might or might not be above the gizmo.
    };

    GizmoState state; ///< Current state of the gizmo.

private slots:
    /// Initializes the gizmo when parent entity is set.
    void Initialize();

    /// Handles mouse events.
    /** @param e Mouse event.
    */
    void HandleMouseEvent(MouseEvent *e);

//    void DrawDebug();
};
