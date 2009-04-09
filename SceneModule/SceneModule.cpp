// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneModule.h"

namespace Scene
{
    SceneModule::SceneModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    SceneModule::~SceneModule()
    {
    }

    // virtual
    void SceneModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    // virtual
    void SceneModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    // virtual
    void SceneModule::Initialize()
    {
        scene_manager_ = Foundation::SceneManagerPtr(new SceneManager(this));
        framework_->GetServiceManager()->RegisterService(Foundation::Service::ST_SceneManager, scene_manager_.get());

        // Register 'Scene' event category and events.
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        scene_event_category_ = event_manager->RegisterEventCategory("Scene");
        event_manager->RegisterEvent(scene_event_category_, EVENT_SCENE_ADDED, "Scene Added");
        event_manager->RegisterEvent(scene_event_category_, EVENT_SCENE_DELETED, "Scene Deleted");
//        event_manager->RegisterEvent(scene_event_category_, EVENT_SCENE_CLONED, "Scene Cloned"); ///\todo
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_ADDED, "Entity Added");
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_UPDATED, "Entity Updated");
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_DELETED, "Entity Deleted");
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_SELECT, "Entity Select");
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_SELECTED, "Entity Selected");
        event_manager->RegisterEvent(scene_event_category_, EVENT_ENTITY_DESELECT, "Entity Deselect");
        
        LogInfo("Module " + Name() + " initialized.");
    }

    // virtual 
    void SceneModule::Uninitialize()
    {
        framework_->GetServiceManager()->UnregisterService(scene_manager_.get());
        scene_manager_.reset();

        LogInfo("Module " + Name() + " uninitialized.");
    }
}

