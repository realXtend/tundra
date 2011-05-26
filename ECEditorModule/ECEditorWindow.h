/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.h
 *  @brief  Entity-component editor window.
 */

#pragma once

#include "ECEditorModuleApi.h"
#include "CoreTypes.h"
#include "SceneFwd.h"

#include <QMap>
#include <QSet>
#include <QListWidgetItem>
#include <QPointer>
#include <QWidget>

class QPushButton;
class QListWidget;
class QTreeWidget;
class QPoint;

struct EntityComponentSelection
{
    EntityPtr entity;
    std::vector<ComponentPtr> components;
};

class Framework;
class ECBrowser;
class TransformEditor;

/// List widget item representing entity. Holds a weak pointer to the represented entity.
/// \ingroup ECEditorModuleClient.
class EntityListWidgetItem: public QListWidgetItem
{
public:
    EntityListWidgetItem(const QString &name, QListWidget *list, Entity *entity);

    // Returns shared pointer of the entity that this item is presenting.
    EntityPtr GetEntity() const { return entity.lock(); }

private:
    //Weak pointer to entity switch will get released and set to null when QObject's destructor is called.
    EntityWeakPtr entity;
};

/// Entity-component editor window.
/** \ingroup ECEditorModuleClient.
*/
class ECEDITOR_MODULE_API ECEditorWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructor
    /** @param fw Framework. */
    explicit ECEditorWindow(Framework* fw, QWidget *parent = 0);

    /// Destructor.
    ~ECEditorWindow();

    /// Adds new entity to the entity list.
    /** @param id Entity ID.
        @param updateUi Do we want to update UI.
        @return The created list item.
    */
    EntityListWidgetItem *AddEntity(entity_id_t id, bool updateUi = true);

    /// Sets new list of entities to be shown in the editor.
    /** Calling this method will clear previously selected entities from the editor.
        @param entities a new list of entities that we want to add into the editor.
        @param select_all Do we want to select all entities from the list.
    */
    void AddEntities(const QList<entity_id_t> &entities, bool select_all = false);

    /// Removes entity from the entity list.
    /** @param id Entity ID. */
    void RemoveEntity(entity_id_t id, bool udpate_ui = true); 

    /// Sets which entities are selected in the editor window.
    /** @param ids List of entity ID's. */
    void SetSelectedEntities(const QList<entity_id_t> &ids);

    /// Clears entity list.
    void ClearEntities();

    /// Returns components that are currently selected.
    /** @return If any components aren't selected return emtpy list. */
    QObjectList GetSelectedComponents() const;

    /// Returns list of selected entities.
    QList<EntityPtr> GetSelectedEntities() const;

    /// Sets item active in the entity list. Also adds/removes EC_Highlight for the entity, if applicable.
    /** @param item Item to be select or deselect
        @param select Do we want to select or deselect.
    */
    void SetEntitySelected(EntityListWidgetItem *item, bool select);

    /// Returns list item for specific entity.
    /** @param id Entity ID. */
    EntityListWidgetItem *FindItem(entity_id_t id) const;

public slots:
    /// Deletes selected entity entries from the list (does not delete the entity itself).
    void DeleteEntitiesFromList();

    /// Remove coponent from entity and refresh property browser.
    void DeleteComponent(const QString &componentType, const QString &name);

    /// Opens a dialog that will handle new entity creation.
    /** After the dialog is done, ComponentDialogFinished method is called. */
    void CreateComponent();

    /// Shows dialog for invoking entity actions for currently selected entities.
    void OpenEntityActionDialog();

    /// Shows dialog for invoking functions for currently selected entities.
    void OpenFunctionDialog();

    /// If entity selection different from previous update change browser to fit those changes.
    ///\todo Rename or split into pieces; does more just than the aforementioned.
    void RefreshPropertyBrowser();

    /// Shows context menu for entities.
    /** @param pos Mouse position of right-click event. */
    void ShowEntityContextMenu(const QPoint &pos);

    /// Shows EC XML editor.for entity's all components.
    void ShowXmlEditorForEntity();

    /// Shows EC XML editor.for each components.
    void ShowXmlEditorForComponent(const std::vector<ComponentPtr> &components);

    /// Shows EC XML editor.for a single component.
    void ShowXmlEditorForComponent(const QString &componentType);

    /// Show/Hide entity list.
    void ToggleEntityList();

    /// Removes entity item from the editor's list.
    void RemoveEntity(Entity* entity);

    /// Set focus to this editor window.
    /** When window has focus it should accept entity select actions and add clicked entities from the scene.
        Also when windows if unfocues, its transform gizmo (if applicable) is hidden. */
    void SetFocus(bool focus);

    /// QWidget override.
    void setVisible(bool visible);

    /// Deselects all entities in the list.
    void DeselectAllEntities();

    /// Highlights an entity.
    /** @note No-op if EC_Highlight is not included in the build.
        @param entity Entity to be highlighted.
        @param highlight Do we want to show highlight or hide it.
    */
    void HighlightEntity(const EntityPtr &entity, bool highlight);

signals:
    /// Emitted user wants to edit entity's EC attributes in XML editor.
    void EditEntityXml(EntityPtr entity);

    /// Emitted user wants to edit EC attributes in XML editor.
    void EditComponentXml(ComponentPtr component);

    /// Emitted user wants to edit entity's EC attributes in XML editor.
    /** @param entities list of entities */
    void EditEntityXml(const QList<EntityPtr> &entities);

    /// Emitted user wants to edit EC attributes in XML editor.
    /** @param list of components */
    void EditComponentXml(const QList<ComponentPtr> & components);

    /// Signal is emitted when this window has gained a focus.
    void FocusChanged(ECEditorWindow *editor);

    /// Forwards ECBrowser's SelectionChanged signal to ECEditorModule, so in script we know when new selection has occurred.
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

protected:
    /// QWidget override.
    void hideEvent(QHideEvent *e);

    /// QWidget override.
    void changeEvent(QEvent *e);

    /// QWidget override.
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    /// Called by entity action dialog when it's finished.
    /** @param result Result of finished. Close is 0, Execute and Close is 1, Execute is 2. */
    void EntityActionDialogFinished(int result);

    /// Called by function dialog when it's finished.
    /** @param result Result of finished. Close is 0, Execute and Close is 1, Execute is 2. */
    void FunctionDialogFinished(int result);

    /// Adds entity to the editor window's entity list (assuming that editor has a focus).
    void OnActionTriggered(Entity *entity, const QString &action, const QStringList &params);

    /// Deletes entity.
    void DeleteEntity();

    /// Copies serializable component values to clipboard.
    void CopyEntity();

    /// Paste create a new entity and add serializable components.
    void PasteEntity();

    /// Highlights all entities from the entities_list that own an instance of given component.
    void HighlightEntities(const QString &type, const QString &name);

    /// Listens when default world scene changes and clears the editor window.
    /** @param scene new default world scene. */
    void OnDefaultSceneChanged(Scene *scene);

    /// Called by add component dialog when it's finished.
    void AddComponentDialogFinished(int result);

private:
    /// Bold all given entities from the entity_list_ QListWidget object.
    /// Note! this method will unbold previous selection.
    void BoldEntityListItems(const QSet<entity_id_t> &bolded_entities);

    Framework *framework; ///< Framework pointer.
    QPushButton* toggleEntitiesButton;
    QListWidget* entityList;
    ECBrowser *ecBrowser;
    bool hasFocus; ///< To track if this editor has a focus.
    TransformEditor *transformEditor;
};
