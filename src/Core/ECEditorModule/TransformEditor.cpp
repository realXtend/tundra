/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   TransformEditor.cpp
    @brief  Controls Transform attributes for group of entities. */

#include "StableHeaders.h"
#include "TransformEditor.h"
#include "LoggingFunctions.h"
#include "OgreWorld.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "EC_Camera.h"
#include "EC_Light.h"
#include "EC_Sound.h"
#include "EC_Placeable.h"
#include "FrameAPI.h"
#include "InputAPI.h"
#include "AssetAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UndoCommands.h"
#include "UndoManager.h"
#ifdef EC_TransformGizmo_ENABLED
#include "EC_TransformGizmo.h"
#endif
#include "Profiler.h"
#include "ConfigAPI.h"
#include "Application.h"

#include <QUiLoader>

#include "MemoryLeakCheck.h"

static const char *cTransformEditorWindowPos = "transform editor window pos";

TransformEditor::TransformEditor(const ScenePtr &editedScene, UndoManager *manager) :
    editorSettings(0),
    localAxes(false),
    scene(editedScene),
    undoManager(manager)
{
    if (!scene.expired())
    {
        QString uniqueName("TransformEditor" + scene.lock()->GetFramework()->Asset()->GenerateUniqueAssetName("",""));
        input = scene.lock()->GetFramework()->Input()->RegisterInputContext(uniqueName, 100);
        connect(input.get(), SIGNAL(KeyEventReceived(KeyEvent *)), SLOT(HandleKeyEvent(KeyEvent *)));
        connect(scene.lock()->GetFramework()->Frame(), SIGNAL(Updated(float)), SLOT(OnUpdated(float)));
    }
}

TransformEditor::~TransformEditor()
{
    DeleteGizmo();
}

QList<EntityPtr> TransformEditor::Selection() const
{
    QList<EntityPtr> ret;
    foreach(const TransformAttributeWeakPtr &attr, targets)
        if (!attr.owner.expired() && attr.owner.lock()->ParentEntity())
            ret.append(attr.owner.lock()->ParentEntity()->shared_from_this());
    return ret;
}

void TransformEditor::SetSelection(const QList<EntityPtr> &entities)
{
    ClearSelection();
    AppendSelection(entities);
    
    // If settings window was hidden, show it again now
    if (editorSettings && !editorSettings->isVisible() && !targets.isEmpty())
        editorSettings->show();
}

