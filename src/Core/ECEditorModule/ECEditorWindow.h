/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   ECEditorWindow.h
    @brief  Entity-component editor window. */

#pragma once

#include "ECEditorModuleApi.h"
#include "ui_ECEditor.h"

#include "CoreTypes.h"
#include "SceneFwd.h"
#include "InputFwd.h"

#include <QSet>
#include <QListWidgetItem>
#include <QPointer>

class QPoint;
class QUndoStack;

class Framework;
class ECBrowser;
class TransformEditor;
class UndoManager;

/// List widget item representing entity.
/** Holds a weak pointer to the represented entity.
    @ingroup ECEditorModuleClient. */
class ECEDITOR_MODULE_API EntityListWidgetItem : public QListWidgetItem
{
public:
    EntityListWidgetItem(const QString &name, QListWidget *list, const EntityPtr &e) :
        QListWidgetItem(name, list), entity(e) {}

    /// Returns shared pointer to the entity this item is presenting.
    EntityPtr Entity() const { return entity.lock(); }

private:
    EntityWeakPtr entity; ///< Weak pointer to the represented entity.
};

/// Entity-component editor window.
/** @ingroup ECEditorModuleClient.
    @todo Currently EC Editor is hardcoded to edit the main camera scene.
          Enhance the API so that the scene can be set by the user. */
class ECEDITOR_MODULE_API ECEditorWindow : public QWidget, public Ui::ECEditor
{
    Q_OBJECT

public:
    /// Constructor
    /** @param fw Framework. */
    explicit ECEditorWindow(Framework* fw, QWidget *parent = 0);

    ~ECEditorWindow();

    /// Adds new entity to the entity list.
    /** @param entity Entity.
        @param updateUi Do we want to update the UI.
        @return The created list item. */
    EntityListWidgetItem *AddEntity(const EntityPtr &entity, bool updateUi = true);
    EntityListWidgetItem *AddEntity(entity_id_t id, bool updateUi = true); /**< @overload @param id Entity ID. */

    /// Sets new list of entities to be shown in the editor.
    /** Calling this method will clear previously selected entities from the editor.
        @param entities a new list of entities that we want to add into the editor.
        @param selectAll Do we want to select all entities from the list. */
    void AddEntities(const EntityList &entities, bool selectAll = false);
    void AddEntities(const QList<entity_id_t> &entities, bool selectAll = false); /**< @overload @param entities IDs of the entities. */

    /// Removes entity from the entity list.
    /** @param id Entity ID.
        @param updateUi Do we want to update the UI. */
    void RemoveEntity(entity_id_t id, bool udpateUi = true);

    /// Sets which entities are selected in the editor window.
    /** @param ids List of entity ID's. */
    void SetSelectedEntities(const QList<entity_id_t> &ids);

    /// Clears the entity list.
    void ClearEntities();

    /// Returns components that are currently selected.
    /** @return If any components aren't selected return empty list. */
    std::vector<ComponentPtr> SelectedComponents() const;

    /// Returns list of selected entities.
    EntityList SelectedEntities() const;

    /// Sets item active in the entity list. Also adds/removes EC_Highlight for the entity, if applicable.
    /** @param item Item to be select or deselect
        @param select Do we want to select or deselect. */
    void SetEntitySelected(EntityListWidgetItem *item, bool select, bool signal = true);

    /// Returns list item for specific entity.
    /** @param id Entity ID. */
    EntityListWidgetItem *FindItem(entity_id_t id) const;

    /// Sets highlighting of selected entities for this editor window (if applicable).
    /** @note The visibility state is not stored within this class. ECEditorModule is authoritative for this state.
        @param show Do we want to show or hide the visual editing aids. */
    void SetHighlightingEnabled(bool show);

    /// Sets visibility of editing gizmo for this editor window (if applicable).
    /** @note The visibility state is not stored within this class. ECEditorModule is authoritative for this state. */
    void SetGizmoVisible(bool show);

