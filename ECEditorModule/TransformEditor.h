/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.h
 *  @brief  Controls Transform attributes for groups of entities.
 */

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
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

    ComponentWeakPtr owner; ///< Owner component.
    IAttribute *attribute; ///< The actual attribute.
};

/// Controls Transform attributes for groups of entities.
class ECEDITOR_MODULE_API TransformEditor : public QObject
{
    Q_OBJECT

public:
    ///
    /** */
    TransformEditor(const ScenePtr &scene);
    ~TransformEditor();

    ///
    /** @param entities */
    void SetSelection(const QList<EntityPtr> &entities);

    ///
    /** @param entities */
    void AppendSelection(const QList<EntityPtr> &entities);

    /// This is an overloaded function.
    /** @param entity */
    void AppendSelection(const EntityPtr &entity);

    ///
    /** @param */
    void RemoveFromSelection(const QList<EntityPtr> &entities);

    /// This is an overloaded function.
    /** @param */
    void RemoveFromSelection(const EntityPtr &entity);

    ///
    void ClearSelection();

    ///
    void FocusGizmoPivotToAabbBottomCenter();

public slots:
    /// 
    void SetGizmoVisible(bool show);

    ///
    /** @param */
    void TranslateTargets(const Vector3df &offset);

    ///
    /** @param */
    void RotateTargets(const Quaternion &delta);

    ///
    /** @param */
    void ScaleTargets(const Vector3df &offset);

private:
    /// Creates transform gizmo for the editor.
    void CreateGizmo();

    /// Destroyes editor's transform gizmo.
    void DeleteGizmo();

    SceneWeakPtr scene;
    EntityPtr gizmo;
    QList<AttributeWeakPtr> targets;
};
