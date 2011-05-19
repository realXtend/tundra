/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.h
 *  @brief  Controls EC_TransformGizmos.
 */

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "Vector3D.h"

#include <QObject>

class Quaternion;

struct AttributeWeakPtr
{
    AttributeWeakPtr(const ComponentPtr c, IAttribute *a) : component(c), attribute(a) {}

    IAttribute *Get() const
    {
        ComponentPtr c = component.lock();
        if (c)
            return attribute;
        else
            return 0;
    }

    ComponentWeakPtr component;
    IAttribute *attribute;
};

///
class ECEDITOR_MODULE_API TransformEditor : public QObject
{
    Q_OBJECT

public:
    ///
    /** */
    TransformEditor(const ScenePtr &scene);

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

    ///
    void CreateGizmo();

public slots:
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
    SceneWeakPtr scene;
    EntityPtr gizmo;
    QList<AttributeWeakPtr> targets;
};
