/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   SceneTreeWidgetItems.h
    @brief  Tree widget -related classes used in @c SceneTreeWidget and @c AssetTreeWidget. */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SceneTreeWidgetItems.h"

#include "Entity.h"
#include "AssetReference.h"
#include "IAsset.h"
#include "IAssetBundle.h"
#include "IAssetStorage.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

// EntityItem

EntityItem::EntityItem(const EntityPtr &entity, EntityGroupItem *parent) :
    ptr(entity), id(entity->Id()), parentItem(parent)
{
    if (parentItem)
    {
        parentItem->numberOfEntities++;
        parentItem->UpdateText();
    }

    SetText(entity.get());
}

EntityItem::~EntityItem()
{
    if (parentItem)
    {
        parentItem->numberOfEntities--;
        parentItem->UpdateText();
    }
}

void EntityItem::SetText(::Entity *entity)
{
    if (ptr.lock().get() != entity)
        LogWarning("EntityItem::SetText: the entity given is different than the entity this item represents.");

    QString name = QString("%1 %2").arg(entity->Id()).arg(entity->Name().isEmpty() ? "(no name)" : entity->Name());

    setTextColor(0, QColor(Qt::black));
    
    const bool local = entity->IsLocal();
    const bool temp = entity->IsTemporary();

    QString info;
    if (local)
    {
        setTextColor(0, QColor(Qt::blue));
        info.append("Local");
    }

    if (temp)
    {
        setTextColor(0, QColor(Qt::red));
        if (!info.isEmpty())
            info.append(" ");
        info.append("Temporary");
    }

    if (!info.isEmpty())
    {
        info.prepend(" [");
        info.append("]");
        setText(0, name + info);
    }
    else
        setText(0, name);
}

