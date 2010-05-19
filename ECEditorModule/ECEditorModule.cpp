// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EC_SerializationTest.h"
#include "EcXmlEditorWidget.h"

#include "EventManager.h"
#include "SceneEvents.h"
#include "NetworkEvents.h"
#include "SceneManager.h"
#include "ConsoleCommandServiceInterface.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/InworldSceneController.h"

#include "MemoryLeakCheck.h"

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

    void ECEditorModule::Uninitialize()
    {
        event_manager_.reset();
        SAFE_DELETE_LATER(editor_window_);
    }

    void ECEditorModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool ECEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (category_id == framework_event_category_ && event_id == Foundation::NETWORKING_REGISTERED)
            network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");

        if (category_id == scene_event_category_)
        {
            switch(event_id)
            {
            case Scene::Events::EVENT_ENTITY_CLICKED:
            {
                //! \todo support multiple entity selection
                Scene::Events::EntityClickedData *entity_clicked_data = dynamic_cast<Scene::Events::EntityClickedData *>(data);
                if (editor_window_)
                    editor_window_->AddEntity(entity_clicked_data->entity->GetId());
            }
            case Scene::Events::EVENT_ENTITY_SELECT:
                //if (editor_window_)
                //    editor_window_->AddEntity(entity_clicked_data->entity->GetId());
                break;
            case Scene::Events::EVENT_ENTITY_DESELECT:
                //if (editor_window_)
                //    editor_window_->RemoveEntity(entity_clicked_data->entity->GetId());
                break;
            default:
                break;
            }
        }

        if (category_id == network_state_event_category_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            if (editor_window_)
                editor_window_->ClearEntities();

        return false;
    }

    Console::CommandResult ECEditorModule::ShowWindow(const StringVector &params)
    {
        UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!ui_module)
            return Console::ResultFailure("Failed to acquire UiModule pointer!");

        if (editor_window_)
        {
            ui_module->GetInworldSceneController()->BringProxyToFront(editor_window_);
            return Console::ResultSuccess();
        }

        editor_window_ = new ECEditorWindow(GetFramework());
        UiServices::UiWidgetProperties widget_properties(editor_window_->windowTitle(), UiServices::SceneWidget);
        ui_module->GetInworldSceneController()->AddWidgetToScene(editor_window_, widget_properties);
        ui_module->GetInworldSceneController()->ShowProxyForWidget(editor_window_);

        connect(editor_window_, SIGNAL(EditEntityXml(Scene::EntityPtr)), this, SLOT(CreateXmlEditor(Scene::EntityPtr)));
        connect(editor_window_, SIGNAL(EditComponentXml(Foundation::ComponentPtr)), this, SLOT(CreateXmlEditor(Foundation::ComponentPtr)));

        return Console::ResultSuccess();
    }

    void ECEditorModule::CreateXmlEditor(Scene::EntityPtr entity)
    {
        UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!entity || !ui_module)
            return;

        EcXmlEditorWidget *editor = new EcXmlEditorWidget(framework_);
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(editor,
            UiServices::UiWidgetProperties(editor->windowTitle(), UiServices::SceneWidget));
        connect(proxy, SIGNAL(Closed()), editor, SLOT(deleteLater()));

        editor->SetEntity(entity);
        ui_module->GetInworldSceneController()->BringProxyToFront(editor);
    }

    void ECEditorModule::CreateXmlEditor(Foundation::ComponentPtr component)
    {
        UiModulePtr ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
        if (!component || !ui_module)
            return;

        EcXmlEditorWidget *editor = new EcXmlEditorWidget(framework_);
        UiServices::UiProxyWidget *proxy = ui_module->GetInworldSceneController()->AddWidgetToScene(editor,
            UiServices::UiWidgetProperties(editor->windowTitle(), UiServices::SceneWidget));
        connect(proxy, SIGNAL(Closed()), editor, SLOT(deleteLater()));

        editor->SetComponent(component);
        ui_module->GetInworldSceneController()->BringProxyToFront(editor);
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

