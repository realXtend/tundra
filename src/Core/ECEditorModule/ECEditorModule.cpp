// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ECEditorModule.h"
#include "ECEditorWindow.h"
#include "EcXmlEditorWidget.h"
#include "DoxygenDocReader.h"
#include "TreeWidgetItemExpandMemory.h"

#include "SceneAPI.h"
#include "Scene/Scene.h"
#include "Entity.h"
#include "InputAPI.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "EC_Placeable.h"
#include "QScriptEngineHelpers.h"

#include <QWebView>
#include <QList>

#include "MemoryLeakCheck.h"

Q_DECLARE_METATYPE(ECEditorWindow *)

// Shortcuts for config keys.
static const char *cGizmoEnabled= "show editing gizmo";
static const char *cHighlightingEnabled = "highlight selected entities";
static const char *cECEditorWindowPos = "eceditor window pos";

ECEditorModule::ECEditorModule() :
    IModule("ECEditor"),
    gizmoEnabled(true),
    highlightingEnabled(true),
    toggleSelectAllEntities(false)
{
}

ECEditorModule::~ECEditorModule()
{
}

void ECEditorModule::Initialize()
{
    expandMemory = ExpandMemoryPtr(new TreeWidgetItemExpandMemory(Name().toStdString().c_str(), framework_));

    ConfigAPI &cfg = *framework_->Config();
    ConfigData gizmoConfig(ConfigAPI::FILE_FRAMEWORK, Name(), cGizmoEnabled, gizmoEnabled, true);
    if (!cfg.HasValue(gizmoConfig))
        cfg.Set(gizmoConfig);
    gizmoEnabled = cfg.Get(gizmoConfig).toBool();

    ConfigData highlightConfig(ConfigAPI::FILE_FRAMEWORK, Name(), cHighlightingEnabled, highlightingEnabled, true);
    if (!cfg.HasValue(highlightConfig))
        cfg.Set(highlightConfig);
    highlightingEnabled = cfg.Get(highlightConfig).toBool();

    framework_->Console()->RegisterCommand("doc", "Prints the class documentation for the given symbol."
        "Usage example: 'doc(EC_Placeable::WorldPosition)'.", this, SLOT(ShowDocumentation(const QString &)));

    inputContext = framework_->Input()->RegisterInputContext("ECEditorInput", 90);
    connect(inputContext.get(), SIGNAL(KeyPressed(KeyEvent *)), this, SLOT(HandleKeyPressed(KeyEvent *)));

    /// @todo Ideally we wouldn't do this, but this is needed for now in order to get OnScriptEngineCreated called
    /// (and ECEditorWindow registered to QtScript) without generating dependendy to the JavascriptModule.
    framework_->RegisterDynamicObject("ecEditorModule", this);
}

void ECEditorModule::Uninitialize()
{
    ConfigAPI &cfg = *framework_->Config();
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, Name());
    cfg.Set(configData, cGizmoEnabled, gizmoEnabled);
    cfg.Set(configData, cHighlightingEnabled, highlightingEnabled);
    if (commonEditor)
        cfg.Set(configData, cECEditorWindowPos, commonEditor->pos());
    SAFE_DELETE(commonEditor);
    SAFE_DELETE_LATER(xmlEditor);
}

ECEditorWindow *ECEditorModule::ActiveEditor() const
{
    return activeEditor;
}

void ECEditorModule::SetGizmoEnabled(bool enabled)
{
    if (framework_->IsHeadless() || enabled == gizmoEnabled)
        return;

    gizmoEnabled = enabled;

    foreach(ECEditorWindow *editor, framework_->Ui()->MainWindow()->findChildren<ECEditorWindow *>())
        if (gizmoEnabled && editor == activeEditor) // if gizmoEnabled == true, show visual aids only for active editor.
            editor->SetGizmoVisible(gizmoEnabled);
        else
            editor->SetGizmoVisible(false);
}

void ECEditorModule::SetHighlightingEnabled(bool enabled)
{
    if (framework_->IsHeadless() || enabled == highlightingEnabled)
        return;

    highlightingEnabled = enabled;

    foreach(ECEditorWindow *editor, framework_->Ui()->MainWindow()->findChildren<ECEditorWindow *>())
        if (highlightingEnabled && editor == activeEditor) // if highlightingEnabled == true, show visual aids only for active editor.
            editor->SetHighlightingEnabled(highlightingEnabled);
        else
            editor->SetHighlightingEnabled(false);
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

    emit ActiveEditorChanged(activeEditor);
}