EntityGroupItem *EntityItem::Parent() const
{
    return parentItem;
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

EntityGroupItem::EntityGroupItem(const QString &groupName) :
    name(groupName),
    numberOfEntities(0)
{
    UpdateText();
}

void EntityGroupItem::UpdateText()
{
    QString text = QString("[ group: %1 (%2 item%3) ]").arg(name).arg(numberOfEntities).arg(numberOfEntities > 1 ? "s" : "");
    setText(0, text);
}

// ComponentItem

ComponentItem::ComponentItem(const ComponentPtr &comp, EntityItem *parent) :
    QTreeWidgetItem(parent),
    parentItem(parent),
    ptr(comp),
    typeId(comp->TypeId()),
    typeName(comp->TypeName()),
    name(comp->Name())
{
    SetText(comp.get());
}

void ComponentItem::SetText(IComponent *comp)
{
    if (ptr.lock().get() != comp)
        LogWarning("ComponentItem::SetText: the component given is different than the component this item represents.");

    QString compType = IComponent::EnsureTypeNameWithoutPrefix(comp->TypeName());
    QString name = QString("%1 %2").arg(compType).arg(comp->Name());

    setTextColor(0, QColor(Qt::black));

    QString localText = QApplication::translate("ComponentItem", "Local");
    QString temporaryText = QApplication::translate("ComponentItem", "Temporary");
    QString localOnlyText = QApplication::translate("ComponentItem", "UpdateMode:LocalOnly");
    QString disconnectedText = QApplication::translate("ComponentItem", "UpdateMode:Disconnected");

    const bool sync = comp->IsReplicated();
    const bool temporary = comp->IsTemporary();

    QString info;
    if (!sync)
    {
        setTextColor(0, QColor(Qt::blue));
        info.append(localText);
    }

    if (temporary)
    {
        setTextColor(0, QColor(Qt::red));
        if (!info.isEmpty())
            info.append(" ");
        info.append(temporaryText);
    }

    if (comp->UpdateMode() == AttributeChange::LocalOnly)
    {
        if (!info.isEmpty())
            info.append(" ");
        info.append(localOnlyText);
    }

    if (comp->UpdateMode() == AttributeChange::Disconnected)
    {
        if (!info.isEmpty())
            info.append(" ");
        info.append(disconnectedText);
    }

    if (!info.isEmpty())
    {
        info.prepend(" (");
        info.append(")");
        setText(0, name + info);
    }
    else
        setText(0, name);
}

ComponentPtr ComponentItem::Component() const
{
    return ptr.lock();
}

EntityItem *ComponentItem::Parent() const
{
    return parentItem;
}

// AttributeItem

AttributeItem::AttributeItem(IAttribute *attr, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    ptr(attr->Owner()->shared_from_this(), attr)
{
    Update(attr);
}

void AttributeItem::Update(IAttribute *attr)
{
    if (attr != ptr.Get())
    {
        LogError("AttributeItem::Update: trying to update item with wrong attribute.");
        return;
    }

    type = attr->TypeName();
    name = attr->Name();
    id = attr->Id();
    value = attr->ToString().c_str();

    setText(0, QString("%1: %2").arg(id).arg(value));
}

// AssetRefItem

AssetRefItem::AssetRefItem(IAttribute *attr, QTreeWidgetItem *parent) :
    AttributeItem(attr, parent)
{
}

AssetRefItem::AssetRefItem(IAttribute *attr, const QString &assetRef, QTreeWidgetItem *parent) :
    AttributeItem(attr, parent)
{
    // Override the regular AssetReferenceList value "ref1;ref2;etc." with a single ref.
    value = assetRef;
    setText(0, QString("%1: %2").arg(id).arg(value));
}

// SceneTreeWidgetSelection

bool SceneTreeWidgetSelection::IsEmpty() const
{
    return groups.isEmpty() && entities.isEmpty() && components.isEmpty() && assets.isEmpty();
}

bool SceneTreeWidgetSelection::HasGroups() const
{
    return !groups.isEmpty();
}

bool SceneTreeWidgetSelection::HasGroupsOnly() const
{
    return !groups.isEmpty() && entities.isEmpty() && components.isEmpty() && assets.isEmpty();
}

bool SceneTreeWidgetSelection::HasEntities() const
{
    return !entities.isEmpty();
}

bool SceneTreeWidgetSelection::HasEntitiesOnly() const
{
    return groups.isEmpty() && !entities.isEmpty() && components.isEmpty() && assets.isEmpty();
}

bool SceneTreeWidgetSelection::HasComponents() const
{
    return !components.isEmpty();
}

bool SceneTreeWidgetSelection::HasComponentsOnly() const
{
    return groups.isEmpty() && entities.isEmpty() && !components.isEmpty() && assets.isEmpty();
}

bool SceneTreeWidgetSelection::HasAssets() const
{
    return !assets.isEmpty();
}

bool SceneTreeWidgetSelection::HasAssetsOnly() const
{
    return groups.isEmpty() && entities.isEmpty() && components.isEmpty() && !assets.isEmpty();
}

QList<entity_id_t> SceneTreeWidgetSelection::EntityIds() const
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
    SetText(asset.get());
}

AssetPtr AssetItem::Asset() const
{
    return assetPtr.lock();
}

void AssetItem::SetText(IAsset *asset)
{
    if (assetPtr.lock().get() != asset)
        LogWarning("AssetItem::SetText: the asset given is different than the asset this item represents.");

    QString name;
    QString subAssetName;
    AssetAPI::ParseAssetRef(asset->Name(), 0, 0, 0, 0, &name, 0, 0, &subAssetName);

    if (!subAssetName.isEmpty())
        name = subAssetName;
    else
    {
        // Find the topmost item.
        QTreeWidgetItem *p = parent();
        while (p != 0 && p->parent() != 0)
            p = p->parent();

        // If this item is in a storage item, trim away the base url to leave only the relative path from it.
        AssetStorageItem *storageItem = dynamic_cast<AssetStorageItem*>(p);
        if (storageItem && storageItem->Storage().get())
        {
            QString storageUrl = storageItem->Storage()->BaseURL();
            if (asset->Name().startsWith(storageUrl))
                name = asset->Name().right(asset->Name().length()-storageUrl.length());
            else
                name = asset->Name(); // Use full url ref as this asset does not belong to the top level parent storage.
        }
        else if (!subAssetName.isEmpty())
            name = subAssetName;
    }

    // "File missing" red
    // "No disk source" red
    // "Read-only" 
    // "Memory-only" red
    // "Unloaded " gray

    QString unloadedText = QApplication::translate("AssetItem", "Unloaded");
    QString fileMissingText = QApplication::translate("AssetItem", "File missing");
    QString noDiskSourceText = QApplication::translate("AssetItem", "No disk source");
    //QString readOnlyText = QApplication::translate("AssetItem", "Read-only");
    QString memoryOnlyText = QApplication::translate("AssetItem", "Memory-only");

    bool unloaded = !asset->IsLoaded();
    bool fileMissing = !asset->DiskSource().isEmpty() && asset->DiskSourceType() == IAsset::Original && !QFile::exists(asset->DiskSource());
    bool memoryOnly = asset->DiskSource().isEmpty() && !asset->GetAssetStorage() && asset->DiskSourceType() == IAsset::Programmatic;
    bool diskSourceMissing = asset->DiskSource().isEmpty();
    bool isModified = asset->IsModified();

    ///\todo Enable when source type is set properly for AssetCreated signal (see the bug in AssetAPI::CreateNewAsset).
//    if (!asset->DiskSource().isEmpty() && asset->DiskSourceType() == IAsset::Programmatic)
//        LogWarning("AssetItem::SetText: Encountered asset (" + asset->Name() +
//            ") which is programmatic but has also disk source " + asset->DiskSource().isEmpty() + ".");

/*
    LogInfo(QString("unloaded")+(unloaded?"1":"0"));
    LogInfo(QString("fileMissing")+(fileMissing?"1":"0"));
    LogInfo(QString("memoryOnly")+(memoryOnly?"1":"0"));
    LogInfo(QString("diskSourceMissing:")+(diskSourceMissing?"1":"0"));
    LogInfo(QString("isModified:")+(isModified?"1":"0"));
*/
    QString info;
    if (fileMissing)
    {
        setTextColor(0, QColor(Qt::red));
        info.append(fileMissingText);
    }
    if (!memoryOnly && diskSourceMissing)
    {
        setTextColor(0, QColor(Qt::red));
        if (!info.isEmpty())
            info.append(" ");
        info.append(noDiskSourceText);
    }
    if (!memoryOnly && unloaded)
    {
        setTextColor(0, QColor(Qt::gray));
        if (!info.isEmpty())
            info.append(" ");
        info.append(unloadedText);
    }
    if (memoryOnly)
    {
        setTextColor(0, QColor(Qt::darkCyan));
        if (!info.isEmpty())
            info.append(" ");
        info.append(memoryOnlyText);
    }

    if (isModified)
        name.append("*");
    if (!info.isEmpty())
    {
        info.prepend(" (");
        info.append(")");
        setText(0, name + info);
    }
    else
    {
        setTextColor(0, QColor(Qt::black));
        setText(0, name);
    }
}

// AssetStorageItem

AssetStorageItem::AssetStorageItem(const AssetStoragePtr &storage, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    assetStorage(storage)
{
    setText(0, storage->ToString() + (!storage->Writable() ?  QApplication::translate("AssetStorageItem", " (Read-only)") : QString()));
}

AssetStoragePtr AssetStorageItem::Storage() const
{
    return assetStorage.lock();
}

// AssetBundleItem

AssetBundleItem::AssetBundleItem(const AssetBundlePtr &bundle, QTreeWidgetItem *parent) :
    QTreeWidgetItem(parent),
    assetBundle(bundle)
{
    QString name;
    AssetAPI::ParseAssetRef(bundle->Name(), 0, 0, 0, 0, &name);

    // Find the topmost item.
    QTreeWidgetItem *p = this->parent();
    while (p != 0 && p->parent() != 0)
        p = p->parent();

    // If this item is in a storage item, trim away the base url to leave only the relative path from it.
    AssetStorageItem *storageItem = dynamic_cast<AssetStorageItem*>(p);
    if (storageItem && storageItem->Storage().get())
    {
        QString storageUrl = storageItem->Storage()->BaseURL();
        if (bundle->Name().startsWith(storageUrl))
            name = bundle->Name().right(bundle->Name().length()-storageUrl.length());
    }

    int subAssetCount = bundle->SubAssetCount();
    if (subAssetCount != -1)
        name = QString("%1 (%2 assets)").arg(name).arg(subAssetCount);
    setText(0, name);
}

bool AssetBundleItem::Contains(const QString &assetRef) const
{
    // We could also query the bundle for this, but for some bundle types this might take a lot
    // of time. So lets do a starts with string check. This should not produce misses if AssetAPI has
    // parsed the asset ref correctly to the bundle and the asset itself.
    return (!assetBundle.expired() && assetRef.startsWith(assetBundle.lock()->Name(), Qt::CaseInsensitive));
}

AssetBundlePtr AssetBundleItem::AssetBundle() const
{
    return assetBundle.lock();
}

AssetStoragePtr AssetBundleItem::Storage() const
{
    if (assetBundle.expired())
        return AssetStoragePtr();
    return assetBundle.lock()->AssetStorage();
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
