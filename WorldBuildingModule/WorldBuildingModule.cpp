// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldBuildingModule.h"
#include "EventManager.h"
#include "ServiceManager.h"

#include "NetworkEvents.h"
#include "SceneEvents.h"

#include "BuildSceneManager.h"
#include "OpenSimScene/OpenSimSceneService.h"

#include <QDebug>
#include <QTimer>

namespace WorldBuilding
{
    static std::string module_name = "WorldBuildingModule";
    const std::string &WorldBuildingModule::NameStatic() { return module_name; }

    WorldBuildingModule::WorldBuildingModule() :
        QObject(),
        IModule(module_name)
    {
        event_query_categories_ << "Framework" << "NetworkState" << "Scene";
    }

    WorldBuildingModule::~WorldBuildingModule()
    {
        build_scene_manager_.reset();
        opensim_scene_service_.reset();
    }

    // Module interface

    void WorldBuildingModule::Initialize()
    {        
        // Init and register services
        build_scene_manager_ = BuildServicePtr(new BuildSceneManager(this, GetFramework()));
        GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_WorldBuilding, build_scene_manager_);

        opensim_scene_service_ = OpenSimSceneServicePtr(new OpenSimSceneService(this, GetFramework()));
        GetFramework()->GetServiceManager()->RegisterService(Foundation::Service::ST_OpenSimScene, opensim_scene_service_);
    }

    void WorldBuildingModule::PostInitialize()
    {
        // Get all category id's that we are interested in
        SubscribeToEventCategories();
        opensim_scene_service_->PostInitialize();

        // Register building key context
        input_context_ = GetFramework()->GetInput()->RegisterInputContext("WorldBuildingContext", 90);
        connect(input_context_.get(), SIGNAL(KeyPressed(KeyEvent*)), build_scene_manager_.get(), SLOT(KeyPressed(KeyEvent*)));
        connect(input_context_.get(), SIGNAL(KeyReleased(KeyEvent*)), build_scene_manager_.get(), SLOT(KeyReleased(KeyEvent*)));
        connect(input_context_.get(), SIGNAL(MouseLeftPressed(MouseEvent*)), opensim_scene_service_.get(), SLOT(MouseLeftPressed(MouseEvent*)));
    }

    bool WorldBuildingModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        bool handled = false;
        QString category = service_category_identifiers_.keys().value(
            service_category_identifiers_.values().indexOf(category_id));
        
        if (category == "Framework")
        {
            if (event_id == Foundation::WORLD_STREAM_READY)
            {
                ProtocolUtilities::WorldStreamReadyEvent *event_data = checked_static_cast<ProtocolUtilities::WorldStreamReadyEvent *>(data);
                if (event_data)
                    opensim_scene_service_->SetWorldStream(event_data->WorldStream);
            }
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
                case Scene::Events::EVENT_ENTITY_NONE_CLICKED:
                {
                    build_scene_manager_->ObjectDeselected();
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
                    build_scene_manager_->CreateCamera();
                    break;
                case ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED:
                    build_scene_manager_->inworld_state = false;
                    build_scene_manager_->ResetCamera();
                    build_scene_manager_->ResetEditing();
                    opensim_scene_service_->ResetWorldStream();
                    break;
                case ProtocolUtilities::Events::EVENT_CONNECTION_FAILED:
                    build_scene_manager_->inworld_state = false;
                    build_scene_manager_->ResetCamera();
                    build_scene_manager_->ResetEditing();
                    break;
                case ProtocolUtilities::Events::EVENT_CAPS_FETCHED:
                    opensim_scene_service_->CheckForCapability();
                    break;
                default:
                    break;
            }
        }

        return handled;
    }

    // WorldBuildingModule

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
POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(WorldBuildingModule)
POCO_END_MANIFEST
