/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.cpp
 *  @brief  Controls EC_TransformGizmos.
 */

#include "StableHeaders.h"
#include "TransformEditor.h"

#include "SceneManager.h"
#include "Entity.h"
#include "EC_Placeable.h"
#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif

TransformEditor::TransformEditor(const ScenePtr &scene)
{
    this->scene = scene;
    CreateGizmo();
}

void TransformEditor::SetSelection(const QList<EntityPtr> &entities)
{
    ClearSelection();
    AppendSelection(entities);
}

void TransformEditor::AppendSelection(const QList<EntityPtr> &entities)
{
    foreach(const EntityPtr &e, entities)
    {
        boost::shared_ptr<EC_Placeable> p = e->GetComponent<EC_Placeable>();
        if (p)
            targets.append(AttributeWeakPtr(p, p->GetAttribute(p->transform.GetName())));
    }
}

void TransformEditor::AppendSelection(const EntityPtr &entity)
{
    AppendSelection(QList<EntityPtr>(QList<EntityPtr>() << entity));
}

void TransformEditor::RemoveFromSelection(const QList<EntityPtr> &entities)
{
}

void TransformEditor::RemoveFromSelection(const EntityPtr &entity)
{
    RemoveFromSelection(QList<EntityPtr>(QList<EntityPtr>() << entity));
}

void TransformEditor::ClearSelection()
{
    targets.clear();
}

void TransformEditor::FocusGizmoPivotToAabbBottomCenter()
{
    if (targets.isEmpty())
        return;

    Vector3df minPos(1e9f, 1e9f, 1e9f);
    Vector3df maxPos(-1e9f, -1e9f, -1e9f);

    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Vector3df pos = transform->Get().position;
            minPos.x = std::min(minPos.x, pos.x);
            minPos.y = std::min(minPos.y, pos.y);
            minPos.z = std::min(minPos.z, pos.z);
            maxPos.x = std::max(maxPos.x, pos.x);
            maxPos.y = std::max(maxPos.y, pos.y);
            maxPos.z = std::max(maxPos.z, pos.z);
        }
    }

    // We assume that world's up axis is Y-coordinate axis.
    Vector3df pivotPos = Vector3df((minPos.x + maxPos.x) / 2, minPos.y, (minPos.z + maxPos.z) / 2);
    if (gizmo)
    {
        EC_TransformGizmo *tg = gizmo->GetComponent<EC_TransformGizmo>().get();
        if (tg)
            tg->SetPosition(pivotPos);
    }
}

void TransformEditor::CreateGizmo()
{
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr s = scene.lock();
    if (!s)
        return;

    gizmo = s->CreateEntity(s->GetNextFreeIdLocal(), QStringList(QStringList() << EC_TransformGizmo::TypeNameStatic()));
    if (!gizmo)
    {
        LogError("!gizmo");
        return;
    }

    EC_TransformGizmo *tg = gizmo->GetComponent<EC_TransformGizmo>().get();
    if (!tg)
    {
        LogError("!tg");
        return;
    }

    connect(tg, SIGNAL(Translated(const Vector3df &)), SLOT(TranslateTargets(const Vector3df &)));
    connect(tg, SIGNAL(Rotated(const Quaternion &)), SLOT(RotateTargets(const Quaternion &)));
    connect(tg, SIGNAL(Scaled(const Vector3df &)), SLOT(ScaleTargets(const Vector3df &)));
#endif
}

void TransformEditor::TranslateTargets(const Vector3df &offset)
{
    foreach(const AttributeWeakPtr attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
        }
    }
}

void TransformEditor::RotateTargets(const Quaternion &delta)
{
    foreach(const AttributeWeakPtr attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
        }
    }
}

void TransformEditor::ScaleTargets(const Vector3df &offset)
{
    foreach(const AttributeWeakPtr attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
        }
    }
}
