// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "InputFwd.h"

#include <QObject>
#include <QPointer>

class QScriptEngine;

class ECEditorWindow;
class EcXmlEditorWidget;
class TreeWidgetItemExpandMemory;
typedef shared_ptr<TreeWidgetItemExpandMemory> ExpandMemoryPtr;

/// Implements and enables visual editing of ECs.
/** @defgroup ECEditorModuleClient ECEditorModule Client interface. */

/// Implements and enables visual editing of ECs.
/** @ingroup ECEditorModuleClient */
class ECEDITOR_MODULE_API ECEditorModule : public IModule
{
    Q_OBJECT
    Q_PROPERTY(bool gizmoEnabled READ IsGizmoEnabled WRITE SetGizmoEnabled)
    Q_PROPERTY(bool highlightingEnabled READ IsHighlightingEnabled WRITE SetHighlightingEnabled)

public:
    ECEditorModule();
    virtual ~ECEditorModule();

    void Initialize();
    void Uninitialize();

    /// Sets the visual editing gizmo enabled.
    /** @note Applicable only if we have EC_TranformGizmo available in the build. */
    void SetGizmoEnabled(bool enabled);

    /// Is the visual editing gizmo enabled.
    bool IsGizmoEnabled() const { return gizmoEnabled; }

    /// Set do we want to highlight selected entities.
    /** @note Applicable only if we have EC_Highlight available in the build. */
    void SetHighlightingEnabled(bool enabled);

    /// Is highlighting of selected entities enabled.
    bool IsHighlightingEnabled() const { return highlightingEnabled; }

public slots:
    /// Shows the entity-component editor window.
    void ShowEditorWindow();

    /// Returns currently active editor.
    ECEditorWindow *ActiveEditor() const;

    /// Returns tree widget state memory object, which keeps track which items in EC editor are expanded.
    /** When constructing new EC editor windows use this if you want to keep all editor windows' state synchronized. */
    ExpandMemoryPtr ExpandMemory() const { return expandMemory; }

    /// Shows Doxygen documentation for symbol in external window.
    /** @param symbolName Name of the symbol (class, function, etc.)
        @bug Works only for functions, not for classes. */
    void ShowDocumentation(const QString &symbolName);

    /// ECEditor has gained a focus event and need to set as active editor.
    /** @param editor editor that has focus. */
    void ECEditorFocusChanged(ECEditorWindow *editor);

    /// Creates EC attribute XML editor widget for entities and components.
    /** @param entities List of entity pointers. */
    void CreateXmlEditor(const QList<EntityPtr> &entities);
    void CreateXmlEditor(EntityPtr entity); ///< @overload
    void CreateXmlEditor(ComponentPtr component); ///< @overload
    void CreateXmlEditor(const QList<ComponentPtr> &components); ///< @overload

signals:
    /// Signal is emitted when active ECEditorWindow's selection has changed.
    /** @param compType Selected item's component type name.
        @param compName Selected item's component name.
        @param attrType Selected item's attribute type name (Empty if attribute isn't selected).
        @param attrName Selected item's attribute name (Empty if attribute isn't selected). */
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

    /// Emitted when the active EC editor changes.
    /** @param editor The editor that just became active. */
    void ActiveEditorChanged(ECEditorWindow *editor);

private:
    InputContextPtr inputContext; ///< Input context.
    ExpandMemoryPtr expandMemory; ///< Keeps track which items in EC editor are expanded and collapsed.
    QPointer<EcXmlEditorWidget> xmlEditor; ///< EC XML editor window
    QPointer<ECEditorWindow> activeEditor; ///< Currently active ECEditorWindow.
    QPointer<ECEditorWindow> commonEditor; ///< ECEditorModule has one common editor for all parties to use.
    bool gizmoEnabled;
    bool highlightingEnabled;
    bool toggleSelectAllEntities;

private slots:
    /// Handles KeyPressed() signal from input context.
    /** @param e Key event. */
    void HandleKeyPressed(KeyEvent *e);

    /// Embeds the ECEditorModule types to the given script engine.
    void OnScriptEngineCreated(QScriptEngine* engine);
};
