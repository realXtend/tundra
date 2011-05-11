/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_TransformGizmo.h
 *  @brief  Enables visual manipulators (gizmos) for Transform attributes.
 */

#pragma once

#include "IComponent.h"
#include "InputFwd.h"

class Vector3df;
class Quaternion;
class EC_Placeable;
class EC_Mesh;

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
    COMPONENT_NAME("EC_TransformGizmo", 29)

public:
    EC_TransformGizmo(Framework *fw);
    ~EC_TransformGizmo();

    enum GizmoType
    {
        Translate,
        Rotate,
        Scale
    };

public slots:
    ///
    //void SetPivot(float3x4 tm)

    ///
    GizmoType CurrentGizmoType() const { return currentType; }

    ///
    void SetCurrentGizmoType(GizmoType type) { currentType = type; }

    ///
//    void Show();

    ///
//    void Hide();

signals:
    ///
    /** @param offset New offset. */
    void Translated(const Vector3df &offset);

    ///
    /** @param delta Change in rotation. */
    void Rotated(const Quaternion &delta);

    ///
    /** @param offset New offset. */
    void Scaled(const Vector3df &offset);

private:
    InputContextPtr input;
    EC_Placeable *placeable;
    EC_Mesh *mesh;
    GizmoType currentType;

private slots:
    /// Handles key events from input service.
    /** @param e Key event.
    */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles mouse events from input service.
    /** @param e Mouse event.
    */
    void HandleMouseEvent(MouseEvent *e);
};
