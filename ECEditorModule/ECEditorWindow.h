/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.h
 *  @brief  Entity-component editor window.
 */

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "ECEditorModuleApi.h"
#include "ForwardDefines.h"
#include "CoreTypes.h"
#include "Entity.h"

#include <QMap>
#include <QSet>
#include <QListWidgetItem>
#include <QPointer>
#include <QWidget>

extern std::vector<std::string> AttributeTypenames;

class QPushButton;
class QListWidget;
class QTreeWidget;
class QPoint;

struct EntityComponentSelection
{
    Scene::EntityPtr entity;
    std::vector<ComponentPtr> components;
};

class ECBrowser;
class AddComponentDialog;

//! Contains entity pointer as a QPointer. This class is used to indentify a right item using an entity ID.
//! \ingroup ECEditorModuleClient.
class EntityListWidgetItem: public QListWidgetItem
{
public:
    EntityListWidgetItem(const QString &name, QListWidget *list, Scene::Entity *entity):
        QListWidgetItem(name, list),
        entity_ptr_(entity)
    {
    }
    
    // Returns QPointer of entity that this item is presenting.
    QPointer<Scene::Entity> GetEntity() const { return entity_ptr_; }

private:
    //Weak pointer to entity switch will get released and set to null when QObject's destructor is called.
    QPointer<Scene::Entity> entity_ptr_;
};

//! Entity-component editor window.
/*! \ingroup ECEditorModuleClient.
*/
class ECEDITOR_MODULE_API ECEditorWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    /** @param framework Framework.
    */
    explicit ECEditorWindow(Foundation::Framework* framework);

    /// Destructor.
    ~ECEditorWindow();

    /// Adds new entity to the entity list.
    /** @param id Entity ID.
    */
    void AddEntity(entity_id_t id, bool udpate_ui = true);

    /// Set new list of entities to ECEditor. Calling this method will clear 
    /// previously selected entities from the editor.
    /** @param entities a new list of entities that we want to add into the editor.
     *  @param select_all Do we want to select all entities from the list.
    */
    void AddEntities(const QList<entity_id_t> &entities, bool select_all = false);

    /// Removes entity from the entity list.
    /** @param id Entity ID.
    */
    void RemoveEntity(entity_id_t id, bool udpate_ui = true); 

    /// Sets which entities are selected in the editor window.
    /** @param ids List of entity ID's.
    */
    void SetSelectedEntities(const QList<entity_id_t> &ids);

    /// Clears entity list.
    void ClearEntities();

    /// Ask ECBrowser what components are currently selected and return them as QObjectList.
    /// If any components aren't selected return emtpy list.
    QObjectList GetSelectedComponents() const;

    /// Returns list of selected entities.
    QList<Scene::EntityPtr> GetSelectedEntities() const;

public slots:
    /// Deletes selected entity entries from the list (does not delete the entity itself).
    void DeleteEntitiesFromList();

    //! Remove coponent from entity and refresh property browser.
    void DeleteComponent(const QString &componentType, const QString &name);

    /// Opens a dialog that will handle new entity creation.
    /** After the dialog is done, ComponentDialogFinished method is called.
     */
    void CreateComponent();

    /// Shows dialog for invoking entity actions.
    void OpenEntityActionDialog();

    /// Called by entity action dialog when it's finished.
    /** @param result Result of finished. Close is 0, Execute and Close is 1, Execute is 2.
    */
    void EntityActionDialogFinished(int result);

    /// Shows dialog for invoking functions.
    void OpenFunctionDialog();

    /// Called by function dialog when it's finished.
    /** @param result Result of finished. Close is 0, Execute and Close is 1, Execute is 2.
    */
    void FunctionDialogFinished(int result);

    /// If entity selection different from previous update change browser to fit those changes.
    void RefreshPropertyBrowser();

    /// Shows context menu for entities.
    /// @param pos Mouse position of right-click event.
    void ShowEntityContextMenu(const QPoint &pos);

    /// Shows EC XML editor.for entity's all components.
    void ShowXmlEditorForEntity();

    /// Shows EC XML editor.for each components.
    void ShowXmlEditorForComponent(std::vector<ComponentPtr> components);

    /// Shows EC XML editor.for a single component.
    //void ShowXmlEditorForComponent();
    void ShowXmlEditorForComponent(const std::string &componentType);

    /// Show/Hide entity list.
    void ToggleEntityList();

    /// Checks if deleted entity is located in editor's list and if so remove it from the editor.
    void EntityRemoved(Scene::Entity* entity);

    /// Set focus to this editor window. When window have
    /// focus it should accept entity select actions and add clicked entities from the scene.
    void SetFocus(bool focus);

    void setVisible(bool visible);

signals:
    /// Emitted user wants to edit entity's EC attributes in XML editor.
    void EditEntityXml(Scene::EntityPtr entity);

    /// Emitted user wants to edit EC attributes in XML editor.
    void EditComponentXml(ComponentPtr component);

    /// Emitted user wants to edit entity's EC attributes in XML editor.
    /// @param entities list of entities
    void EditEntityXml(const QList<Scene::EntityPtr> &entities);

    /// Emitted user wants to edit EC attributes in XML editor.
    /// @param list of components
    void EditComponentXml(const QList<ComponentPtr> & components);

    /// Signal is emmitted when this window has gained a focus.
    void OnFocusChanged(ECEditorWindow *editor);

    /// Forward ECBrowser's SelectionChanged signal to ECEditorModule, so in script
    /// we know when new selection has occurred.
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

protected:
    /// QWidget override.
    void hideEvent(QHideEvent *hide_event);

    /// QWidget override.
    void changeEvent(QEvent *change_event);

    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    /// Listens SceneManager's ActionTriggered signal and if action was "MousePress"
    /// add entity to editor window (assuming that editor has a focus).
    void ActionTriggered(Scene::Entity *entity, const QString &action, const QStringList &params);

    /// Deletes entity.
    void DeleteEntity();

    /// Copy serializable component values to clipboard.
    void CopyEntity();

    /// Paste create a new entity and add serializable components.
    void PasteEntity();

    /// Highlights all entities from the entities_list that own an instance of given component.
    void HighlightEntities(const QString &type, const QString &name);

    /// Listenes when default world scene has changed and clear the editor window.
    /// @param scene new default world scene.
    void DefaultSceneChanged(Scene::SceneManager *scene);

    //When user have pressed ok or cancel button in component dialog this mehtod is called.
    void ComponentDialogFinished(int result);

private:
    /// Bold all given entities from the entity_list_ QListWidget object.
    /// Note! this method will unbold previous selection.
    void BoldEntityListItems(const QSet<entity_id_t> &bolded_entities);

    /// Initializes the widget.
    void Initialize();

    /// Framework pointer.
    Foundation::Framework *framework_;

    QPushButton* toggle_entities_button_;
    QListWidget* entity_list_;
    ECBrowser *browser_;
    QPointer<AddComponentDialog> component_dialog_; /// @todo remove this.
    bool has_focus_; // To tack if this editor has a focus.
};

#endif