void TransformEditor::AppendSelection(const QList<EntityPtr> &entities)
{
    if (!gizmo)
        CreateGizmo();
    foreach(const EntityPtr &e, entities)
    {
        shared_ptr<EC_Placeable> p = e->Component<EC_Placeable>();
        if (p)
        {
            Entity *parentPlaceableEntity = p->ParentPlaceableEntity();
            EntityPtr parent = (parentPlaceableEntity ? parentPlaceableEntity ->shared_from_this() : EntityPtr());
            targets.append(TransformAttributeWeakPtr(p, p->AttributeById(p->transform.Id()), parent));
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
        shared_ptr<EC_Placeable> p = e->Component<EC_Placeable>();
        if (p)
        {
            Entity *parentPlaceableEntity = p->ParentPlaceableEntity();
            EntityPtr parent = (parentPlaceableEntity ? parentPlaceableEntity ->shared_from_this() : EntityPtr());
            targets.append(TransformAttributeWeakPtr(p, p->AttributeById(p->transform.Id()), parent));
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

void TransformEditor::FocusGizmoPivotToAabbCenter()
{
#ifdef EC_TransformGizmo_ENABLED
    if (targets.isEmpty())
        return;
    if (!gizmo)
        CreateGizmo();

    PROFILE(TransformEditor_FocusGizmoPivotToAabbCenter);

    // If all objects have the same parent, parent the gizmo with the common parent for all the objects.
    // Currently this logic is not used.
    /*
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
    */

    float3 minPos(1e9f, 1e9f, 1e9f);
    float3 maxPos(-1e9f, -1e9f, -1e9f);

    float3 pivotPos;
    Quat pivotRot = Quat::identity;
    bool useLocalAxisRotation = localAxes;
    
    foreach(const TransformAttributeWeakPtr &attr, targets)
    {
        Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
        if (transform)
        {
            ///\todo The gizmo is placed according to the target placeable position, although it is meant as a generic transform editor. Refactor needed in the future.
            EC_Placeable* placeable = transform->Owner()->ParentEntity()->Component<EC_Placeable>().get();
            if (placeable)
            {
                float3 worldPos = placeable->WorldPosition();
                minPos = Min(minPos, worldPos);
                maxPos = Max(maxPos, worldPos);
                // When multiple targets are selected, only use the local axes if orientations are equal in all targets, else revert to world axes
                if (useLocalAxisRotation)
                {
                    if (attr == targets[0])
                        pivotRot = placeable->WorldOrientation();
                    else if (!pivotRot.Equals(placeable->WorldOrientation()))
                        useLocalAxisRotation = false; // Mismatch, revert to world axes
                }
            }
        }
    }
    pivotPos = float3((minPos.x + maxPos.x) / 2.f, (minPos.y + maxPos.y) / 2.f, (minPos.z + maxPos.z) / 2.f);
    
    if (gizmo)
    {
        EC_TransformGizmo *tg = TransformGizmo();
        if (tg)
        {
            tg->SetPosition(pivotPos);
            if (useLocalAxisRotation)
                tg->SetOrientation(pivotRot);
            else
                tg->SetOrientation(Quat::identity);
        }
    }
#endif
}

void TransformEditor::SetGizmoVisible(bool show)
{
#ifdef EC_TransformGizmo_ENABLED
    EC_TransformGizmo *tg = TransformGizmo();
    if (tg)
        tg->SetVisible(show);
#endif
}

float3 TransformEditor::GizmoPos() const
{
    if (!gizmo)
        return float3::zero;
    shared_ptr<EC_Placeable> placeable = gizmo->Component<EC_Placeable>();
    if (!placeable)
        return float3::zero;
    return placeable->transform.Get().pos;
}

EC_TransformGizmo *TransformEditor::TransformGizmo() const
{
#ifdef EC_TransformGizmo_ENABLED
    if (!gizmo)
        return 0;
    return gizmo->Component<EC_TransformGizmo>().get();
#else
    return 0;
#endif
}

void TransformEditor::Translate(const TransformAttributeWeakPtr &attr, const float3 &offset, AttributeChange::Type change)
{
    Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
    if (transform)
    {
        Transform t = transform->Get();
        // If we have parented transform, translate the changes to parent's world space.
        Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
        EC_Placeable *parentPlaceable = parentPlaceableEntity ? parentPlaceableEntity->GetComponent<EC_Placeable>().get() : 0;
        if (parentPlaceable)
            t.pos += parentPlaceable->WorldToLocal().MulDir(offset);
        else
            t.pos += offset;
        transform->Set(t, change);
    }
}

void TransformEditor::Rotate(const TransformAttributeWeakPtr &attr, const float3x4 &rotation, AttributeChange::Type change)
{
    Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
    if (transform)
    {
        Transform t = transform->Get();
        // If we have parented transform, translate the changes to parent's world space.
        Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
        EC_Placeable* parentPlaceable = parentPlaceableEntity ? parentPlaceableEntity->GetComponent<EC_Placeable>().get() : 0;
        if (parentPlaceable)
            t.FromFloat3x4(parentPlaceable->WorldToLocal() * rotation * parentPlaceable->LocalToWorld() * t.ToFloat3x4());
        else
            t.FromFloat3x4(rotation * t.ToFloat3x4());
        transform->Set(t, change);
    }
}

void TransformEditor::Scale(const TransformAttributeWeakPtr &attr, const float3 &offset, AttributeChange::Type change)
{
    Attribute<Transform> *transform = dynamic_cast<Attribute<Transform> *>(attr.Get());
    if (transform)
    {
        Transform t = transform->Get();
        t.scale += offset;
        transform->Set(t, change);
    }
}

void TransformEditor::TranslateTargets(const float3 &offset)
{
    PROFILE(TransformEditor_TranslateTargets);

    TransformAttributeWeakPtrList targetAttrs;
    foreach(const TransformAttributeWeakPtr &attr, targets)
    {
        // If selected object's parent is also selected, do not apply changes to the child object.
        if (!TargetsContainAlsoParent(attr))
            targetAttrs << attr;
    }

    if (undoManager)
    {
        TransformCommand::Action action = TransformCommand::Translate;
#ifdef EC_TransformGizmo_ENABLED
        EC_TransformGizmo *tg = TransformGizmo();
        EC_TransformGizmo::GizmoAxisList axes = (tg ? tg->ActiveAxes() : EC_TransformGizmo::GizmoAxisList());
        if (axes.size() == 1)
        {
            EC_TransformGizmo::GizmoAxis activeAxis = axes.first();
            action = (activeAxis.axis == EC_TransformGizmo::GizmoAxis::X ? TransformCommand::TranslateX :
                (activeAxis.axis == EC_TransformGizmo::GizmoAxis::Y ? TransformCommand::TranslateY : TransformCommand::TranslateZ));
        }
#endif
        undoManager->Push(new TransformCommand(targetAttrs, targets.size(), action, offset));
    }
    else
    {
        foreach(const TransformAttributeWeakPtr &attr, targets)
            Translate(attr, offset, AttributeChange::Default);
    }

    FocusGizmoPivotToAabbCenter();
}

void TransformEditor::RotateTargets(const Quat &delta)
{
    PROFILE(TransformEditor_RotateTargets);

    float3 gizmoPos = GizmoPos();
    float3x4 rotation = float3x4::Translate(gizmoPos) * float3x4(delta) * float3x4::Translate(-gizmoPos);
    TransformAttributeWeakPtrList targetAttrs;

    foreach(const TransformAttributeWeakPtr &attr, targets)
    {
        // If selected object's parent is also selected, do not apply changes to the child object.
        if (!TargetsContainAlsoParent(attr))
            targetAttrs << attr;
    }

    if (undoManager)
    {
        TransformCommand::Action action = TransformCommand::Rotate;
#ifdef EC_TransformGizmo_ENABLED
        EC_TransformGizmo *tg = TransformGizmo();
        EC_TransformGizmo::GizmoAxisList axes = (tg ? tg->ActiveAxes() : EC_TransformGizmo::GizmoAxisList());
        if (axes.size() == 1)
        {
            EC_TransformGizmo::GizmoAxis activeAxis = axes.first();
            action = (activeAxis.axis == EC_TransformGizmo::GizmoAxis::X ? TransformCommand::RotateX :
                (activeAxis.axis == EC_TransformGizmo::GizmoAxis::Y ? TransformCommand::RotateY : TransformCommand::RotateZ));
        }
#endif
        undoManager->Push(new TransformCommand(targetAttrs, targets.size(), action, rotation));
    }
    else
    {
        foreach(const TransformAttributeWeakPtr &attr, targets)
            Rotate(attr, rotation, AttributeChange::Default);
    }

    FocusGizmoPivotToAabbCenter();
}

void TransformEditor::ScaleTargets(const float3 &offset)
{
    PROFILE(TransformEditor_ScaleTargets);

    TransformAttributeWeakPtrList targetAttrs;
    foreach(const TransformAttributeWeakPtr &attr, targets)
    {
        // If selected object's parent is also selected, do not apply changes to the child object.
        if (TargetsContainAlsoParent(attr))
            continue;
        targetAttrs << attr;
    }

    if (undoManager)
    {
        TransformCommand::Action action = TransformCommand::Scale;
#ifdef EC_TransformGizmo_ENABLED
        EC_TransformGizmo *tg = TransformGizmo();
        EC_TransformGizmo::GizmoAxisList axes = (tg ? tg->ActiveAxes() : EC_TransformGizmo::GizmoAxisList());
        if (axes.size() == 1)
        {
            EC_TransformGizmo::GizmoAxis activeAxis = axes.first();
            action = (activeAxis.axis == EC_TransformGizmo::GizmoAxis::X ? TransformCommand::ScaleX :
                (activeAxis.axis == EC_TransformGizmo::GizmoAxis::Y ? TransformCommand::ScaleY : TransformCommand::ScaleZ));
        }
#endif
        undoManager->Push(new TransformCommand(targetAttrs, targets.size(), action, offset));
    }
    else
    {
        foreach(const TransformAttributeWeakPtr &attr, targets)
            Scale(attr, offset, AttributeChange::Default);
    }

    FocusGizmoPivotToAabbCenter();
}

bool TransformEditor::TargetsContainAlsoParent(const TransformAttributeWeakPtr &attr) const
{
    Entity *parentPlaceableEntity = attr.parentPlaceableEntity.lock().get();
    if (!parentPlaceableEntity)
        return false; // Not parented, ignore.

    foreach(const TransformAttributeWeakPtr &target, targets)
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

    gizmo = s->CreateLocalEntity(QStringList(EC_TransformGizmo::TypeNameStatic()), AttributeChange::LocalOnly, false, true);
    if (!gizmo)
    {
        LogError("TransformEditor::CreateGizmo: could not create gizmo entity.");
        return;
    }

    EC_TransformGizmo *tg = TransformGizmo();
    if (!tg)
    {
        LogError("TransformEditor::CreateGizmo: could not acquire EC_TransformGizmo.");
        return;
    }

    connect(tg, SIGNAL(Translated(const float3 &)), SLOT(TranslateTargets(const float3 &)));
    connect(tg, SIGNAL(Rotated(const Quat &)), SLOT(RotateTargets(const Quat &)));
    connect(tg, SIGNAL(Scaled(const float3 &)), SLOT(ScaleTargets(const float3 &)));
    
    // Create editor window for choosing gizmo mode
    QUiLoader loader;
    loader.setLanguageChangeEnabled(true);
    QFile file(Application::InstallationDirectory() + "data/ui/EditorSettings.ui");
    file.open(QFile::ReadOnly);
    if (!file.exists())
    {
        LogError("TransformEditor::CreateGizmo: Cannot find " + Application::InstallationDirectory() + "data/ui/EditorSettings.ui file.");
        return;
    }
    editorSettings = loader.load(&file, s->GetFramework()->Ui()->MainWindow());
    file.close();
    if (!editorSettings)
    {
        LogError("Could not load editor settings layout");
        return;
    }
    
    QComboBox* modeCombo = editorSettings->findChild<QComboBox*>("modeComboBox");
    if (modeCombo)
        connect(modeCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGizmoModeSelected(int)));
    QComboBox* axisCombo = editorSettings->findChild<QComboBox*>("axisComboBox");
    if (axisCombo)
        connect(axisCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnGizmoAxisSelected(int)));

    // Load position from config
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, "eceditor", cTransformEditorWindowPos);
    QPoint pos = s->GetFramework()->Config()->Get(configData).toPoint();
    UiMainWindow::EnsurePositionWithinDesktop(editorSettings, pos);
    editorSettings->setWindowFlags(Qt::Tool);
#endif
}

void TransformEditor::DeleteGizmo()
{
    ScenePtr s = scene.lock();
    if (s && gizmo)
        s->RemoveEntity(gizmo->Id());

    if (s && s->GetFramework() && s->GetFramework()->Config() && editorSettings) // Save position to config.
    {
        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, "eceditor", cTransformEditorWindowPos);
        s->GetFramework()->Config()->Set(configData, cTransformEditorWindowPos, editorSettings->pos());
    }
    SAFE_DELETE(editorSettings);
}

