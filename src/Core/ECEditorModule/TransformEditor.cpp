/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   TransformEditor.cpp
 *  @brief  Controls Transform attributes for groups of entities.
 */

#include "StableHeaders.h"
#include "TransformEditor.h"
#include "LoggingFunctions.h"
#include "OgreWorld.h"
#include "Scene.h"
#include "Entity.h"
#include "EC_Camera.h"
#include "EC_Light.h"
#include "EC_Placeable.h"
#include "FrameAPI.h"
#include "InputAPI.h"
#include "AssetAPI.h"
#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif
#include "Profiler.h"

TransformEditor::TransformEditor(const ScenePtr &scene)
{
    if (scene)
    {
        this->scene = scene;
        QString uniqueName("TransformEditor" + scene->GetFramework()->Asset()->GenerateUniqueAssetName("",""));
        input = scene->GetFramework()->Input()->RegisterInputContext(uniqueName, 100);
        connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(scene->GetFramework()->Frame(), SIGNAL(Updated(float)), SLOT(OnUpdated(float)));
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
    if (!gizmo)
        CreateGizmo();
    foreach(const EntityPtr &e, entities)
    {
        boost::shared_ptr<EC_Placeable> p = e->GetComponent<EC_Placeable>();
        if (p)
        {
            Entity *parentPlaceableEntity = p->ParentPlaceableEntity();
            EntityPtr parent = (parentPlaceableEntity ? parentPlaceableEntity ->shared_from_this() : EntityPtr());
            targets.append(AttributeWeakPtr(p, p->GetAttribute(p->transform.Name()), parent));
        }
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
        {
            Entity *parentPlaceableEntity = p->ParentPlaceableEntity();
            EntityPtr parent = (parentPlaceableEntity ? parentPlaceableEntity ->shared_from_this() : EntityPtr());
            targets.append(AttributeWeakPtr(p, p->GetAttribute(p->transform.Name()), parent));
        }
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
    if (!gizmo)
        CreateGizmo();

    PROFILE(TransformEditor_FocusGizmoPivotToAabbBottomCenter);

    // If all objects have the same parent, parent the gizmo with the common parent for all the objects.
    entity_id_t prevParentId = 0;
    Entity *parentEntity = 0;
    foreach(const AttributeWeakPtr &attr, targets)
        if (attr.Get())
        {
            parentEntity = attr.parentPlaceableEntity.lock().get();
            if (parentEntity)
            {
                if (prevParentId == 0)
                    prevParentId = parentEntity->Id();
                if (prevParentId != parentEntity->Id())
                {
                    parentEntity = 0;
                    break;
                }
            }
        }

    gizmo->GetComponent<EC_Placeable>()->SetParent(parentEntity, true);

    float3 minPos(1e9f, 1e9f, 1e9f);
    float3 maxPos(-1e9f, -1e9f, -1e9f);

    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            float3 worldPos = transform->Owner()->ParentEntity()->GetComponent<EC_Placeable>()->WorldPosition();
            minPos = Min(minPos, worldPos);
            maxPos = Max(maxPos, worldPos);
        }
    }

    // We assume that world's up axis is Y-coordinate axis.
    float3 pivotPos = float3((minPos.x + maxPos.x) / 2.f, minPos.y, (minPos.z + maxPos.z) / 2.f);
    if (parentEntity)
        pivotPos = parentEntity->GetComponent<EC_Placeable>()->WorldToLocal().MulPos(pivotPos);
    if (gizmo)
    {
        EC_TransformGizmo *tg = gizmo->GetComponent<EC_TransformGizmo>().get();
        if (tg)
            tg->SetPosition(pivotPos);
        ///\todo Hack: for some odd reason when gizmo is parented its scale will go to zero. Hack the scale to stay the same.
        gizmo->GetComponent<EC_Placeable>()->SetScale(1,1,1);
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

float3 TransformEditor::GizmoPos() const
{
    if (!gizmo)
        return float3::zero;
    boost::shared_ptr<EC_Placeable> placeable = gizmo->GetComponent<EC_Placeable>();
    if (!placeable)
        return float3::zero;
    return placeable->transform.Get().pos;
}

void TransformEditor::TranslateTargets(const float3 &offset)
{
    PROFILE(TransformEditor_TranslateTargets);
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            // If selected object's parent is also selected, do not apply changes to the child object.
            if (TargetsContainAlsoParent(attr))
                continue;

            Transform t = transform->Get();
            // If we have parented transform, translate the changes to parent's world space.
            Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
            if (parentPlaceableEntity)
                t.pos += parentPlaceableEntity->GetComponent<EC_Placeable>()->WorldToLocal().MulDir(offset);
            else
                t.pos += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

void TransformEditor::RotateTargets(const Quat &delta)
{
    PROFILE(TransformEditor_RotateTargets);
    float3 gizmoPos = GizmoPos();
    float3x4 rotation = float3x4::Translate(gizmoPos) * float3x4(delta) * float3x4::Translate(-gizmoPos);
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            // If selected object's parent is also selected, do not apply changes to the child object.
            if (TargetsContainAlsoParent(attr))
                continue;

            Transform t = transform->Get();
            t.FromFloat3x4(rotation * t.ToFloat3x4());
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

void TransformEditor::ScaleTargets(const float3 &offset)
{
    PROFILE(TransformEditor_ScaleTargets);
    foreach(const AttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            // If selected object's parent is also selected, do not apply changes to the child object.
            if (TargetsContainAlsoParent(attr))
                continue;

            Transform t = transform->Get();
            t.scale += offset;
            transform->Set(t, AttributeChange::Default);
        }
    }

    FocusGizmoPivotToAabbBottomCenter();
}

bool TransformEditor::TargetsContainAlsoParent(const AttributeWeakPtr &attr) const
{
    Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
    if (!parentPlaceableEntity)
        return false; // Not parented, ignore.

    foreach(const AttributeWeakPtr &target, targets)
        if (target.Get() && target.Get() != attr.Get() && target.Get()->Owner()->ParentEntity() == parentPlaceableEntity)
            return true;

    return false;
}

void TransformEditor::CreateGizmo()
{
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr s = scene.lock();
    if (!s)
        return;

    gizmo = s->CreateLocalEntity(QStringList(QStringList() << EC_TransformGizmo::TypeNameStatic()), AttributeChange::LocalOnly, false);
    if (!gizmo)
    {
        LogError("TransformEditor: could not create gizmo entity.");
        return;
    }
    gizmo->SetTemporary(true);

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
        s->RemoveEntity(gizmo->Id());
}

void TransformEditor::HandleKeyEvent(KeyEvent *e)
{
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr scn = scene.lock();
    if (!scn)
        return;
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

void TransformEditor::OnUpdated(float frameTime)
{
    if (targets.size() && !scene.expired())
    {
        OgreWorldPtr ogreWorld = scene.lock()->GetWorld<OgreWorld>();
        if (ogreWorld)
        {
            for(int i = 0; i < targets.size(); ++i)
            {
                if (!targets[i].owner.expired())
                {
                    Entity* ent = targets[i].owner.lock()->ParentEntity();
                    if (ent)
                        DrawDebug(ogreWorld.get(), ent);
                }
            }
        }
    }
}

void TransformEditor::DrawDebug(OgreWorld* world, Entity* entity)
{
    const Entity::ComponentMap& components = entity->Components();
    for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
    {
        u32 type = i->second->TypeId();
        if (type == EC_Placeable::TypeIdStatic())
        {
            EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(i->second.get());
            // Draw without depth test to ensure the axes do not get lost within a mesh for example
            world->DebugDrawAxes(float3x4::FromTRS(placeable->WorldPosition(), placeable->WorldOrientation(), placeable->WorldScale()), false);
        }
        if (type == EC_Light::TypeIdStatic())
        {
            EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
            if (placeable)
            {
                EC_Light* light = checked_static_cast<EC_Light*>(i->second.get());
                const Color& color = light->diffColor.Get();
                world->DebugDrawLight(float3x4(placeable->WorldOrientation(), placeable->WorldPosition()), light->type.Get(), light->range.Get(), light->outerAngle.Get(), color.r, color.g, color.b);
            }
        }
        if (type == EC_Camera::TypeIdStatic())
        {
            EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
            if (placeable)
                world->DebugDrawCamera(float3x4(placeable->WorldOrientation(), placeable->WorldPosition()), 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}
