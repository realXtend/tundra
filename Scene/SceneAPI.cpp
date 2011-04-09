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
    framework_(framework),
    sceneCategoryName_("Scene")
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
    event_category_id_t scene_event_category = event_manager->RegisterEventCategory(sceneCategoryName_);

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

SceneInteractWeakPtr SceneAPI::GetSceneInteract() const
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

Scene::ScenePtr SceneAPI::CreateScene(const QString &name, bool viewenabled)
{
    if (HasScene(name))
        return Scene::ScenePtr();

    Scene::ScenePtr newScene = Scene::ScenePtr(new Scene::SceneManager(name, framework_, viewenabled));
    if (newScene.get())
    {
        scenes_[name] = newScene;

        // Send internal event of creation
        Scene::Events::SceneEventData eventData(newScene->Name().toStdString());
        event_category_id_t categoryId = framework_->GetEventManager()->QueryEventCategory(sceneCategoryName_);
        framework_->GetEventManager()->SendEvent(categoryId, Scene::Events::EVENT_SCENE_ADDED, &eventData);

        // Emit signal of creation
        emit SceneAdded(newScene->Name());
    }
    return newScene;
}

void SceneAPI::RemoveScene(const QString &name)
{
    SceneMap::iterator sceneIter = scenes_.find(name);
    if (sceneIter != scenes_.end())
    {
        // If default scene is being removed. Reset our ref so it does not keep ref count alive.
        if (defaultScene_ == sceneIter->second)
            defaultScene_.reset();
        scenes_.erase(sceneIter);

        // Send internal event about removed scene
        Scene::Events::SceneEventData eventData(name.toStdString());
        event_category_id_t categoryId = framework_->GetEventManager()->QueryEventCategory(sceneCategoryName_);
        framework_->GetEventManager()->SendEvent(categoryId, Scene::Events::EVENT_SCENE_DELETED, &eventData);

        // Emit signal about removed scene
        emit SceneRemoved(name);
    }
}

const SceneMap &SceneAPI::GetSceneMap() const
{
    return scenes_;
}
