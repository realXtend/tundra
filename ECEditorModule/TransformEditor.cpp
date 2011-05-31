/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.cpp
 *  @brief  Controls Transform attributes for groups of entities.
 */

#include "StableHeaders.h"
#include "TransformEditor.h"
#include "LoggingFunctions.h"
#include "Scene.h"
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
            targets.append(AttributeWeakPtr(p, p->GetAttribute(p->transform.Name())));
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
            targets.removeOne(AttributeWeakPtr(p, p->GetAttribute(p->transform.Name())));
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

    float3 minPos(1e9f, 1e9f, 1e9f);
    float3 maxPos(-1e9f, -1e9f, -1e9f);

    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            minPos = Min(minPos, transform->Get().pos);
            maxPos = Max(maxPos, transform->Get().pos);
        }
    }

    // We assume that world's up axis is Y-coordinate axis.
    float3 pivotPos = float3((minPos.x + maxPos.x) / 2, minPos.y, (minPos.z + maxPos.z) / 2);
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

void TransformEditor::TranslateTargets(const float3 &offset)
{
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            t.pos += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

float3 TransformEditor::GetGizmoPos() const
{
    if (!gizmo)
        return float3(0,0,0);
    boost::shared_ptr<EC_Placeable> placeable = gizmo->GetComponent<EC_Placeable>();
    if (!placeable)
        return float3(0,0,0);
    return placeable->transform.Get().pos;
}

void TransformEditor::RotateTargets(const Quat &delta)
{
    float3 gizmoPos = GetGizmoPos();
    float3x4 rotation = float3x4::Translate(gizmoPos) * float3x4(delta) * float3x4::Translate(-gizmoPos);
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            Transform t = transform->Get();
            float3x4 f = t.ToFloat3x4();
            Transform q;
            q.FromFloat3x4(f);
            float4x4 g = t.ToFloat4x4();
            Transform w;
            w.FromFloat4x4(g);
//            std::cout << "Org: " << t.ToQString().toStdString() << std::endl;
            t.FromFloat3x4(rotation * t.ToFloat3x4());
//            std::cout << "TMed: " << t.ToQString().toStdString() << std::endl;

//            t.FromFloat3x4(rotation * t.ToFloat3x4());
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

void TransformEditor::ScaleTargets(const float3 &offset)
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

    connect(tg, SIGNAL(Translated(const float3 &)), SLOT(TranslateTargets(const float3 &)));
    connect(tg, SIGNAL(Rotated(const Quat &)), SLOT(RotateTargets(const Quat &)));
    connect(tg, SIGNAL(Scaled(const float3 &)), SLOT(ScaleTargets(const float3 &)));
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
