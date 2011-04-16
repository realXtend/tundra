// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EcXmlEditorWidget.h"
#include "DoxygenDocReader.h"
#include "TreeWidgetItemExpandMemory.h"

#include "EventManager.h"
#include "SceneAPI.h"
#include "SceneManager.h"
#include "ConsoleCommandUtils.h"
#include "ModuleManager.h"
#include "EC_DynamicComponent.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "ConsoleAPI.h"

#include "MemoryLeakCheck.h"

#include <QWebView>

std::string ECEditorModule::name_static_ = "ECEditor";

ECEditorModule::ECEditorModule() :
    IModule(name_static_),
    xmlEditor_(0)
{
}

ECEditorModule::~ECEditorModule()
{
}

void ECEditorModule::Load()
{
}

void ECEditorModule::Initialize()
{
    GetFramework()->RegisterDynamicObject("eceditor", this);
    expandMemory = ExpandMemoryPtr(new TreeWidgetItemExpandMemory(name_static_.c_str(), framework_));
}

void ECEditorModule::PostInitialize()
{
    /*framework_->Console()->RegisterCommand(CreateConsoleCommand("ECEditor",
        "Shows the EC editor.",
        ConsoleBind(this, &ECEditorModule::ShowWindow)));*/

    framework_->Console()->RegisterCommand(CreateConsoleCommand("EditDynComp",
        "Command that will create/remove components from the dynamic component."
        "Params:"
        " 0 = entity id."
        " 1 = operation (add or rem)"
        " 2 = component type.(ec. EC_DynamicComponent)"
        " 3 = attribute name."
        " 4 = attribute type. (Add only)"
        " 5 = attribute value. (Add only)",
        ConsoleBind(this, &ECEditorModule::EditDynamicComponent)));

    framework_->Console()->RegisterCommand(CreateConsoleCommand("ShowDocumentation",
        "Prints the class documentation for the given symbol."
        "Params:"
        " 0 = The symbol to fetch the documentation for.",
        ConsoleBind(this, &ECEditorModule::ShowDocumentation)));

    AddEditorWindowToUI();

    inputContext = framework_->Input()->RegisterInputContext("ECEditorInput", 90);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));
}

void ECEditorModule::Uninitialize()
{
    if (common_editor_)
        SAFE_DELETE(common_editor_);
    SAFE_DELETE_LATER(xmlEditor_);
}

void ECEditorModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool ECEditorModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    /* Regression. Removed the dependency from ECEditorModule to ProtocolUtilities. Reimplement this by deleting
       all ECEditors when we disconnect. -jj.
    if (category_id == network_state_event_category_ && event_id == ProtocolUtilities::Events::EVENT_SERVER_DISCONNECTED)
        if (active_editor_)
            active_editor_->ClearEntities(); 
    */
    return false;
}

ECEditorWindow *ECEditorModule::GetActiveECEditor() const
{
    return active_editor_;
}

void ECEditorModule::ECEditorFocusChanged(ECEditorWindow *editor)
{
    if (editor == active_editor_ && !editor)
        return;

    // Unfocus previously active editor and disconnect all signals from that editor.
    if (active_editor_)
    {
        active_editor_->SetFocus(false);
        disconnect(active_editor_, SIGNAL(destroyed(QObject*)), this, SLOT(ActiveECEditorDestroyed(QObject*)));
        disconnect(active_editor_, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)),
                   this, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)));
    }
    active_editor_ = editor;
    active_editor_->SetFocus(true);
    connect(active_editor_, SIGNAL(destroyed(QObject*)), SLOT(ActiveECEditorDestroyed(QObject*)), Qt::UniqueConnection);
    connect(active_editor_, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)),
            this, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)), Qt::UniqueConnection);
}

void ECEditorModule::AddEditorWindowToUI()
{
    if (active_editor_)
    {
        active_editor_->setVisible(!active_editor_->isVisible());
        return;
    }

    UiAPI *ui = GetFramework()->Ui();
    if (!ui)
        return;

    active_editor_ = new ECEditorWindow(GetFramework());
    common_editor_ = active_editor_;
    active_editor_->setParent(ui->MainWindow());
    active_editor_->setWindowFlags(Qt::Tool);
    active_editor_->setAttribute(Qt::WA_DeleteOnClose);

    //UiProxyWidget *editor_proxy = ui->AddWidgetToScene(editor_window_);
    // We need to listen proxy widget's focus signal, because for some reason QWidget's focusInEvent wont get triggered when
    // it's attached to QGraphicsProxyWidget.
    //connect(editor_proxy, SIGNAL(FocusChanged(QFocusEvent *)), editor_window_, SLOT(FocusChanged(QFocusEvent *)), Qt::UniqueConnection);

    // We don't need to worry about attaching ECEditorWindow to ui scene, because ECEditorWindow's initialize operation will do it automaticly.
    //ui->AddWidgetToMenu(editor_window_, tr("Entity-component Editor"), "", "./data/ui/images/menus/edbutton_OBJED_normal.png");
    //ui->RegisterUniversalWidget("Components", editor_window_->graphicsProxyWidget());
}

/*ConsoleCommandResult ECEditorModule::ShowWindow(const StringVector &params)
{
    UiServicePtr ui = framework_->GetService<UiServiceInterface>(Service::ST_Gui).lock();
    if (!ui)
        return ConsoleResultFailure("Failed to acquire UiModule pointer!");

    if (editor_window_)
    {
        ui->BringWidgetToFront(editor_window_);
        return ConsoleResultSuccess();
    }
    else
        return ConsoleResultFailure("EC Editor window was not initialised, something went wrong on startup!");
}*/

