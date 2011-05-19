// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorModule_h
#define incl_ECEditorModule_ECEditorModule_h

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
    EC Editor implements a way of adding arbitrary EC's to world entities.
*/
class ECEDITOR_MODULE_API ECEditorModule : public QObject, public IModule
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
        collapsed items similar.
    */
    ExpandMemoryPtr ExpandMemory() const { return expandMemory; }

    /// Shows Doxygen documentation for symbol in external window.
    /** @param symbolName Name of the symbol (class, function, etc.) */
    void ShowDocumentation(const QString &symbolName);

    /// ECEditor has gained a focus event and need to set as active editor.
    /// @param editor editor that has focus.
    void ECEditorFocusChanged(ECEditorWindow *editor);

    /// Creates EC attribute XML editor widget for entity.
    /// @param entity Entity pointer.
    void CreateXmlEditor(EntityPtr entity);

    /// Creates EC attribute XML editor widget for entity.
    /// @param entities List of entity pointers.
    void CreateXmlEditor(const QList<EntityPtr> &entities);

    /// Creates EC attribute XML editor widget for component.
    /// @param component Component pointer.
    void CreateXmlEditor(ComponentPtr component);

    /// Creates EC attribute XML editor widget for component.
    /// @param components List of component pointers.
    void CreateXmlEditor(const QList<ComponentPtr> &components);

    /// Return selected components from the active ECEditorWindow.
    /// If editor isn't initialized or any components aren't selected from the editor, method will return emtpy list.
    QObjectList GetSelectedComponents() const;

    /// Return selected entity ids as QVariantList from the active ECEditorWindow.
    QVariantList GetSelectedEntities() const;

signals:
    /// Signal is emitted when active ECEditorWindow's selection has changed.
    /** @param compType selected item's component type name.
        @param compName selected item's component name.
        @param attrType selected item's attribute type name (Empty if attribute isn't selected).
        @param attrName selected item's attribute name (Empty if attribute isn't selected).
    */
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

private:
    /// EC XML editor window
    QPointer<EcXmlEditorWidget> xmlEditor;

    /// Input context.
    boost::shared_ptr<InputContext> inputContext;

    /// Tree widget item expand memory keeps track of which items in EC editor are expanded.
    ExpandMemoryPtr expandMemory;

    /// Currently active ECEditorWindow.
    QPointer<ECEditorWindow> activeEditor;

    /// ECEditorModule will create it's own ECEditorWindow instance while it's initializing.
    /// To avoid a memory leak, we store that pointer.
    QPointer<ECEditorWindow> commonEditor;

private slots:
    /// Handles KeyPressed() signal from input context.
    /** @param e Key event.
    */
    void HandleKeyPressed(KeyEvent *e);

    // When active_editor is destroyed we need to set it's pointer back to null.
    void ActiveECEditorDestroyed(QObject *obj = 0);
};

#endif