void TransformEditor::HandleKeyEvent(KeyEvent *e)
{
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr scn = scene.lock();
    if (!scn)
        return;
    EC_TransformGizmo *tg = TransformGizmo();
    if (!tg)
        return;

    InputAPI *inputApi = scn->GetFramework()->Input();
    const QKeySequence translate= inputApi->KeyBinding("TransformEditor.SetTranslateGizmo", QKeySequence(Qt::Key_1));
    const QKeySequence rotate = inputApi->KeyBinding("TransformEditor.SetRotateGizmo", QKeySequence(Qt::Key_2));
    const QKeySequence scale = inputApi->KeyBinding("TransformEditor.SetScaleGizmo", QKeySequence(Qt::Key_3));
    QComboBox* modeCombo = editorSettings ? editorSettings->findChild<QComboBox*>("modeComboBox") : 0;
    if (modeCombo)
    {
        if (e->sequence == translate)
            modeCombo->setCurrentIndex(0);
        if (e->sequence == rotate)
            modeCombo->setCurrentIndex(1);
        if (e->sequence == scale)
            modeCombo->setCurrentIndex(2);
    }
    else
    {
        if (e->sequence == translate)
            tg->SetCurrentGizmoType(EC_TransformGizmo::Translate);
        if (e->sequence == rotate)
            tg->SetCurrentGizmoType(EC_TransformGizmo::Rotate);
        if (e->sequence == scale)
            tg->SetCurrentGizmoType(EC_TransformGizmo::Scale);
    }
#endif
}

