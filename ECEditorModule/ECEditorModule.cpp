// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EventManager.h"
#include "SceneEvents.h"
#include "NetworkEvents.h"
#include "SceneManager.h"
#include "ConsoleCommandServiceInterface.h"

#include "EC_SerializationTest.h"

namespace ECEditor
{
    std::string ECEditorModule::name_static_ = "ECEditorModule";
    
    ECEditorModule::ECEditorModule() :
        ModuleInterfaceImpl(name_static_),
        editor_window_(0),
        scene_event_category_(0),
        framework_event_category_(0),
        input_event_category_(0),
        network_state_event_category_(0)
    {
    }
    
    ECEditorModule::~ECEditorModule()
    {
    }
    
    void ECEditorModule::Load()
    {
        DECLARE_MODULE_EC(EC_SerializationTest);
    }

    void ECEditorModule::Initialize()
    {
        editor_window_ = new ECEditorWindow(GetFramework());
        event_manager_ = framework_->GetEventManager();
    }

    void ECEditorModule::PostInitialize()
    {
        RegisterConsoleCommand(Console::CreateCommand("ECEditor", 
            "Shows the EC editor.",
            Console::Bind(this, &ECEditorModule::ShowWindow)));
            
        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        framework_event_category_ = event_manager_->QueryEventCategory("Framework");
        input_event_category_ = event_manager_->QueryEventCategory("Input");
    }

    void ECEditorModule::SubscribeToNetworkEvents()
    {
        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");
    }

    void ECEditorModule::Uninitialize()
    {
        if (editor_window_)
        {
            editor_window_->deleteLater();
            editor_window_ = 0;
        }
    }

    void ECEditorModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    Console::CommandResult ECEditorModule::ShowWindow(const StringVector &params)
    {
        if (editor_window_)
            editor_window_->BringToFront();
        
        return Console::ResultSuccess();
    }
    
    bool ECEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (category_id == framework_event_category_ && event_id == Foundation::NETWORKING_REGISTERED)
        {
            SubscribeToNetworkEvents();
        }
        
        if (category_id == scene_event_category_ && event_id == Scene::Events::EVENT_ENTITY_CLICKED)
        {
            //! \todo support multiple entity selection
            Scene::Events::EntityClickedData *entity_clicked_data = dynamic_cast<Scene::Events::EntityClickedData *>(data);
            if (editor_window_)
                editor_window_->AddEntity(entity_clicked_data->entity->GetId());
        }
        
        if (category_id == network_state_event_category_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        {
            if (editor_window_)
                editor_window_->ClearEntities();
        }
        return false;
    }

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace ECEditor;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
    POCO_EXPORT_CLASS(ECEditorModule)
POCO_END_MANIFEST 

