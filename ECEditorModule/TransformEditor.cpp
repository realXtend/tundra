/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.cpp
 *  @brief  Controls Transform attributes for groups of entities.
 */

#include "StableHeaders.h"
#include "TransformEditor.h"
#include "LoggingFunctions.h"
#include "SceneManager.h"
#include "Entity.h"
#include "EC_Placeable.h"
#include "InputAPI.h"
#include "AssetAPI.h"
#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif

TransformEditor::TransformEditor(const ScenePtr &scene)
{
    if (scene)
    {
        this->scene = scene;
        QString uniqueName("TransformEditor" + scene->GetFramework()->Asset()->GenerateUniqueAssetName("",""));
        input = scene->GetFramework()->Input()->RegisterInputContext(uniqueName, 100);
        connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        CreateGizmo();
    }
}

TransformEditor::~TransformEditor()
{
    DeleteGizmo();
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
    foreach(const EntityPtr &e, entities)
    {
        boost::shared_ptr<EC_Placeable> p = e->GetComponent<EC_Placeable>();
        if (p)
            targets.removeOne(AttributeWeakPtr(p, p->GetAttribute(p->transform.GetName())));
    }
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
#ifdef EC_TransformGizmo_ENABLED
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
#endif
}

void TransformEditor::SetGizmoVisible(bool show)
{
#ifdef EC_TransformGizmo_ENABLED
    if (gizmo)
    {
        EC_TransformGizmo *tg = gizmo->GetComponent<EC_TransformGizmo>().get();
        if (tg)
            tg->SetVisible(show);
    }
#endif
}

void TransformEditor::TranslateTargets(const Vector3df &offset)
{
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            t.position += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

void TransformEditor::RotateTargets(const Quaternion &delta)
{
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            ///\todo Implement!
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

void TransformEditor::ScaleTargets(const Vector3df &offset)
{
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            t.scale += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
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
        LogError("TransformEditor: could not create gizmo entity.");
        return;
    }

    EC_TransformGizmo *tg = gizmo->GetComponent<EC_TransformGizmo>().get();
    if (!tg)
    {
        LogError("TransformEditor: could not acquire EC_TransformGizmo.");
        return;
    }

    connect(tg, SIGNAL(Translated(const Vector3df &)), SLOT(TranslateTargets(const Vector3df &)));
    connect(tg, SIGNAL(Rotated(const Quaternion &)), SLOT(RotateTargets(const Quaternion &)));
    connect(tg, SIGNAL(Scaled(const Vector3df &)), SLOT(ScaleTargets(const Vector3df &)));
#endif
}

void TransformEditor::DeleteGizmo()
{
    ScenePtr s = scene.lock();
    if (s && gizmo)
        s->RemoveEntity(gizmo->GetId());
}

void TransformEditor::HandleKeyEvent(KeyEvent *e)
{
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr scn = scene.lock();
    EC_TransformGizmo *tg = 0;
    if (gizmo && scn)
    {
        tg = gizmo->GetComponent<EC_TransformGizmo>().get();
        if (!tg)
            return;
    }

    InputAPI *inputApi = scn->GetFramework()->Input();
    const QKeySequence &translate= inputApi->KeyBinding("SetTranslateGizmo", QKeySequence(Qt::Key_1));
    const QKeySequence &rotate = inputApi->KeyBinding("SetRotateGizmo", QKeySequence(Qt::Key_2));
    const QKeySequence &scale = inputApi->KeyBinding("SetScaleGizmo", QKeySequence(Qt::Key_3));
    if (e->sequence == translate)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Translate);
    if (e->sequence == rotate)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Rotate);
    if (e->sequence == scale)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Scale);
#endif
}