    /// Returns undo manager for this editor.
    UndoManager *GetUndoManager() const { return undoManager_; }

public slots:
    /// Refreshes the component view to match the selected entities.
    void Refresh();

    /// Show/Hide entity list.
    void ToggleEntityList();

    /// Removes entity item from the editor's list.
    void RemoveEntity(Entity* entity);

    /// Set focus to this editor window.
    /** When window has focus it should accept entity select actions and add clicked entities from the scene.
        Also, when window is unfocused, its transform gizmo (if applicable) is hidden. */
    void SetFocus(bool focus);

    /// QWidget override.
    void setVisible(bool visible);

    /// Deselects all entities in the list.
    /** @note Emits EntitiesSelected only, not EntitySelected. */
    void DeselectAllEntities();

signals:
    /// Emitted user wants to edit entity's EC attributes in XML editor.
    /** @param entities list of entities.
        @todo Use EntityList instead. */
    void EditEntityXml(const QList<EntityPtr> &entities);

    /// Emitted user wants to edit EC attributes in XML editor.
    /** @param list of components.
        @todo Use std::vector<ComponentPtr> instead. */
    void EditComponentXml(const QList<ComponentPtr> & components);

    /// Signal is emitted when this window has gained a focus.
    void FocusChanged(ECEditorWindow *editor);

    /// Forwards ECBrowser's SelectionChanged signal to ECEditorModule, so in script we know when new selection has occurred.
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

    /// Emitted when entity is selected or deselected.
    /** @param entity Entity in question.
        @param selected Was the entity selected (true) or deselected (false). */
    void EntitySelected(const EntityPtr &entity, bool selected);

    /// Same as EntitySelected but used for batch selections.
    void EntitiesSelected(const EntityList &entity, bool selected);

protected:
    void hideEvent(QHideEvent *e); ///< QWidget override.
    void changeEvent(QEvent *e); ///< QWidget override.
    bool eventFilter(QObject *obj, QEvent *event); ///< QWidget override.

private slots:
    /// Opens a dialog that will handle new entity creation.
    /** After the dialog is done, ComponentDialogFinished method is called. */
    void CreateComponent();

    /// Shows dialog for invoking entity actions for currently selected entities.
    void OpenEntityActionDialog();

    /// Shows dialog for invoking functions for currently selected entities.
    void OpenFunctionDialog();

    /// Shows context menu for entities.
    /** @param pos Mouse position of right-click event. */
    void ShowEntityContextMenu(const QPoint &pos);

    /// Shows EC XML editor for entity's all components.
    void ShowXmlEditorForEntity();

    /// Shows EC XML editor for specific components.
    void ShowXmlEditorForComponent(const QList<ComponentPtr> &components);

    /// Shows EC XML editor for a single component.
    void ShowXmlEditorForComponent(const QString &componentType);

    /// Highlights an entity.
    /** @note No-op if EC_Highlight is not included in the build.
        @param entity Entity to be highlighted.
        @param highlight Do we want to show highlight or hide it. */
    void HighlightEntity(const EntityPtr &entity, bool highlight);

    void OnKeyEvent(KeyEvent *keyEvent);

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

    /// Called by add component dialog when it's finished.
    void AddComponentDialogFinished(int result);

    void OnAboutToEditAttribute(IAttribute *attr);
    void OnUndoChanged(bool canUndo);
    void OnRedoChanged(bool canRedo);
    /// Clears the window if the scene of which entities are shown is cleared or removed.
    void OnSceneRemoved(Scene *);

private:
    struct EntityComponentSelection
    {
        EntityPtr entity;
        std::vector<ComponentPtr> components;
    };

    /// Bold all given entities from the entity_list_ QListWidget object.
    /** @note Will unbold previous selection. */
    void BoldEntityListItems(const QSet<entity_id_t> &bolded_entities);

    Framework *framework;
    ECBrowser *ecBrowser;
    bool hasFocus; ///< To track if this editor has a focus.
    TransformEditor *transformEditor;
    UndoManager * undoManager_;
};
