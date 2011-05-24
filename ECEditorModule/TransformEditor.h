/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.h
 *  @brief  Controls Transform attributes for groups of entities.
 */

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "InputFwd.h"
#include "Vector3D.h"

#include <QObject>

class Quaternion;

/// Represents weak pointer to IAttribute.
struct AttributeWeakPtr
{
    /// Constructor.
    /** @param c Owner component.
        @param a The actual attribute.
    */
    AttributeWeakPtr(const ComponentPtr c, IAttribute *a) : owner(c), attribute(a) {}

    /// Returns pointer to the attribute or null if the owner component doens't exist anymore.
    IAttribute *Get() const { return owner.lock() ? attribute : 0; }

    bool operator ==(const AttributeWeakPtr &rhs) const
    {
        ComponentPtr ownerPtr = owner.lock();
        return rhs.owner.lock() == ownerPtr && (rhs.attribute == attribute || !ownerPtr);
    }

    bool operator !=(const AttributeWeakPtr &rhs) const { return !(*this == rhs); }

    ComponentWeakPtr owner; ///< Owner component.
    IAttribute *attribute; ///< The actual attribute.
};

/// Controls Transform attributes for groups of entities.
/** Can be used to alter transforms of entities even without the visual gizmo (EC_TransformGizmo).*/
class ECEDITOR_MODULE_API TransformEditor : public QObject
{
    Q_OBJECT

public:
    /// Constructs the editor.
    /** Creates EC_TransformGizmo if it is available.
        @scene Scene in which the edited entities reside.
    */
    TransformEditor(const ScenePtr &scene);

    /// Destroys the editor.
    /** Destroys the EC_TransformGizmo if it was created. */
    ~TransformEditor();

    /// Sets new selection of entities, clears possible previous selection.
    /** @param entities Entities to be added. */
    void SetSelection(const QList<EntityPtr> &entities);

    /// Appends selection with new entities.
    /** @param entities Entities to be added. */
    void AppendSelection(const QList<EntityPtr> &entities);

    /// This is an overloaded function.
    /** @param entity Entity to be added. */
    void AppendSelection(const EntityPtr &entity);

    /// Removes entities from selection.
    /** @param entities Entities to be removed. */
    void RemoveFromSelection(const QList<EntityPtr> &entities);

    /// This is an overloaded function.
    /** @param entity Entity to be removed. */
    void RemoveFromSelection(const EntityPtr &entity);

    /// Clears the selection.
    void ClearSelection();

    /// Focuses the position of gizmo (if used) to bottom center of AABB
    void FocusGizmoPivotToAabbBottomCenter();

    /// Sets visibility of the gizmo (if used).
    void SetGizmoVisible(bool show);

public slots:
    /// Translates current target transforms.
    /** @param offset Offset to be applied. */
    void TranslateTargets(const Vector3df &offset);

    /// Rotates current target transforms.
    /** @param delta Change to be applied. */
    void RotateTargets(const Quaternion &delta);

    /// Scales current target transforms.
    /** @param offset Offset to be applied. */
    void ScaleTargets(const Vector3df &offset);

private:
    /// Creates transform gizmo for the editor.
    void CreateGizmo();

    /// Destroys editor's transform gizmo.
    void DeleteGizmo();

    SceneWeakPtr scene; ///< Scene in which the edited entities reside.
    EntityPtr gizmo; ///< Gizmo entity.
    QList<AttributeWeakPtr> targets; ///< Current target transform attributes.
    InputContextPtr input; ///< Input context for controlling gizmo mode.

private slots:
    /// Handles KeyEvents and changes gizmo's mode.
    /** @param e Key event. */
    void HandleKeyEvent(KeyEvent *e);
};