ConsoleCommandResult ECEditorModule::ShowDocumentation(const StringVector &params)
{
    if (params.size() == 0)
        return ConsoleResultFailure("The first parameter must be the documentation symbol to find!");

    QUrl styleSheetPath;
    QString documentation;
    /*bool success = */DoxygenDocReader::GetSymbolDocumentation(params[0].c_str(), &documentation, &styleSheetPath);
    if (documentation.length() == 0)
        return ConsoleResultFailure("Failed to find documentation!");

    QWebView *webview = new QWebView();
    webview->setHtml(documentation, styleSheetPath);
    webview->show();
    webview->setAttribute(Qt::WA_DeleteOnClose);

    return ConsoleResultSuccess();
}

/* Params
 * 0 = entity id.
 * 1 = operation (add/rem)
 * 2 = component type.
 * 3 = attribute name
 * 4 = attribute type
 * 5 = attribute value
 */
ConsoleCommandResult ECEditorModule::EditDynamicComponent(const StringVector &params)
{
    Scene::SceneManager *sceneMgr = GetFramework()->Scene()->GetDefaultScene().get();
    if(!sceneMgr)
        return ConsoleResultFailure("Failed to find main scene.");

    if(params.size() == 6)
    {
        entity_id_t id = ParseString<entity_id_t>(params[0]);
        Scene::Entity *ent = sceneMgr->GetEntity(id).get();
        if(!ent)
            return ConsoleResultFailure("Cannot find entity by name of " + params[0]);

        if(params[1] == "add")
        {
            ComponentPtr comp = ent->GetComponent(QString::fromStdString(params[2]));
            EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
            if(!dynComp)
                return ConsoleResultFailure("Invalid component type " + params[2]);
            IAttribute *attribute = dynComp->CreateAttribute(QString::fromStdString(params[4]), params[3].c_str());
            if(!attribute)
                return ConsoleResultFailure("Failed to create attribute type " + params[4]);
            attribute->FromString(params[5], AttributeChange::Default);
            //dynComp->ComponentChanged("Default");//AttributeChange::Local); 
        }
    }
    if(params.size() == 4)
    {
        entity_id_t id = ParseString<entity_id_t>(params[0]);
        Scene::Entity *ent = sceneMgr->GetEntity(id).get();
        if(!ent)
            return ConsoleResultFailure("Cannot find entity by name of " + params[0]);

        else if(params[1] == "rem")
        {
            ComponentPtr comp = ent->GetComponent(QString::fromStdString(params[2]));
            EC_DynamicComponent *dynComp = dynamic_cast<EC_DynamicComponent *>(comp.get());
            if(!dynComp)
                return ConsoleResultFailure("Wrong component typename " + params[2]);
            dynComp->RemoveAttribute(QString::fromStdString(params[3]));
            dynComp->ComponentChanged(AttributeChange::Default);
        }
    }
    return ConsoleResultSuccess();
}

void ECEditorModule::CreateXmlEditor(EntityPtr entity)
{
    QList<EntityPtr> entities;
    entities << entity;
    CreateXmlEditor(entities);
}

QObjectList ECEditorModule::GetSelectedComponents() const
{
    if (active_editor_)
        return active_editor_->GetSelectedComponents();
    return QObjectList();
}

QVariantList ECEditorModule::GetSelectedEntities() const
{
    if (active_editor_)
    {
        QList<EntityPtr> entities = active_editor_->GetSelectedEntities();
        QVariantList retEntities;
        for(uint i = 0; i < entities.size(); ++i)
            retEntities.push_back(QVariant(entities[i]->GetId()));
        return retEntities;
    }
    return QVariantList();
}

void ECEditorModule::CreateXmlEditor(const QList<EntityPtr> &entities)
{
    UiAPI *ui = GetFramework()->Ui();
    if (entities.empty())
        return;

    if (!xmlEditor_)
    {
        xmlEditor_ = new EcXmlEditorWidget(framework_);
        xmlEditor_->setParent(ui->MainWindow());
        xmlEditor_->setWindowFlags(Qt::Tool);
        //ui->AddWidgetToScene(xmlEditor_);
    }

    xmlEditor_->SetEntity(entities);
    //ui->BringWidgetToFront(xmlEditor_);
}

void ECEditorModule::CreateXmlEditor(ComponentPtr component)
{
    QList<ComponentPtr> components;
    components << component;
    CreateXmlEditor(components);
}

void ECEditorModule::CreateXmlEditor(const QList<ComponentPtr> &components)
{
    UiAPI *ui = GetFramework()->Ui();
    if (!components.empty())
        return;

    if (!xmlEditor_)
    {
        xmlEditor_ = new EcXmlEditorWidget(framework_);
        xmlEditor_->setParent(ui->MainWindow());
        xmlEditor_->setWindowFlags(Qt::Tool);
        //ui->AddWidgetToScene(xmlEditor_);
    }

    xmlEditor_->SetComponent(components);
    //ui->BringWidgetToFront(xmlEditor_);
}

void ECEditorModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    const QKeySequence showEcEditor = framework_->Input()->KeyBinding("ShowECEditor", QKeySequence(Qt::ShiftModifier + Qt::Key_E));
    if (QKeySequence(e->keyCode | e->modifiers) == showEcEditor)
    {
        if (!active_editor_)
        {
            AddEditorWindowToUI();
            active_editor_->show();
        }
        else
            active_editor_->setVisible(!active_editor_->isVisible());
        e->handled = true;
    }
}

void ECEditorModule::ActiveECEditorDestroyed(QObject *obj)
{
    if (active_editor_ == obj)
        active_editor_ = 0;
}

bool ECEditorModule::IsECEditorWindowVisible() const
{
    if (active_editor_)
    {
        return active_editor_->isVisible();
    }
    else
    {
        return false;
    }
}

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new ECEditorModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