void TransformEditor::OnGizmoModeSelected(int mode)
{
    if (mode < 0)
        return;
#ifdef EC_TransformGizmo_ENABLED
    ScenePtr scn = scene.lock();
    if (!scn)
        return;
    EC_TransformGizmo *tg = TransformGizmo();
    if (!tg)
        return;
    
    if (mode == 0)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Translate);
    if (mode == 1)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Rotate);
    if (mode == 2)
        tg->SetCurrentGizmoType(EC_TransformGizmo::Scale);
#endif
}

void TransformEditor::OnGizmoAxisSelected(int axis)
{
    if (axis < 0)
        return;
    
    localAxes = axis != 0;
    FocusGizmoPivotToAabbCenter();
}

void TransformEditor::OnUpdated(float /*frameTime*/)
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
        
#ifdef EC_TransformGizmo_ENABLED
        // If gizmo is not active (ie. no drag going on), update the axes
        // This is needed to update local mode rotation after a rotation edit is finished, as well as to make it follow autonomously moving objects
        EC_TransformGizmo *tg = TransformGizmo();
        if (tg && tg->State() != EC_TransformGizmo::Active)
            FocusGizmoPivotToAabbCenter();
#endif
    }
}

void TransformEditor::DrawDebug(OgreWorld* world, Entity* entity)
{
    const Entity::ComponentMap& components = entity->Components();
    for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
    {
        switch(i->second->TypeId())
        {
        case EC_Placeable::ComponentTypeId:
            {
                EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(i->second.get());
                // Draw without depth test to ensure the axes do not get lost within a mesh for example
                world->DebugDrawAxes(placeable->WorldTransform(), false);
            }
            break;
        case EC_Light::ComponentTypeId:
            {
                EC_Placeable* placeable = entity->Component<EC_Placeable>().get();
                if (placeable)
                {
                    EC_Light* light = checked_static_cast<EC_Light*>(i->second.get());
                    world->DebugDrawLight(placeable->WorldTransform(), light->type.Get(), light->range.Get(), light->outerAngle.Get(), light->diffColor.Get());
                }
            }
            break;
        case EC_Camera::ComponentTypeId:
            {
                EC_Placeable* placeable = entity->Component<EC_Placeable>().get();
                if (placeable)
                    world->DebugDrawCamera(placeable->WorldTransform(), 1.0f, 1.0f, 1.0f, 1.0f);
            }
            break;
        case EC_Sound::ComponentTypeId:
            {
                EC_Placeable* placeable = entity->Component<EC_Placeable>().get();
                EC_Sound *soundSource = entity->Component<EC_Sound>().get();
                if (placeable && soundSource)
                    world->DebugDrawSoundSource(placeable->WorldPosition(), soundSource->soundInnerRadius.Get(), soundSource->soundOuterRadius.Get(), 1.0f, 1.0f, 1.0f, 1.0f);
            }
            break;
        }
    }
}
