/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeWidgetItems.h
 *  @brief  Tree widget utility classes and functions used in the @c SceneTreeWidget and @c AssetTreeWidget.
 */

#ifndef incl_SceneStructureModule_SceneTreeWidgetItems_h
#define incl_SceneStructureModule_SceneTreeWidgetItems_h

#include "SceneFwd.h"

/// Tree widget item representing an entity.
class EntityItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param entity Entity which the item represents.
    */
    explicit EntityItem(const EntityPtr &entity);

    /// Sets the item text accordingly to the entity information.
    /** @param entity Entity which the item represents.
    */
    void SetText(Scene::Entity *entity);

    /// Returns pointer to the entity this item represents.
    EntityPtr Entity() const;

    /// Return Entity ID of the entity associated with this tree widget item.
    entity_id_t Id() const;

    /// QTreeWidgetItem override.
    /** If treeWidget::sortColumn() is 0, items are sorted by ID,
        or if it's 1, items are sorted by name (if applicable).
    */
    bool operator <(const QTreeWidgetItem &rhs) const;

private:
    entity_id_t id; ///< Entity ID associated with this tree widget item.
    Scene::EntityWeakPtr ptr; ///< Weak pointer to the component this item represents.
};

/// Tree widget item representing a component.
class ComponentItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param comp Component which the item represents.
        @param parent Parent entity item.
    */
    ComponentItem(const ComponentPtr &comp, EntityItem *parent);

    /// Sets the item text accordingly to the component information.
    /** @param comp Component which the item represents.
    */
    void SetText(IComponent *comp);

    /// Returns pointer to the entity this item represents.
    ComponentPtr Component() const;

    /// Returns the parent entity item.
    EntityItem *Parent() const;

    QString typeName; ///< Type name.
    QString name; ///< Name, if applicable.

private:
    ComponentWeakPtr ptr; ///< Weak pointer to the component this item represents.
    EntityItem *parentItem; ///< Parent entity item.
};

/// Tree widget item representing an asset reference.
class AssetRefItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param attr Asset reference attribute.
    */
    AssetRefItem(IAttribute *attr, QTreeWidgetItem *parent = 0);
    AssetRefItem(const QString &name, const QString &ref, QTreeWidgetItem *parent = 0);

    /// Sets the item text accordingly to the attribute information.
    /** @param attr Asset reference attribute.
    */
    void SetText(IAttribute *attr);

    QString name; ///< Name of the attribute.
    QString id; ///< ID.
    QString type; ///< Type. ///<\todo Remove
};

/// Represents selection of selected scene tree widget items.
struct Selection
{
    /// Returns true if no entity or component items selected.
    bool IsEmpty() const;

    /// Returns true if selection contains entities;
    bool HasEntities() const;

    /// Returns true if selected contains components.
    bool HasComponents() const;

    /// Returns true if selection contains assets.
    bool HasAssets() const;

    /// Returns list containing unique entity ID's of both selected entities and parent entities of selected components
    QList<entity_id_t> EntityIds() const;

    QList<EntityItem *> entities; ///< List of selected entities.
    QList<ComponentItem *> components; ///< List of selected components.
    QList<AssetRefItem *> assets; ///< List of selected asset refs
};

#endif
