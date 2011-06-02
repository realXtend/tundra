/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneTreeWidgetItems.h
 *  @brief  Tree widget -related classes used in @c SceneTreeWidget and @c AssetTreeWidget.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneTreeWidgetItems.h"

#include "Entity.h"
#include "AssetReference.h"
#include "IAsset.h"
#include "IAssetStorage.h"

#include "MemoryLeakCheck.h"

// EntityItem

EntityItem::EntityItem(const EntityPtr &entity) :
    ptr(entity), id(entity->Id())
{
    SetText(entity.get());
}

void EntityItem::SetText(::Entity *entity)
{
    setText(0, QString("%1 %2").arg(entity->Id()).arg(entity->GetName()));
}

EntityPtr EntityItem::Entity() const
{
    return ptr.lock();
}

entity_id_t EntityItem::Id() const
{
    return id;
}

bool EntityItem::operator <(const QTreeWidgetItem &rhs) const
{
    int c = treeWidget()->sortColumn();
    if (c == 0)
        return (entity_id_t)text(0).split(" ")[0].toInt() < (entity_id_t)rhs.text(0).split(" ")[0].toInt();
    else if (c == 1)
    {
        QStringList lhsText = text(0).split(" ");
        QStringList rhsText = rhs.text(0).split(" ");
        if (lhsText.size() > 1 && rhsText.size() > 1)
            return lhsText[1].toLower() < rhsText[1].toLower();
        else
            return false;
    }
    else
        return QTreeWidgetItem::operator <(rhs);
}

// ComponentItem

ComponentItem::ComponentItem(const ComponentPtr &comp, EntityItem *parent) :
    QTreeWidgetItem(parent), parentItem(parent), ptr(comp), typeName(comp->TypeName()), name(comp->Name())
{
    SetText(comp.get());
}

void ComponentItem::SetText(IComponent *comp)
{
    setText(0, QString("%1 %2").arg(comp->TypeName()).arg(comp->Name()));
}

ComponentPtr ComponentItem::Component() const
{
    return ptr.lock();
}

EntityItem *ComponentItem::Parent() const
{
    return parentItem;
}

// AssetRefItem

AssetRefItem::AssetRefItem(IAttribute *attr, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent)
{
    Attribute<AssetReference> *assetRef = dynamic_cast<Attribute<AssetReference> *>(attr);
    assert(assetRef);
    this->name = assetRef->Name();
    id = assetRef->Get().ref;
    SetText(assetRef);
}

AssetRefItem::AssetRefItem(const QString &name, const QString &ref, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent)
{
    this->name = name;
    id = ref;
    setText(0, QString("%1: %2").arg(name).arg(ref));
}

void AssetRefItem::SetText(IAttribute *attr)
{
    Attribute<AssetReference> *assetRef = dynamic_cast<Attribute<AssetReference> *>(attr);
    assert(assetRef);
    if (assetRef)
        setText(0, QString("%1: %2").arg(assetRef->Name()).arg(assetRef->Get().ref));
}

// Selection

bool Selection::IsEmpty() const
{
    return entities.isEmpty() && components.isEmpty() && assets.isEmpty();
}

bool Selection::HasEntities() const
{
    return !entities.isEmpty();
}

bool Selection::HasComponents() const
{
    return !components.isEmpty();
}

bool Selection::HasAssets() const
{
    return !assets.isEmpty();
}

QList<entity_id_t> Selection::EntityIds() const
{
    QSet<entity_id_t> ids;
    foreach(EntityItem *e, entities)
        ids.insert(e->Id());
    foreach(ComponentItem *c, components)
        ids.insert(c->Parent()->Id());

    return ids.toList();
}

// AssetItem

AssetItem::AssetItem(const AssetPtr &asset, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    assetPtr(asset)
{
    setText(0, asset->Name());
    MarkUnloaded(!asset->IsLoaded());
}

AssetPtr AssetItem::Asset() const
{
    return assetPtr.lock();
}

void AssetItem::MarkUnloaded(bool value)
{
    QString unloaded = QApplication::translate("AssetItem", " (Unloaded)");
    if (value)
        setText(0, text(0) + unloaded);
    else
        setText(0, text(0).remove(unloaded));
}

// AssetStorageItem

AssetStorageItem::AssetStorageItem(const AssetStoragePtr &storage, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    assetStorage(storage)
{
    setText(0, storage->ToString());//Name());
}

AssetStoragePtr AssetStorageItem::Storage() const
{
    return assetStorage.lock();
}

// AssetSelection

bool AssetTreeWidgetSelection::IsEmpty() const
{
    return assets.isEmpty() && storages.isEmpty();
}

bool AssetTreeWidgetSelection::HasAssets() const
{
    return !assets.isEmpty();
}

bool AssetTreeWidgetSelection::HasStorages() const
{
    return !storages.isEmpty();
}
