/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneStructureWindow.h
    @brief  Window with tree view showing every entity in a scene. */

#pragma once

#include "SceneFwd.h"
#include "CoreTypes.h"

#include <QWidget>
#include <QTimer>
#include <QHash>

class SceneTreeWidget;
class Framework;
class EntityItem;
class EntityGroupItem;
class ComponentItem;
class AttributeItem;

class QTreeWidgetItem;
class QLineEdit;
class QPushButton;
class QTreeWidgetItem;
class QToolButton;
class QCheckBox;
class QComboBox;

/// Window with tree view showing every entity in a scene.
/** This class will only handle adding and removing of entities and components and updating
    their names. The SceneTreeWidget implements most of the functionality. */
class SceneStructureWindow : public QWidget
{
    Q_OBJECT
    Q_ENUMS(AttributeVisibility)
    Q_ENUMS(SortCriteria)

public:
    /// Constructs the window.
    /** @param fw Framework.
        @parent parent Parent widget. */
    explicit SceneStructureWindow(Framework *fw, QWidget *parent = 0);

    ~SceneStructureWindow();

    /// Sets new scene to be shown in the tree view.
    /** Populates tree view with entities.
        If scene is set to null, the tree view is cleared and previous signal connections are disconnected.
        @param newScene Scene. */
    void SetShownScene(const ScenePtr &newScene);
    /// The scene that is shown in the tree view.
    ScenePtr ShownScene() const { return scene.lock(); }

    enum AttributeVisibilityType
    {
        DoNotShowAttributes, ///< Do not show attributes.
        ShowAssetReferences, ///< Show asset reference attributes (AssetReference and AssetReferenceList).
        ShowDynamicAttributes, ///< Show dynamic attributes.
        ShowAllAttributes ///< Show all attributes.
    };

    enum SortCriteria
    {
        SortById,
        SortByName
    };

    /// Sets the sorting criteria and sorts the contents accordingly.
    void SortBy(SortCriteria criteria, Qt::SortOrder order);

    /// Returns the currently used sorting criteria.
    SortCriteria SortingCriteria() const { return sortingCriteria; }

public slots:
    /// Sets do we want to entity groups in the tree view.
    /** @param show Visibility of entity groups in the tree view.
        @note Due to optimizations this will not immediately sort the tree. */
    void ShowGroups(bool show);

    /// Sets do we want to show components in the tree view.
    /** @param show Visibility of components in the tree view. */
    void ShowComponents(bool show, bool refreshView);

    /// Sets what kind of attributes we want to show in the tree view.
    /** @param type The type of attributes we want to show. */
    void SetAttributeVisibility(AttributeVisibilityType type);

    /// Decorates (bolds) or undecorates item representing the @c entity.
    /** @param entity Entity in question.
        @param selected Whether to decorate (true) or undecorate (false) the item. */
    void SetEntitySelected(const EntityPtr &entity, bool selected);

    /// Decorates (bolds) or undecorates items representing the @c entities.
    /** @param entities Entities in question.
        @param selected Whether to decorate (true) or undecorate (false) the item. */
    void SetEntitiesSelected(const EntityList &entities, bool selected);

    /// Undecorates all selected entities.
    void ClearSelectedEntites();

signals:
    /// Emitted when the widget is about to be closed.
    void AboutToClose(SceneStructureWindow *window);

protected:
    void changeEvent(QEvent* e);     ///< QWidget override.
    void closeEvent(QCloseEvent *e); ///< QWidget override.

private:
    /// Populates tree widget with all entities.
    void Populate();

    /// Creates attribute items for a single entity item.
    void CreateAttributesForItem(ComponentItem *cItem);
    void CreateAttributesForItem(EntityItem *eItem);

    /// Shows attributes of the current visibility type or hides the all.
    void SetAttributesVisible(bool show, bool refreshView);

    /// Create asset reference item to the tree widget.
    /** @param parentItem Parent item, can be entity or component item.
        @param attr AssetReference attribute. */
    void CreateAttributeItem(QTreeWidgetItem *parentItem, IAttribute *attr);

    EntityGroupItem *GetOrCreateEntityGroupItem(const QString &name, bool addToTreeRoot);
    void RemoveEntityGroupItem(const QString &name);
    /// @note gItem will be deleted and null after calling this function.
    void RemoveEntityGroupItem(EntityGroupItem *gItem);

    EntityItem* EntityItemOfEntity(Entity* ent) const;
    /// @note This function does lookup from a different map than EntityItemOfEntity.
    EntityItem* EntityItemById(entity_id_t id) const ;

    /// Item @c eItem was acked as @c entity with previous id @c oldId
    void EntityItemAcked(EntityItem* entityItem, Entity *entity, entity_id_t oldId);

    void RemoveEntityItem(EntityItem* item);
    void RemoveChildEntityItems(EntityItem* eItem);
    ComponentItem *ComponentItemOfComponent(IComponent *) const;
    std::vector<AttributeItem *> AttributeItemOfAttribute(IAttribute *) const;
    void SetEntityItemSelected(EntityItem *item, bool selected);

    void Refresh();

