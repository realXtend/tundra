// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneAPI.h"
#include "SceneEvents.h"
#include "SceneManager.h"

#include "EventManager.h"

#include "MemoryLeakCheck.h"

SceneAPI::SceneAPI(Foundation::Framework *framework) :
    QObject(framework),
    framework_(framework)
{
    sceneInteract_ = QSharedPointer<SceneInteract>(new SceneInteract());
    framework->RegisterDynamicObject("sceneinteract", sceneInteract_.data());

    defaultScene_.reset();
    scenes_.clear();
}

SceneAPI::~SceneAPI()
{
}

void SceneAPI::Reset()
{
    sceneInteract_.clear();
    defaultScene_.reset();
    scenes_.clear();
}

void SceneAPI::RegisterSceneEvents() const
{
    ///\todo Get rid of all the legacy scene events
    const EventManagerPtr &event_manager = framework_->GetEventManager();
    event_category_id_t scene_event_category = event_manager->RegisterEventCategory("Scene");

    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_SCENE_ADDED, "Scene Added");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_SCENE_DELETED, "Scene Deleted");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_SCENE_CLONED, "Scene Cloned");

    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_ADDED, "Entity Added");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_UPDATED, "Entity Updated");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_DELETED, "Entity Deleted");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_SELECT, "Entity Select");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_SELECTED, "Entity Selected");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_DESELECT, "Entity Deselect");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_CLICKED, "Entity Clicked");

    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_CONTROLLABLE_ENTITY, "Controllable Entity Created");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_VISUALS_MODIFIED, "Entity Visual Appearance Modified");
    event_manager->RegisterEvent(scene_event_category, Scene::Events::EVENT_ENTITY_MEDIAURL_SET, "Mediaurl set");
}

void SceneAPI::Initialise()
{
    sceneInteract_->Initialize(framework_);
}

void SceneAPI::PostInitialize()
{
    sceneInteract_->PostInitialize();
}

SceneInteractWeakPtr SceneAPI::GetSceneIteract() const
{
    return SceneInteractWeakPtr(sceneInteract_);
}

bool SceneAPI::HasScene(const QString &name) const
{
    return scenes_.find(name) != scenes_.end();
}

void SceneAPI::SetDefaultScene(const QString &name)
{
    Scene::ScenePtr scene = GetScene(name);
    if(scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

void SceneAPI::SetDefaultScene(const Scene::ScenePtr &scene)
{
    if (scene != defaultScene_)
    {
        defaultScene_ = scene;
        emit DefaultWorldSceneChanged(defaultScene_.get());
    }
}

const Scene::ScenePtr &SceneAPI::GetDefaultScene() const
{
    return defaultScene_;
}

Scene::SceneManager* SceneAPI::GetDefaultSceneRaw() const
{
    return defaultScene_.get();
}

Scene::ScenePtr SceneAPI::GetScene(const QString &name) const
{
    SceneMap::const_iterator scene = scenes_.find(name);
    if (scene != scenes_.end())
        return scene->second;
    return Scene::ScenePtr();  
}

Scene::SceneManager* SceneAPI::GetSceneRaw(const QString& name) const
{
    return GetScene(name).get();
}

Scene::ScenePtr SceneAPI::CreateScene(const QString &name, bool viewenabled)
{
    if (HasScene(name))
        return Scene::ScenePtr();

    Scene::ScenePtr new_scene = Scene::ScenePtr(new Scene::SceneManager(name, framework_, viewenabled));
    scenes_[name] = new_scene;

    Scene::Events::SceneEventData event_data(name.toStdString());
    event_category_id_t cat_id = framework_->GetEventManager()->QueryEventCategory("Scene");
    framework_->GetEventManager()->SendEvent(cat_id, Scene::Events::EVENT_SCENE_ADDED, &event_data);

    emit SceneAdded(new_scene->Name());
    return new_scene;
}

void SceneAPI::RemoveScene(const QString &name)
{
    SceneMap::iterator scene = scenes_.find(name);
    if (scene != scenes_.end())
    {
        if (defaultScene_ == scene->second)
            defaultScene_.reset();
        scenes_.erase(scene);
        emit SceneRemoved(defaultScene_->Name());
    }
}

const SceneMap &SceneAPI::GetSceneMap() const
{
    return scenes_;
}
