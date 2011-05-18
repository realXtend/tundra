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
    TransformEditor();

//    void AddEntities();
//    void RemoveEntities();
//    void ClearSelection();

    void SetSelectedEntities(const QList<EntityPtr> &entities);
    void FocusGizmoPivotToAabbBottomCenter();
    void CreateGizmo();

public slots:
    ///
    /** @param */
    void TranslateTargets(const Vector3df offset);

    ///
    /** @param */
    void RotateTargets(const Quaternion &delta);

    ///
    /** @param */
    void ScaleTargets(const Vector3df &offset);

private:
    EntityPtr gizmo;
    QList<AttributeWeakPtr> targets;
};