void ECEditorModule::ShowEditorWindow()
{
    if (framework_->IsHeadless())
        return;
    
    // Check null scene here, don't let it go to the ECEditorWindow ctor.
    if (!GetFramework()->Scene()->MainCameraScene())
    {
        LogError("ECEditorModule::ShowEditorWindow: Main camera scene is null.");
        return;
    }

    ConfigAPI &config = *framework_->Config();
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, Name(), cECEditorWindowPos);

    if (commonEditor)
    {
        commonEditor->setVisible(!commonEditor->isVisible());
        if (!commonEditor->isVisible())
        {
            config.Set(configData, cECEditorWindowPos, commonEditor->pos());
            commonEditor->close();
        }
        return;
    }

    activeEditor = new ECEditorWindow(GetFramework(), framework_->Ui()->MainWindow());
    commonEditor = activeEditor;
    activeEditor->setWindowFlags(Qt::Tool);
    activeEditor->setAttribute(Qt::WA_DeleteOnClose);
    // Load position from config
    QPoint pos = config.Get(configData).toPoint();
    UiMainWindow::EnsurePositionWithinDesktop(activeEditor, pos);
    activeEditor->show();
    activeEditor->activateWindow();
}

void ECEditorModule::ShowDocumentation(const QString &symbol)
{
    QUrl styleSheetPath;
    QString documentation;
    DoxygenDocReader::GetSymbolDocumentation(symbol, &documentation, &styleSheetPath);
    if (documentation.length() == 0)
    {
        LogError("ECEditorModule::ShowDocumentation: Failed to find documentation for symbol \"" + symbol + "\"!");
        return;
    }

    QWebView *webview = new QWebView(framework_->Ui()->MainWindow());
    webview->setAttribute(Qt::WA_DeleteOnClose);
    webview->setWindowFlags(Qt::Tool);
    webview->setHtml(documentation, styleSheetPath);
    webview->show();
}

void ECEditorModule::CreateXmlEditor(EntityPtr entity)
{
    CreateXmlEditor(QList<EntityPtr>(QList<EntityPtr>() << entity));
}

void ECEditorModule::CreateXmlEditor(const QList<EntityPtr> &entities)
{
    if (framework_->IsHeadless())
        return;
    if (entities.empty())
        return;

    if (!xmlEditor)
    {
        xmlEditor = new EcXmlEditorWidget(framework_, GetFramework()->Ui()->MainWindow());
        xmlEditor->setAttribute(Qt::WA_DeleteOnClose);
        xmlEditor->setWindowFlags(Qt::Tool);
        if (activeEditor) // make sure the editing gizmo follow the entity when it's saved
            connect(xmlEditor, SIGNAL(Saved()), activeEditor.data(), SLOT(RefreshPropertyBrowser()), Qt::UniqueConnection);
    }

    xmlEditor->SetEntity(entities);
    xmlEditor->show();
    xmlEditor->activateWindow();
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
        xmlEditor = new EcXmlEditorWidget(framework_, GetFramework()->Ui()->MainWindow());
        xmlEditor->setAttribute(Qt::WA_DeleteOnClose);
        xmlEditor->setWindowFlags(Qt::Tool);
    }

    xmlEditor->SetComponent(components);
    xmlEditor->show();
    xmlEditor->activateWindow();
}

void ECEditorModule::HandleKeyPressed(KeyEvent *e)
{
    if (e->eventType != KeyEvent::KeyPressed || e->keyPressCount > 1)
        return;

    const QKeySequence showEcEditor = framework_->Input()->KeyBinding("ShowECEditor", QKeySequence(Qt::ShiftModifier + Qt::Key_E));
    const QKeySequence toggle = framework_->Input()->KeyBinding("ToggleVisualEditingAids", QKeySequence(Qt::Key_section));
    const QKeySequence toggleSelectAll = framework_->Input()->KeyBinding("ToggleSelectAllEntities", QKeySequence(Qt::ControlModifier + Qt::Key_A));
    if (e->sequence == showEcEditor)
    {
        ShowEditorWindow();
        e->Suppress();
    }
    else if (e->sequence == toggle)
    {
        /// @todo For now toggling both, but should be we have separate keyboard shortcuts for individual editing aid?
        SetGizmoEnabled(!IsGizmoEnabled());
        SetHighlightingEnabled(!IsHighlightingEnabled());
        e->Suppress();
    }
    else if (e->sequence == toggleSelectAll)
    {
        // Only if visual editing is enabled
        if ((gizmoEnabled || highlightingEnabled) && activeEditor)
        {
            // Select/deselect all entities for the current editor
            toggleSelectAllEntities = !toggleSelectAllEntities;
            if (toggleSelectAllEntities)
            {
                Scene *activeScene = framework_->Scene()->MainCameraScene();
                if (activeScene)
                {
                    // We can only manipulate entities that have placeable, but exclude temporarys (avatar, cameras etc.)
                    /// @todo Shouldn't "select all" select all, not exclude some entities with some random criteria?
                    /// Maybe add separate actions for the right-click context menu for each criteria (local, replicated, temprorary etc.)
                    QList<entity_id_t> entIdsSelection;
                    foreach(const EntityPtr &e, activeScene->EntitiesWithComponent(EC_Placeable::TypeNameStatic()))
                        if (!e->IsTemporary())
                            entIdsSelection.append(e->Id());

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

void ECEditorModule::OnScriptEngineCreated(QScriptEngine* engine)
{
    qScriptRegisterQObjectMetaType<ECEditorWindow *>(engine);
}
