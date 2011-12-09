/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   TransformEditor.h
 *  @brief  Controls Transform attributes for groups of entities.
 */

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "InputFwd.h"

#include <QObject>

class OgreWorld;

/// Represents weak pointer to Transform attribute.
/// @cond PRIVATE
struct AttributeWeakPtr
{
    /// Constructor.
    /** @param c Owner component.
        @param a The actual attribute.
        @param If the placeable component is parented, pointer to the parent placeable entity. */
    AttributeWeakPtr(const ComponentPtr &c, IAttribute *a, const EntityPtr &p) :
        owner(c),
        attribute(a),
        parentPlaceableEntity(p)
    {
    }

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
    EntityWeakPtr parentPlaceableEntity; ///< If the placeable component is parented, points to the parent placeable entity.
};
///endcond

/// Controls Transform attributes for groups of entities.
/** Can be used to alter transforms of entities even without the visual gizmo (EC_TransformGizmo).*/
class ECEDITOR_MODULE_API TransformEditor : public QObject
{
    Q_OBJECT

public:
    /// Constructs the editor.
    /** Creates EC_TransformGizmo if it is available.
        @param scene Scene in which the edited entities reside. */
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

    /// Focuses the transform of gizmo (if used) to center of AABB
    void FocusGizmoPivotToAabbCenter();

    /// Sets visibility of the gizmo (if used).
    void SetGizmoVisible(bool show);

    /// Returns position of the editing gizmo.
    float3 GizmoPos() const;

public slots:
    /// Translates current target transforms.
    /** @param offset Offset to be applied. */
    void TranslateTargets(const float3 &offset);

    /// Rotates current target transforms.
    /** @param delta Change to be applied. */
    void RotateTargets(const Quat &delta);

    /// Scales current target transforms.
    /** @param offset Offset to be applied. */
    void ScaleTargets(const float3 &offset);

private:
    /// Returns whether or not transform attribute @attr is parented and current selection of targets contain also the parent.
    bool TargetsContainAlsoParent(const AttributeWeakPtr &attr) const;

    /// Creates transform gizmo for the editor.
    void CreateGizmo();

    /// Destroys editor's transform gizmo.
    void DeleteGizmo();

    /// Draw debug visualization for an entity.
    void DrawDebug(OgreWorld* world, Entity* entity);

    SceneWeakPtr scene; ///< Scene in which the edited entities reside.
    EntityPtr gizmo; ///< Gizmo entity.
    QList<AttributeWeakPtr> targets; ///< Current target transform attributes.
    InputContextPtr input; ///< Input context for controlling gizmo mode.
    QWidget* editorSettings; ///< Editor settings window
    bool localAxes; ///< Whether to show object local axes instead of global world axes.

private slots:
    /// Handles KeyEvents and changes gizmo's mode.
    /** @param e Key event. */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles frame update. Redraws debug visualizations as necessary.
    void OnUpdated(float frameTime);
    
    /// Gizmo mode has been selected from the settings widget.
    void OnGizmoModeSelected(int mode);
    /// Gizmo axis has been selected from the settings widget.
    void OnGizmoAxisSelected(int axis);
};
