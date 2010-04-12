/**
 * For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file InventoryFolder.cpp
 *  @brief  A class representing folder in the inventory item tre model.
 */

#include "StableHeaders.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"
#include "RexUUID.h"

namespace Inventory
{

InventoryFolder::InventoryFolder(const QString &id, const QString &name, InventoryFolder *parent, const bool editable) :
    AbstractInventoryItem(id, name, parent, editable), itemType_(AbstractInventoryItem::Type_Folder), dirty_(false),
    libraryItem_(false)
{
}

// virtual
InventoryFolder::~InventoryFolder()
{
    qDeleteAll(children_);
}

AbstractInventoryItem *InventoryFolder::AddChild(AbstractInventoryItem *child)
{
    child->SetParent(this);
    children_.append(child);
    return children_.back();
}

bool InventoryFolder::RemoveChildren(int position, int count)
{
    if (position < 0 || position + count > children_.size())
        return false;

    for(int row = 0; row < count; ++row)
        delete children_.takeAt(position);

    return true;
}

/*
void InventoryFolder::DeleteChild(InventoryItemBase *child)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if (item == child)
            SAFE_DELETE(item);
    }
}

void InventoryFolder::DeleteChild(const RexUUID &id)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        if (item->GetID() == id)
            SAFE_DELETE(item);
    }
}

const bool InventoryFolder::IsChild(InventoryFolder *child)
{
    QListIterator<InventoryItemBase *> it(childItems_);
    while(it.hasNext())
    {
        InventoryItemBase *item = it.next();
        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
        if (folder)
        {
            if (folder == child)
                return true;

            if (folder->IsChild(child))
                return true;
        }
    }

    return false;
}
*/

InventoryFolder *InventoryFolder::GetFirstChildFolderByName(const QString &searchName) const
{
    if (GetName() == searchName)
        return const_cast<InventoryFolder *>(this);

    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetItemType() == Type_Folder)
            folder = static_cast<InventoryFolder *>(item);
        else
            continue;

        if (folder->GetName() == searchName)
            return folder;

        InventoryFolder *folder2 = folder->GetFirstChildFolderByName(searchName);
        if (folder2)
            if (folder2->GetName() == searchName)
                return folder2;
    }

    return 0;
}

InventoryFolder *InventoryFolder::GetChildFolderById(const QString &searchId) const
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryFolder *folder = 0;
        if (item->GetItemType() == Type_Folder)
            folder = static_cast<InventoryFolder *>(item);
        else
            continue;

        if (folder->GetID() == searchId)
            return folder;

        InventoryFolder *folder2 = folder->GetChildFolderById(searchId);
        if (folder2)
            if (folder2->GetID() == searchId)
                return folder2;
    }

    return 0;
}

InventoryAsset *InventoryFolder::GetChildAssetById(const QString &searchId) const
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(it.next());
        if (!asset)
            continue;

        if (asset->GetID() == searchId)
            return asset;

    ///\todo Recursion, if needed.
/*
        InventoryFolder *folder = folder->GetChildAssetById(searchId);
        if (folder)
            if (asset->GetID() == searchId)
                return folder2;
*/
    }

    return 0;
}

AbstractInventoryItem *InventoryFolder::GetChildById(const QString &searchId) const
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        assert(item);
        if (item->GetID() == searchId)
            return item;

        if (item->GetItemType() == Type_Folder)
        {
            InventoryFolder *folder = checked_static_cast<InventoryFolder *>(item);

            AbstractInventoryItem *item2 = folder->GetChildById(searchId);
            if (item2)
                return item2;
        }
    }

    return 0;
}

InventoryAsset *InventoryFolder::GetFirstAssetByAssetId(const QString &id) const
{
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(item);
        if (asset && asset->GetAssetReference() == id)
            return asset;

        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
        assert(folder);
        if (!folder)
            return 0;

        asset = folder->GetFirstAssetByAssetId(id);
        if (asset && asset->GetAssetReference() == id)
            return asset;
    }

    return 0;
}

QList<const InventoryAsset *> InventoryFolder::GetChildAssetsByAssetType(const asset_type_t type) const
{
    QList<const InventoryAsset *> list;
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(item);
        if (asset && asset->GetAssetType() == type)
        {
            list.push_back(asset);
            continue;
        }

        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
        if (!folder)
            continue;

        list.append(folder->GetChildAssetsByAssetType(type));
    }

    return list;
}

QList<const InventoryAsset *> InventoryFolder::GetChildAssetsByInventoryType(const inventory_type_t type) const
{
    QList<const InventoryAsset *> list;
    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        AbstractInventoryItem *item = it.next();
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(item);
        if (asset && asset->GetInventoryType() == type)
        {
            list.push_back(asset);
            continue;
        }

        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
        if (!folder)
            continue;

        list.append(folder->GetChildAssetsByInventoryType(type));
    }

    return list;
}

bool InventoryFolder::IsDescendentOf(AbstractInventoryItem *searchFolder) const
{
    forever
    {
        AbstractInventoryItem *parent = GetParent();
        if (parent)
        {
            if (parent == searchFolder)
                return true;
            else
                return parent->IsDescendentOf(searchFolder);
        }

        return false;
    }
}

AbstractInventoryItem *InventoryFolder::Child(int row) const
{
    if (row < 0 || row > children_.size() - 1)
        return 0;

    return children_.value(row);
}

int InventoryFolder::Row() const
{
    if (GetParent())
    {
        InventoryFolder *folder = static_cast<InventoryFolder *>(GetParent());
        return folder->children_.indexOf(const_cast<InventoryFolder *>(this));
    }

    return 0;
}

#ifdef _DEBUG
void InventoryFolder::DebugDumpInventoryFolderStructure(int indentationLevel)
{
    for(int i = 0; i < indentationLevel; ++i)
        std::cout << " ";
    std::cout << GetName().toStdString() << " " << std::endl;

    QListIterator<AbstractInventoryItem *> it(children_);
    while(it.hasNext())
    {
        InventoryFolder *folder = dynamic_cast<InventoryFolder *>(it.next());
        if (folder)
            folder->DebugDumpInventoryFolderStructure(indentationLevel + 3);
    }
}
#endif

}
