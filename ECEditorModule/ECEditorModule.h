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

class TreeWidgetItemExpandMemory;
typedef boost::shared_ptr<TreeWidgetItemExpandMemory> ExpandMemoryPtr;

class ECEditorWindow;
class EcXmlEditorWidget;

/// EC Editor module
/** \defgroup ECEditorModuleClient ECEditorModule Client interface.
 *  EC Editor implements a way of adding arbitrary EC's to world entities.
 */
class ECEDITOR_MODULE_API ECEditorModule : public QObject, public IModule
{
    Q_OBJECT

public:
    /// Constructor.
    ECEditorModule();

    /// Destructor 
    virtual ~ECEditorModule();

    /// IModuleImpl overrides.
    void Load();
    void Initialize();
    void PostInitialize();
    void Uninitialize();
    void Update(f64 frametime);
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

    /// Show EC editor window.
    //Console::CommandResult ShowWindow(const StringVector &params);

    Console::CommandResult ShowDocumentation(const StringVector &params);

    /// Added for testing EC_DynamicComponent.
    /** @param params Params should be following:
     *  0 = entity id.
     *  1 = operation (add or rem)
     *  2 = component type.(ec. EC_DynamicComponent)
     *  3 = attribute name.
     *  4 = attribute type. !Only rem dont use in rem operation.
     *  5 = attribute value. !Only rem dont use in rem operation.
     */
    Console::CommandResult EditDynamicComponent(const StringVector &params);

    ECEditorWindow *GetActiveECEditor() const;

    /// returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return name_static_; }

    /// Return Tree widget item expand memory pointer, which keeps track of which items in EC editor are expanded.
    /** When constructing new EC editor windows use this if you want to keep all editor windows' expanded and 
        collapsed items similar.
    */
    ExpandMemoryPtr ExpandMemory() const { return expandMemory; }

public slots:
    /// ECEditor has gained a focus event and need to set as active editor.
    /// @param editor editor that has focus.
    void ECEditorFocusChanged(ECEditorWindow *editor);

    void AddEditorWindowToUI();

    /// Creates EC attribute XML editor widget for entity.
    /// \param entity Entity pointer.
    void CreateXmlEditor(EntityPtr entity);

    /// Creates EC attribute XML editor widget for entity.
    /// \param entities List of entity pointers.
    void CreateXmlEditor(const QList<EntityPtr> &entities);

    /// Creates EC attribute XML editor widget for component.
    /// \param component Component pointer.
    void CreateXmlEditor(ComponentPtr component);

    /// Creates EC attribute XML editor widget for component.
    /// \param components List of component pointers.
    void CreateXmlEditor(const QList<ComponentPtr> &components);

    /// Return selected components from the active ECEditorWindow.
    /// If edtior isn't initialized or any components aren't selected from the editor, method will return emtpy list.
    QObjectList GetSelectedComponents() const;

    /// Return selected entity ids as QVariantList from the active ECEditorWindow.
    QVariantList GetSelectedEntities() const;

    bool IsECEditorWindowVisible() const;

signals:
    /// Signal is emitted when active ECEditorWindow's selection has changed.
    /** @param compType selected item's component type name.
     *  @param compName selected item's component name.
     *  @param attrType selected item's attribute type name (Empty if attribute isn't selected).
     *  @param attrName selected item's attribute name (Empty if attribute isn't selected).
     */
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

private:
    /// Static name of the module
    static std::string name_static_;

    /// EC XML editor window
    QPointer<EcXmlEditorWidget> xmlEditor_;

    /// Input context.
    boost::shared_ptr<InputContext> inputContext;

    /// Tree widget item expand memory keeps track of which items in EC editor are expanded.
    ExpandMemoryPtr expandMemory;

    /// Active ECEditorWindow.
    QPointer<ECEditorWindow> active_editor_;

    /// ECEditorModule will create it's own ECEditorWindow instance while it's initializing. 
    /// To avoid a memory leak, we store that pointer.
    QPointer<ECEditorWindow> common_editor_;

private slots:
    /// Handles KeyPressed() signal from input context.
    /** @param e Key event.
    */
    void HandleKeyPressed(KeyEvent *e);

    // When active_editor is destroyed we need to set it's pointer back to null.
    void ActiveECEditorDestroyed(QObject *obj = 0);
};

#endif

