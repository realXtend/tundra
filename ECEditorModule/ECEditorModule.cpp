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
xmlEditor(0)
{
}

ECEditorModule::~ECEditorModule()
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
        this, SLOT(ShowDocumentation(const QString &)));

    inputContext = framework_->Input()->RegisterInputContext("ECEditorInput", 90);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));
}

void ECEditorModule::Uninitialize()
{
    if (commonEditor)
        SAFE_DELETE(commonEditor);
    SAFE_DELETE_LATER(xmlEditor);
}

void ECEditorModule::Update(f64 frametime)
{
}

ECEditorWindow *ECEditorModule::ActiveEditor() const
{
    return activeEditor;
}

void ECEditorModule::ECEditorFocusChanged(ECEditorWindow *editor)
{
    if (editor == activeEditor && !editor)
        return;

    // Unfocus previously active editor and disconnect all signals from that editor.
    if (activeEditor)
    {
        activeEditor->SetFocus(false);
        disconnect(activeEditor, SIGNAL(destroyed(QObject*)), this, SLOT(ActiveECEditorDestroyed(QObject*)));
        disconnect(activeEditor, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)),
                   this, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)));
    }

    activeEditor = editor;
    activeEditor->SetFocus(true);
    connect(activeEditor, SIGNAL(destroyed(QObject*)), SLOT(ActiveECEditorDestroyed(QObject*)), Qt::UniqueConnection);
    connect(activeEditor, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)),
            this, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)), Qt::UniqueConnection);
}

void ECEditorModule::ShowEditorWindow()
{
    if (framework_->IsHeadless())
        return;

    if (activeEditor)
    {
        activeEditor->setVisible(!activeEditor->isVisible());
        return;
    }

    activeEditor = new ECEditorWindow(GetFramework());
    commonEditor = activeEditor;
    activeEditor->setParent(framework_->Ui()->MainWindow());
    activeEditor->setWindowFlags(Qt::Tool);
    activeEditor->setAttribute(Qt::WA_DeleteOnClose);
    activeEditor->show();
}

void ECEditorModule::ShowDocumentation(const QString &symbol)
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
    if (activeEditor)
        return activeEditor->GetSelectedComponents();
    return QObjectList();
}

QVariantList ECEditorModule::GetSelectedEntities() const
{
    if (activeEditor)
    {
        QList<EntityPtr> entities = activeEditor->GetSelectedEntities();
        QVariantList retEntities;
        for(uint i = 0; i < (uint)entities.size(); ++i)
            retEntities.push_back(QVariant(entities[i]->GetId()));
        return retEntities;
    }
    return QVariantList();
}

void ECEditorModule::CreateXmlEditor(const QList<EntityPtr> &entities)
{
    if (framework_->IsHeadless())
        return;
    if (entities.empty())
        return;

    if (!xmlEditor)
    {
        xmlEditor = new EcXmlEditorWidget(framework_);
        xmlEditor->setParent(GetFramework()->Ui()->MainWindow());
        xmlEditor->setWindowFlags(Qt::Tool);
    }

    xmlEditor->SetEntity(entities);
}

void ECEditorModule::CreateXmlEditor(ComponentPtr component)
{
    QList<ComponentPtr> components;
    components << component;
    CreateXmlEditor(components);
}

void ECEditorModule::CreateXmlEditor(const QList<ComponentPtr> &components)
{
    if (framework_->IsHeadless())
        return;
    if (!components.empty())
        return;

    if (!xmlEditor)
    {
        xmlEditor = new EcXmlEditorWidget(framework_);
        xmlEditor->setParent(GetFramework()->Ui()->MainWindow());
        xmlEditor->setWindowFlags(Qt::Tool);
    }

    xmlEditor->SetComponent(components);
}

void ECEditorModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    const QKeySequence showEcEditor = framework_->Input()->KeyBinding("ShowECEditor", QKeySequence(Qt::ShiftModifier + Qt::Key_E));
    if (QKeySequence(e->keyCode | e->modifiers) == showEcEditor)
    {
        ShowEditorWindow();
        e->handled = true;
    }
}

void ECEditorModule::ActiveECEditorDestroyed(QObject *obj)
{
    if (activeEditor == obj)
        activeEditor = 0;
}
