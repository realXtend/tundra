// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"
#include "ECEditorModuleApi.h"
#include "SceneFwd.h"
#include "InputFwd.h"

#include <QObject>
#include <QPointer>
#include <QVariantList>

class ECEditorWindow;
class EcXmlEditorWidget;
class TreeWidgetItemExpandMemory;
typedef boost::shared_ptr<TreeWidgetItemExpandMemory> ExpandMemoryPtr;

/// Entity-component editor module.
/** @defgroup ECEditorModuleClient ECEditorModule Client interface.
    Implements and enables visual editing of ECs. */
class ECEDITOR_MODULE_API ECEditorModule : public IModule
{
    Q_OBJECT

public:
    ECEditorModule();
    virtual ~ECEditorModule();

    // IModuleImpl overrides.
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(f64 frametime);

public slots:
    /// Shows the entity-component editor window.
    void ShowEditorWindow();

    /// Returns currently active editor.
    ECEditorWindow *ActiveEditor() const;

    /// Return Tree widget item expand memory pointer, which keeps track of which items in EC editor are expanded.
    /** When constructing new EC editor windows use this if you want to keep all editor windows' expanded and 
        collapsed items similar. */
    ExpandMemoryPtr ExpandMemory() const { return expandMemory; }

    /// Sets do we want to show visual editing aids (gizmo and highlights) when EC editor is open/active.
    /// This value is applicable for all open/active EC editors which are children of the main window.
    /** @note The effect of this depends whether or not we have EC_Highlight and EC_TranformGizmo available in the build.
        @param show Do we want to show or hide visual editings aids. */
    void ShowVisualEditingAids(bool show);

    /// Returns are we showing transform editing gizmo when EC editor is open/active.
    bool VisualEditingAidsEnabled() const { return showVisualAids; }

    /// Shows Doxygen documentation for symbol in external window.
    /** @param symbolName Name of the symbol (class, function, etc.) */
    void ShowDocumentation(const QString &symbolName);

    /// ECEditor has gained a focus event and need to set as active editor.
    /** @param editor editor that has focus. */
    void ECEditorFocusChanged(ECEditorWindow *editor);

    /// Creates EC attribute XML editor widget for entities and components.
    /** @param entities List of entity pointers. */
    void CreateXmlEditor(const QList<EntityPtr> &entities);

    /// This is an overloaded function.
    /** @param entity Entity pointer. */
    void CreateXmlEditor(EntityPtr entity);

    /// This is an overloaded function.
    /** @param component Component pointer. */
    void CreateXmlEditor(ComponentPtr component);

    /// This is an overloaded function.
    /** @param components List of component pointers. */
    void CreateXmlEditor(const QList<ComponentPtr> &components);

    /// Returns Selected components from the active ECEditorWindow.
    /** @return If editor isn't initialized or any components aren't selected from the editor, method will return an empty list. */
    QObjectList GetSelectedComponents() const;

    /// Return selected entity ids as QVariantList from the active ECEditorWindow.
    QVariantList GetSelectedEntities() const;

signals:
    /// Signal is emitted when active ECEditorWindow's selection has changed.
    /** @param compType Selected item's component type name.
        @param compName Selected item's component name.
        @param attrType Selected item's attribute type name (Empty if attribute isn't selected).
        @param attrName Selected item's attribute name (Empty if attribute isn't selected). */
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

private:
    InputContextPtr inputContext; ///< Input context.
    ExpandMemoryPtr expandMemory; ///< Keeps track which items in EC editor are expanded and collapsed.
    QPointer<EcXmlEditorWidget> xmlEditor; ///< EC XML editor window
    QPointer<ECEditorWindow> activeEditor; ///< Currently active ECEditorWindow.
    QPointer<ECEditorWindow> commonEditor; ///< ECEditorModule has one common editor for all parties to use.
    bool showVisualAids; ///< Do we want to show visual editing aids (gizmo and highlights) when EC editor is open/active.

private slots:
    /// Handles KeyPressed() signal from input context.
    /** @param e Key event. */
    void HandleKeyPressed(KeyEvent *e);
};
