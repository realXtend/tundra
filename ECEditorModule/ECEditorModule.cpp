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
#include "EC_DynamicComponent.h"
#include "UiServiceInterface.h"
#include "UiProxyWidget.h"
#include "Input.h"

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    std::string ECEditorModule::name_static_ = "ECEditor";
    
    ECEditorModule::ECEditorModule() :
        IModule(name_static_),
        scene_event_category_(0),
        network_state_event_category_(0),
        editor_window_(0),
        xmlEditor_(0)
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

        RegisterConsoleCommand(Console::CreateCommand("EditDynComp",
            "Edit dynamic component's attributes."
            "Params:"
            " 0 = entity id."
            " 1 = operation (add or rem)"
            " 2 = component type.(ec. EC_DynamicComponent)"
            " 3 = attribute name."
            " 4 = attribute type. !Only rem dont use in rem operation."
            " 5 = attribute value. !Only rem dont use in rem operation.",
            Console::Bind(this, &ECEditorModule::EditDynamicComponent)));

        scene_event_category_ = event_manager_->QueryEventCategory("Scene");
        network_state_event_category_ = event_manager_->QueryEventCategory("NetworkState");

        AddEditorWindowToUI();

        inputContext = framework_->GetInput()->RegisterInputContext("ECEditorInput", 90);
        //input->SetTakeKeyboardEventsOverQt(true);
        connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));
    }

    void ECEditorModule::Uninitialize()
    {
        event_manager_.reset();
        SAFE_DELETE_LATER(editor_window_);
        SAFE_DELETE_LATER(xmlEditor_);
    }

    void ECEditorModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool ECEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
    {
        if (category_id == scene_event_category_)
        {
            switch(event_id)
            {
            case Scene::Events::EVENT_ENTITY_CLICKED:
            {
                //! \todo support multiple entity selection
                Scene::Events::EntityClickedData *entity_clicked_data = dynamic_cast<Scene::Events::EntityClickedData *>(data);
                if (editor_window_ && entity_clicked_data)
                    editor_window_->AddEntity(entity_clicked_data->entity->GetId());
                break;
            }
            case Scene::Events::EVENT_ENTITY_SELECT:
                //if (editor_window_)
                //    editor_window_->AddEntity(entity_clicked_data->entity->GetId());
                break;
            case Scene::Events::EVENT_ENTITY_DESELECT:
                //if (editor_window_)
                //    editor_window_->RemoveEntity(entity_clicked_data->entity->GetId());
                break;
            case Scene::Events::EVENT_ENTITY_DELETED:
            {
                Scene::Events::SceneEventData *entity_clicked_data = dynamic_cast<Scene::Events::SceneEventData*>(data);
                if (editor_window_ && entity_clicked_data)
                    editor_window_->RemoveEntity(entity_clicked_data->localID);
                break;
            }
            default:
                break;
            }
        }

        if (category_id == network_state_event_category_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
            if (editor_window_)
                editor_window_->ClearEntities(); 

        return false;
    }

    void ECEditorModule::AddEditorWindowToUI()
    {
        if (editor_window_)
            return;

        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (!ui)
            return;

        editor_window_ = new ECEditorWindow(GetFramework());

        UiProxyWidget *editor_proxy = ui->AddWidgetToScene(editor_window_);
        ui->AddWidgetToMenu(editor_window_, tr("Entity-component Editor"), "", "./data/ui/images/menus/edbutton_OBJED_normal.png");
        ui->RegisterUniversalWidget("Components", editor_proxy);

        connect(editor_window_, SIGNAL(EditEntityXml(Scene::EntityPtr)), this, SLOT(CreateXmlEditor(Scene::EntityPtr)));
        connect(editor_window_, SIGNAL(EditComponentXml(ComponentPtr)), this, SLOT(CreateXmlEditor(ComponentPtr)));
        connect(editor_window_, SIGNAL(EditEntityXml(const QList<Scene::EntityPtr> &)), this, SLOT(CreateXmlEditor(const QList<Scene::EntityPtr> &)));
        connect(editor_window_, SIGNAL(EditComponentXml(const QList<ComponentPtr> &)), this, SLOT(CreateXmlEditor(const QList<ComponentPtr> &)));
    }

    Console::CommandResult ECEditorModule::ShowWindow(const StringVector &params)
    {
        UiServicePtr ui = framework_->GetService<UiServiceInterface>(Foundation::Service::ST_Gui).lock();
        if (!ui)
            return Console::ResultFailure("Failed to acquire UiModule pointer!");

        if (editor_window_)
        {
            ui->BringWidgetToFront(editor_window_);
            return Console::ResultSuccess();
        }
        else
            return Console::ResultFailure("EC Editor window was not initialised, something went wrong on startup!");
    }

    /* Params
     * 0 = entity id.
     * 1 = operation (add/rem)
     * 2 = component type.
     * 3 = attribute name
     * 4 = attribute type
     * 5 = attribute value
     */
    Console::CommandResult ECEditorModule::EditDynamicComponent(const StringVector &params)
    {
        Scene::SceneManager *sceneMgr = framework_->GetDefaultWorldScene().get();
        if(!sceneMgr)
            return Console::ResultFailure("Failed to find main scene.");

        if(params.size() == 6)
        {
            entity_id_t id = ParseString<entity_id_t>(params[0]);
            Scene::Entity *ent = sceneMgr->GetEntity(id).get();
            if(!ent)
                return Console::ResultFailure("Cannot find entity by name of " + params[0]);

            if(params[1] == "add")
            {
                ComponentPtr comp = ent->GetComponent(QString::fromStdString(params[2]));
                EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
                if(!dynComp)
                    return Console::ResultFailure("Wrong component type name" + params[2]);
                IAttribute *attribute = dynComp->CreateAttribute(QString::fromStdString(params[4]), params[3].c_str());
                if(!attribute)
                    return Console::ResultFailure("invalid attribute type" + params[4]);
                attribute->FromString(params[5], AttributeChange::Default);
                //dynComp->ComponentChanged("Default");//AttributeChange::Local); 
            }
        }
        if(params.size() == 4)
        {
            entity_id_t id = ParseString<entity_id_t>(params[0]);
            Scene::Entity *ent = sceneMgr->GetEntity(id).get();
            if(!ent)
                return Console::ResultFailure("Cannot find entity by name of" + params[0]);

            else if(params[1] == "rem")
            {
                ComponentPtr comp = ent->GetComponent(QString::fromStdString(params[2]));
                EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
                if(!dynComp)
                    return Console::ResultFailure("Wrong component type name" + params[2]);
                dynComp->RemoveAttribute(QString::fromStdString(params[3]));
                dynComp->ComponentChanged(AttributeChange::Default);
            }
        }
        return Console::ResultSuccess();
    }

    void ECEditorModule::CreateXmlEditor(Scene::EntityPtr entity)
    {
        QList<Scene::EntityPtr> entities;
        entities << entity;
        CreateXmlEditor(entities);
    }

    void ECEditorModule::CreateXmlEditor(const QList<Scene::EntityPtr> &entities)
    {
        UiServicePtr ui = framework_->GetService<UiServiceInterface>(Foundation::Service::ST_Gui).lock();
        if (entities.empty() || !ui)
            return;

        if (!xmlEditor_)
        {
            xmlEditor_ = new EcXmlEditorWidget(framework_);
            ui->AddWidgetToScene(xmlEditor_);
        }

        xmlEditor_->SetEntity(entities);
        ui->BringWidgetToFront(xmlEditor_);
    }

    void ECEditorModule::CreateXmlEditor(ComponentPtr component)
    {
        QList<ComponentPtr> components;
        components << component;
        CreateXmlEditor(components);
    }

    void ECEditorModule::CreateXmlEditor(const QList<ComponentPtr> &components)
    {
        UiServicePtr ui = framework_->GetService<UiServiceInterface>(Foundation::Service::ST_Gui).lock();
        if (components.empty() || !ui)
            return;

        if (!xmlEditor_)
        {
            xmlEditor_ = new EcXmlEditorWidget(framework_);
            ui->AddWidgetToScene(xmlEditor_);
        }

        xmlEditor_->SetComponent(components);
        ui->BringWidgetToFront(xmlEditor_);
    }

    void ECEditorModule::HandleKeyPressed(KeyEvent *e)
    {
        if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
            return;

        Input &input = *framework_->GetInput();

        const QKeySequence showEcEditor = input.KeyBinding("ShowECEditor", QKeySequence(Qt::ShiftModifier + Qt::Key_E));
        if (QKeySequence(e->keyCode | e->modifiers) == showEcEditor)
            ShowWindow(StringVector());
    }

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace ECEditor;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(ECEditorModule)
POCO_END_MANIFEST

