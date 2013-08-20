/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneTreeWidgetItems.h
    @brief  Tree widget -related classes used in @c SceneTreeWidget and @c AssetTreeWidget. */

#pragma once

#include "SceneFwd.h"
#include "AssetFwd.h"
#include "IAttribute.h"

class EntityGroupItem : public QTreeWidgetItem
{
public:
    explicit EntityGroupItem(const QString &groupName);
    QString GroupName() const { return name; }
    void UpdateText();
    
    int numberOfEntities;
private:
    QString name;
};

/// Tree widget item representing an entity.
class EntityItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param entity Entity which the item represents. */
    explicit EntityItem(const EntityPtr &entity, EntityGroupItem *parent = 0);

    /// Destructor
    ~EntityItem();

    /// Decorates the item (text + color) accordingly to the entity information.
    /** @param entity Entity which the item represents. */
    void SetText(::Entity *entity);

    /// Parent entity group item, if the entity is assigned to a group
    EntityGroupItem* Parent() const;

    /// Returns pointer to the entity this item represents.
    EntityPtr Entity() const;

    /// Return Entity ID of the entity associated with this tree widget item.
    entity_id_t Id() const;

    /// QTreeWidgetItem override.
    /** If treeWidget::sortColumn() is 0, items are sorted by ID, or if it's 1, items are sorted by name (if applicable). */
    bool operator <(const QTreeWidgetItem &rhs) const;

private:
    entity_id_t id; ///< Entity ID associated with this tree widget item.
    EntityWeakPtr ptr; ///< Weak pointer to the component this item represents.
    EntityGroupItem *parentItem;
};

/// Tree widget item representing a component.
class ComponentItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param comp Component which the item represents.
        @param parent Parent entity item. */
    ComponentItem(const ComponentPtr &comp, EntityItem *parent);

    /// Decorates the item (text + color) accordingly to the component information.
    /** @param comp Component which the item represents. */
    void SetText(IComponent *comp);

    /// Returns pointer to the entity this item represents.
    ComponentPtr Component() const;

    /// Returns the parent entity item.
    EntityItem *Parent() const;

    u32 typeId; ///< Type ID.
    QString typeName; ///< Type name.
    QString name; ///< Name, if applicable.

private:
    ComponentWeakPtr ptr; ///< Weak pointer to the component this item represents.
    EntityItem *parentItem; ///< Parent entity item.
};

/// Tree widget item representing an asset reference.
class AttributeItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param attr Asset reference attribute.
        @param parent Parent item. */
    AttributeItem(IAttribute *attr, QTreeWidgetItem *parent = 0);

    /// Updates the item's text accordingly to the current attribute information.
    void Update(IAttribute *attr);

    AttributeWeakPtr ptr;
    QString type; ///< Type name.
    QString id; ///< ID.
    QString name; ///< Name.
    QString value; ///< Value.
};

/// Tree widget item representing an asset reference attribute.
class AssetRefItem : public AttributeItem
{
public:
    /// Constructor.
    /** @param attr AssetReference or AssetReferenceList attribute.
        @param parent Parent item. */
    AssetRefItem(IAttribute *attr, QTreeWidgetItem *parent = 0);

    /// Constructor for creating individual items for AssetReferenceList.
    /** @param assetRef Asset reference. */
    AssetRefItem(IAttribute *attr, const QString &assetRef, QTreeWidgetItem *parent = 0);
};

/// Represents selection of SceneTreeWidget items.
struct SceneTreeWidgetSelection
{
    /// Returns true if no entity or component items selected.
    bool IsEmpty() const;

    /// Returns true if selection contains entity groups.
    bool HasGroups() const;

    /// Returns true if selection contains entity groups only.
    bool HasGroupsOnly() const;

    /// Returns true if selection contains entities;
    bool HasEntities() const;

    /// Returns true if selection contains entities only;
    bool HasEntitiesOnly() const;

    /// Returns true if selected contains components.
    bool HasComponents() const;
    
    /// Returns true if selected contains components only.
    bool HasComponentsOnly() const;

    /// Returns true if selection contains assets.
    bool HasAssets() const;

    /// Returns true if selection contains assets only.
    bool HasAssetsOnly() const;

    /// Returns list containing unique entity IDs of both selected entities and parent entities of selected components
    QList<entity_id_t> EntityIds() const;

    QList<EntityGroupItem *> groups; ///< List of selected entity groups.
    QList<EntityItem *> entities; ///< List of selected entities.
    QList<ComponentItem *> components; ///< List of selected components.
    QList<AssetRefItem *> assets; ///< List of selected asset refs
};

/// Item representing asset in the tree widget.
class AssetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param asset Asset pointer.
        @param parent Parent tree widget item. */
    AssetItem(const AssetPtr &asset, QTreeWidgetItem *parent = 0);

    /// Returns shared pointer to the asset. Always remember to check that the pointer is not null.
    AssetPtr Asset() const;

    /// Appends information text to the item that the asset is unloaded.
    /** @param value. If true, "(Unloaded)" is appended to the item text. If false, the aforementiond text is removed. */
    //void MarkUnloaded(bool value);

    /// Decorates the item (text + color) accordingly to the asset properties.
    /** @param asset Asset which this item represents. */
    void SetText(IAsset *asset);

private:
    AssetWeakPtr assetPtr; ///< Weak pointer to the asset.
};

/// Item representing asset storage in the tree widget.
class AssetStorageItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param storage Asset storage pointer.
        @param parent Parent tree widget item. */
    AssetStorageItem(const AssetStoragePtr &storage, QTreeWidgetItem *parent = 0);

    /// Returns shared pointer to the asset. Always remember to check that the pointer is not null.
    AssetStoragePtr Storage() const;

private:
    AssetStorageWeakPtr assetStorage; ///< Weak pointer to the asset storage.
};

/// Item representing asset bundle in the tree widget.
class AssetBundleItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param bundle Asset bundle.
        @param parent Parent tree widget item. */
    AssetBundleItem(const AssetBundlePtr &bundle, QTreeWidgetItem *parent = 0);

    bool Contains(const QString &assetRef) const;

    /// Return shared ptr to the asset bundle. Always remember to check that the pointer is not null.
    AssetBundlePtr AssetBundle() const;

    /// Returns shared pointer to the asset. Always remember to check that the pointer is not null.
    AssetStoragePtr Storage() const;

private:
    AssetBundleWeakPtr assetBundle; ///< Weak pointer to the asset bundle.
};

/// Represents selection of AssetTreeWidget items.
struct AssetTreeWidgetSelection
{
    /// Returns true if no entity or component items selected.
    bool IsEmpty() const;

    /// Returns true if selection contains entities;
    bool HasAssets() const;

    /// Returns true if selected contains components.
    bool HasStorages() const;

    QList<AssetItem *> assets; ///< List of selected assets.
    QList<AssetStorageItem *> storages; ///< List of selected asset storages.
};
