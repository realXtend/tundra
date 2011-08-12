// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EcXmlEditorWidget.h"
#include "DoxygenDocReader.h"
#include "TreeWidgetItemExpandMemory.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"

#include "EC_Placeable.h"

#include "MemoryLeakCheck.h"

#include <QWebView>
#include <QList>

const QString cShowAidsSetting("show visual editing aids");

ECEditorModule::ECEditorModule() : 
    IModule("ECEditor"),
    showVisualAids(false),
    toggleSelectAllEntities(false)
{
}

ECEditorModule::~ECEditorModule()
{
}

void ECEditorModule::Initialize()
{
    GetFramework()->RegisterDynamicObject("eceditor", this);
    expandMemory = ExpandMemoryPtr(new TreeWidgetItemExpandMemory(Name().c_str(), framework_));

    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, Name().c_str());
    if (!framework_->Config()->HasValue(configData, cShowAidsSetting))
        framework_->Config()->Set(configData, cShowAidsSetting, true);
    showVisualAids = framework_->Config()->Get(configData, cShowAidsSetting, QVariant(showVisualAids)).toBool();
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
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, Name().c_str());
    framework_->Config()->Set(configData, cShowAidsSetting, showVisualAids);

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

void ECEditorModule::ShowVisualEditingAids(bool show)
{
    if (framework_->IsHeadless())
        return;

    if (show != showVisualAids)
    {
        showVisualAids = show;
        foreach(ECEditorWindow *editor, framework_->Ui()->MainWindow()->findChildren<ECEditorWindow *>())
            if (showVisualAids && editor == activeEditor) // if showVisualAids == true, show visual aids only for active editor.
                editor->ShowVisualEditingAids(showVisualAids);
            else
                editor->ShowVisualEditingAids(false);
    }
}

void ECEditorModule::ECEditorFocusChanged(ECEditorWindow *editor)
{
    if (editor == activeEditor && !editor)
        return;

    // Unfocus previously active editor and disconnect all signals from that editor.
    if (activeEditor)
    {
        activeEditor->SetFocus(false);
        disconnect(activeEditor, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)),
            this, SIGNAL(SelectionChanged(const QString&, const QString&, const QString&, const QString&)));
    }

    activeEditor = editor;
    activeEditor->SetFocus(true);
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

    activeEditor = new ECEditorWindow(GetFramework(), framework_->Ui()->MainWindow());
    commonEditor = activeEditor;
    activeEditor->setWindowFlags(Qt::Tool);
    activeEditor->setAttribute(Qt::WA_DeleteOnClose);
    activeEditor->show();
    activeEditor->activateWindow();
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
    webview->setAttribute(Qt::WA_DeleteOnClose);
    webview->setHtml(documentation, styleSheetPath);
    webview->show();
}

void ECEditorModule::CreateXmlEditor(EntityPtr entity)
{
    CreateXmlEditor(QList<EntityPtr>(QList<EntityPtr>() << entity));
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
            retEntities.push_back(QVariant(entities[i]->Id()));
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
    if (components.empty())
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
    const QKeySequence &toggle= framework_->Input()->KeyBinding("ToggleVisualEditingAids", QKeySequence(Qt::Key_section));
    const QKeySequence toggleSelectAll = framework_->Input()->KeyBinding("ToggleSelectAllEntities", QKeySequence(Qt::ControlModifier + Qt::Key_A));
    if (e->sequence == showEcEditor)
    {
        ShowEditorWindow();
        e->Suppress();
    }
    else if (e->sequence == toggle)
    {
        ShowVisualEditingAids(!showVisualAids);
        e->Suppress();
    }
    else if (e->sequence == toggleSelectAll)
    {
        // Only if visual editing is enabled
        if (showVisualAids && !activeEditor.isNull())
        {
            // Select/deselect all entities for the current editor
            toggleSelectAllEntities = !toggleSelectAllEntities;
            if (toggleSelectAllEntities)
            {
                ScenePtr activeScene = framework_->Scene()->GetDefaultScene();
                if (activeScene.get())
                {
                    // We can only manipulate entities that have placeable, but exclude temporarys (avatar, cameras etc.)
                    QList<entity_id_t> entIdsSelection;
                    QVariantList entIds = activeScene->GetEntityIdsWithComponent(EC_Placeable::TypeNameStatic());
                    foreach(QVariant entId, entIds)
                    {
                        Entity *ent = activeScene->GetEntity(entId.toUInt()).get();
                        if (ent)
                        {
                            if (ent->IsTemporary())
                                continue;
                            entIdsSelection.append(entId.toUInt());
                        }
                    }
                    if (!entIdsSelection.isEmpty())
                        activeEditor->AddEntities(entIdsSelection, true);
                }
            }
            else
                activeEditor->ClearEntities();
        }
        e->Suppress();
    }
}