    Framework *framework;
    SceneWeakPtr scene; ///< Scene which we are showing the in tree widget currently.
    SceneTreeWidget *treeWidget; ///< Scene tree widget.
    bool showGroups; ///< Are entity groups shown in the tree view.
    bool showComponents; ///< Are components shown in the tree view.
    AttributeVisibilityType attributeVisibility; ///< What types of attributes, if any, are shown in the tree view.
    QLineEdit *searchField; ///< Search field line edit.
    QPushButton *expandAndCollapseButton; ///< Expand/collapse all button.
    QToolButton * undoButton_; ///< Undo button with drop-down menu
    QToolButton * redoButton_; ///< Redo button with drop-down menu
    SortCriteria sortingCriteria;
    QCheckBox *groupCheckBox;
    QCheckBox *componentCheckBox;
    QComboBox *attributeComboBox;
    QTimer addEntitiesTimer_;

    /// @todo 15.09.2013 Profile if unordered_(multi)map would give better performance
    typedef std::map<entity_id_t, EntityItem *> EntityItemIdMap;
    typedef std::map<Entity *, EntityItem *> EntityItemMap;
    typedef std::map<IComponent *, ComponentItem *> ComponentItemMap;
    typedef std::multimap<IAttribute *, AttributeItem *> AttributeItemMap;
    typedef std::pair<AttributeItemMap::iterator, AttributeItemMap::iterator> AttributeItemMapRange;
    typedef std::pair<AttributeItemMap::const_iterator, AttributeItemMap::const_iterator> AttributeItemMapConstRange;
    typedef QHash<QString, EntityGroupItem *> EntityGroupItemMap;
    typedef QList<EntityWeakPtr > EntityWeakPtrList;
    typedef QPair<QTreeWidgetItem*, QTreeWidgetItem*> ParentChildPair;
    
    EntityGroupItemMap entityGroupItems; ///< Entity groups
    EntityItemIdMap entityItemsById;
    EntityItemMap entityItems;
    ComponentItemMap componentItems;
    AttributeItemMap attributeItems;
    EntityWeakPtrList pendingNewEntities;

private slots:
    /// Add new entity to the pending logic. This should only be called for
    /// Entities that are not yet in the tree.
    void AddPendingEntity(EntityPtr &entity);
    void AddPendingEntity(Entity *entity);
    void AddPendingEntities(QList<Entity*> entities);

    /// Entity level main tree manipulator.
    ParentChildPair AddEntity(Entity *entity, bool addToTreeRoot, bool setParent, bool refreshView);

    /// Removes @c entity from pending logic.
    void RemovePendingEntity(Entity *entity);
    void RemovePendingEntity(entity_id_t id);

    /// Returns if @c entity is already in the pending entity list.
    bool IsPendingEntityKnown(const EntityWeakPtr &entity) const;

    /// Process pending new Entities as a batch operation.
    void ProcessPendingNewEntities();

    /// Clears the whole tree widget.
    void Clear();

    /// Adds the item represeting the @c entity to the tree widget.
    void AddEntity(Entity *entity);

    /// Removes item representing @c entity from the tree widget.
    void RemoveEntity(Entity *entity);

    /// Readds entity on server ack.
    /** @param entity Entity to be readded.
        @param oldId Old entity id */
    void AckEntity(Entity *entity, entity_id_t oldId);

    /// Updates temporary state of an entity and its components in the tree widget
    void UpdateEntityTemporaryState(Entity *entity);

    /// Adds the entity to the tree widget, performs EntityItemOfEntity lookup.
    void AddComponent(Entity *entity, IComponent *comp);
    void AddComponent(EntityItem *eItem, Entity *entity, IComponent *comp, bool updateView);

    /// Removes entity from the tree widget, performs EntityItemOfEntity lookup.
    void RemoveComponent(Entity *entity, IComponent *comp);
    /// @note Null entity can be passed if the entity is about to be deleted.
    void RemoveComponent(EntityItem *eItem, Entity *entity, IComponent *comp);

    /// Adds an attribute item item to the tree widget, invoked only by dynamic components' currently.
    void AddDynamicAttribute(IAttribute *attr);

    /// Removes an attribute item from the tree widget.
    void RemoveAttribute(IAttribute *attr);

    /// Updates an attribute item, invoked only by dynamic components' currently.
    void UpdateDynamicAttribute(IAttribute *attr);

    /// Updates entity item's name or group in the tree widget if entity's Name component has changed.
    void UpdateEntityName(IAttribute *attr);

    /// Updates the sender component's name in the tree widget when the component's name changeds.
    void UpdateComponentName();

    /// Handler for entity parent changed signal.
    void EntityParentChanged(Entity* entity);

    /// Updates the parent of an entity (moves it in the tree widget.)
    /** @return New EntityItem parent if was reparented. */
    EntityItem *UpdateEntityParent(Entity* entity, bool movetoRootIfUnparented);

    /// Sort items in the tree widget. The outstanding sort order is used.
    /** @param column Column that is used as the sorting criteria. */
    void Sort(int column);

    /// Searches for items containing @c text (case-insensitive) and toggles their visibility.
    /** If match is found the item is set visible and expanded, otherwise it's hidden.
        @param filter Text used as a filter. */
    void Search(const QString &filter);

    /// Expands or collapses the whole tree view, depending on the previous action.
    void ExpandOrCollapseAll();

    /// Checks the expand status to mark it to the expand/collapse button
    void CheckTreeExpandStatus(QTreeWidgetItem *item);

    /// Removes entity from the tree widget by ID
    void RemoveEntityById(entity_id_t id);

    void SetUndoEnabled(bool canUndo);
    void SetRedoEnabled(bool canRedo);
    void OnSceneRemoved(Scene *);

    void ShowComponentsInternal(bool show) { ShowComponents(show, true); }
    void SetAttributeVisibilityInternal(int mode) { SetAttributeVisibility(static_cast<AttributeVisibilityType>(mode)); }
};
