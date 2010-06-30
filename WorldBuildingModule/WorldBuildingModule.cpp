// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldBuildingModule.h"
#include "EventManager.h"
#include "ServiceManager.h"

#include "NetworkEvents.h"
#include "SceneEvents.h"

#include "BuildSceneManager.h"

#include <QDebug>
#include <QTimer>

namespace WorldBuilding
{
    static std::string module_name = "WorldBuildingModule";
    const std::string &WorldBuildingModule::NameStatic() { return module_name; }

    WorldBuildingModule::WorldBuildingModule() :
        QObject(),
        ModuleInterface(module_name)
    {
        event_query_categories_ << "Framework" << "NetworkState" << "Scene";
    }

    WorldBuildingModule::~WorldBuildingModule()
    {
        build_scene_manager_.reset();
    }

    // Module interface

    void WorldBuildingModule::Initialize()
    {
        // Get all category id's that we are interested in
        SubscribeToEventCategories();
        
        // Init building scene manager and register as service
        build_scene_manager_ = BuildServicePtr(new BuildSceneManager(this, GetFramework()));
        GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_WorldBuilding, build_scene_manager_);

        // Register building key context
        input_context_ = GetFramework()->Input().RegisterInputContext("WorldBuildingContext", 90);
        connect(input_context_.get(), SIGNAL(KeyPressed(KeyEvent&)), SLOT(KeyPressed(KeyEvent&)));
    }

    bool WorldBuildingModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        bool handled = false;
        QString category = service_category_identifiers_.keys().value(
            service_category_identifiers_.values().indexOf(category_id));
        
        if (category == "Framework")
        {
            // Might need world stream later from here...
        }
        else if (category == "Scene")
        {
            switch (event_id)
            {
                case Scene::Events::EVENT_ENTITY_CLICKED:
                {
                    Scene::Events::EntityClickedData *entity_data = checked_static_cast<Scene::Events::EntityClickedData*>(data);
                    if (entity_data)
                        build_scene_manager_->ObjectSelected(entity_data->entity);
                    break;
                }
                default:
                    break;
            }
        }
        else if (category == "NetworkState")
        {
            switch (event_id)
            {
                case ProtocolUtilities::Events::EVENT_SERVER_CONNECTED:
                    build_scene_manager_->inworld_state = true;
                    break;
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                    build_scene_manager_->inworld_state = false;
                    break;
                case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                    build_scene_manager_->inworld_state = false;
                    break;
                case ProtocolUtilities::Events::EVENT_CAPS_FETCHED:
                    break;
                default:
                    break;
            }
        }

        return handled;
    }

    // WorldBuildingModule

    void WorldBuildingModule::KeyPressed(KeyEvent &key)
    {
        // Ctrl + B to toggle build scene
        if (!key.IsRepeat() && key.modifiers == Qt::ControlModifier && key.keyCode == Qt::Key_B)
            build_scene_manager_->ToggleBuildScene();
    }

    QObject *WorldBuildingModule::GetPythonHandler()
    {
        if (build_scene_manager_)
            return build_scene_manager_->GetPythonHandler();
        return 0;
    }
    void WorldBuildingModule::SubscribeToEventCategories()
    {
        service_category_identifiers_.clear();
        foreach (QString category, event_query_categories_)
            service_category_identifiers_[category] = GetFramework()->GetEventManager()->QueryEventCategory(category.toStdString());
    }
}

/************** Poco Module Loading System **************/

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace WorldBuilding;
POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(WorldBuildingModule)
POCO_END_MANIFEST
