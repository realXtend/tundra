/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneStructureWindow.h
    @brief  Window with tree view showing every entity in a scene. */

#pragma once

#include "SceneFwd.h"
#include "CoreTypes.h"

#include <QWidget>
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

/// Window with tree view showing every entity in a scene.
/** This class will only handle adding and removing of entities and components and updating
    their names. The SceneTreeWidget implements most of the functionality. */
class SceneStructureWindow : public QWidget
{
    Q_OBJECT
    Q_ENUMS(AttributeVisibility)

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
    void SetScene(const ScenePtr &newScene);
    /// The scene that is shown in the tree view.
    ScenePtr Scene() const { return scene.lock(); }

    enum AttributeVisibilityType
    {
        DoNotShowAttributes, ///< Do not show attributes.
        ShowAssetReferences, ///< Show asset reference attributes (AssetReference and AssetReferenceList).
        ShowDynamicAttributes, ///< Show dynamic attributes.
        ShowAllAttributes ///< Show all attributes.
    };

public slots:
    /// Sets do we want to entity groups in the tree view.
    /** @param show Visibility of entity groups in the tree view. */
    void ShowGroups(bool show);

    /// Sets do we want to show components in the tree view.
    /** @param show Visibility of components in the tree view. */
    void ShowComponents(bool show);

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

protected:
    void changeEvent(QEvent* e); ///< QWidget override.

private:
    /// Populates tree widget with all entities.
    void Populate();

    /// Clears the whole tree widget.
    void Clear();

    /// Creates attribute items for a single entity item.
    void CreateAttributesForItem(ComponentItem *cItem);
    void CreateAttributesForItem(EntityItem *eItem);

    /// Shows (and creates) attribute items depending on the currently set visibility type.
    void ShowAttributes();

    /// Hides all attribute items.
    void HideAttributes();

    /// Create asset reference item to the tree widget.
    /** @param parentItem Parent item, can be entity or component item.
        @param attr AssetReference attribute. */
    void CreateAttributeItem(QTreeWidgetItem *parentItem, IAttribute *attr);

    EntityItem* EntityItemOfEntity(Entity* ent) const;
    /// @note This function does lookup from a different map than EntityItemOfEntity.
    EntityItem* EntityItemById(entity_id_t id) const;
    void RemoveEntityItem(EntityItem* item);
    ComponentItem *ComponentItemOfComponent(IComponent *) const;
    std::vector<AttributeItem *> AttributeItemOfAttribute(IAttribute *) const;
    void SetEntityItemSelected(EntityItem *item, bool selected);

    Framework *framework; ///< Framework.
    SceneWeakPtr scene; ///< Scene which we are showing the in tree widget currently.
    SceneTreeWidget *treeWidget; ///< Scene tree widget.
    bool showGroups; ///< Are entity groups shown in the tree view.
    bool showComponents; ///< Are components shown in the tree view.
    AttributeVisibilityType attributeVisibility; ///< What types of attributes, if any, are shown in the tree view.
    QLineEdit *searchField; ///< Search field line edit.
    QPushButton *expandAndCollapseButton; ///< Expand/collapse all button.
    QToolButton * undoButton_; ///< Undo button with drop-down menu
    QToolButton * redoButton_; ///< Redo button with drop-down menu

    typedef std::map<entity_id_t, EntityItem *> EntityItemIdMap;
    typedef std::map<Entity *, EntityItem *> EntityItemMap;
    typedef std::map<IComponent *, ComponentItem *> ComponentItemMap;
    typedef std::multimap<IAttribute *, AttributeItem *> AttributeItemMap;
    typedef std::pair<AttributeItemMap::iterator, AttributeItemMap::iterator> AttributeItemMapRange;
    typedef std::pair<AttributeItemMap::const_iterator, AttributeItemMap::const_iterator> AttributeItemMapConstRange;
    typedef QHash<QString, EntityGroupItem *> EntityGroupItemMap;
    EntityGroupItemMap entityGroupItems; ///< Entity groups
    EntityItemIdMap entityItemsById;
    EntityItemMap entityItems;
    ComponentItemMap componentItems;
    AttributeItemMap attributeItems;

private slots:
    /// Adds the item represeting the @c entity to the tree widget.
    void AddEntity(Entity *entity);

    /// Removes item representing @c entity from the tree widget.
    void RemoveEntity(Entity *entity);

    /// Readds entity on server ack.
    /** @param entity Entity to be readded.
        @param oldId Old entity id */
    void AckEntity(Entity *entity, entity_id_t oldId);

    /// Updates temporary state of an entity and its components in the tree widget
    /** @param entity The entity which temporary state was toggled */
    void UpdateEntityTemporaryState(Entity *entity);

    /// Adds the entity to the tree widget.
    /** @param entity Altered entity.
        @param comp Component which was added. */
    void AddComponent(Entity *entity, IComponent *comp);

    /// Removes entity from the tree widget.
    /** @param entity Altered entity.
        @param comp Component which was removed. */
    void RemoveComponent(Entity *entity, IComponent *comp);

    /// Adds an attribute item item to the tree widget.
    /** This is called only by EC_DynamicComponent when asset ref attribute is added to it.
        @param attr AssetReference attribute. */
    void AddDynamicAttribute(IAttribute *attr);

    /// Removes an attribute item from the tree widget.
    /** This is called only by EC_DynamicComponent when asset ref attribute is removed from it.
        @param attr AssetReference attribute. */
    void RemoveAttribute(IAttribute *attr);

    /// Updates an attribute item.
    /** @param attr AssetReference attribute. */
    void UpdateDynamicAttribute(IAttribute *attr);

    /// Updates entity's name in the tree widget if entity's EC_Name component's "name" attribute has changed.
    /** EC_Name component's AttributeChanged() signal is connected to this slot.
        @param attr EC_Name's attribute which was changed. */
    void UpdateEntityName(IAttribute *attr);

    /// Updates the sender component's name in the tree widget when the component's name changeds.
    void UpdateComponentName();

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
    void SetAttributeVisibilityInternal(int mode) { SetAttributeVisibility(static_cast<AttributeVisibilityType>(mode)); }
};
