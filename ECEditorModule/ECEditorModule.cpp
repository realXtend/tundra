// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EcXmlEditorWidget.h"
#include "DoxygenDocReader.h"
#include "TreeWidgetItemExpandMemory.h"

#include "SceneAPI.h"
#include "SceneManager.h"
#include "ModuleManager.h"
#include "EC_DynamicComponent.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "ConsoleAPI.h"

#include "MemoryLeakCheck.h"

#include <QWebView>

ECEditorModule::ECEditorModule()
:IModule("ECEditor"),
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
    expandMemory = ExpandMemoryPtr(new TreeWidgetItemExpandMemory(Name().c_str(), framework_));
}

void ECEditorModule::PostInitialize()
{
    framework_->Console()->RegisterCommand("doc", "Prints the class documentation for the given symbol.",
        this, SLOT(ShowDocumentation()));

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
    //ui->AddWidgetToMenu(editor_window_, tr("Entity-component Editor"), "", Application::InstallationDirectory() + "data/ui/images/menus/edbutton_OBJED_normal.png");
    //ui->RegisterUniversalWidget("Components", editor_window_->graphicsProxyWidget());
}

void ECEditorModule::ShowDocumentation(QString symbol)
{
    QUrl styleSheetPath;
    QString documentation;
    /*bool success = */DoxygenDocReader::GetSymbolDocumentation(symbol.toStdString().c_str(), &documentation, &styleSheetPath);
    if (documentation.length() == 0)
    {
        LogError("Failed to find documentation for symbol \"" + symbol + "\"!");
        return;
    }

    QWebView *webview = new QWebView();
    webview->setHtml(documentation, styleSheetPath);
    webview->show();
    webview->setAttribute(Qt::WA_DeleteOnClose);
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
        for(uint i = 0; i < (uint)entities.size(); ++i)
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
