// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Inventory.h"

InventoryItem::InventoryItem(
    const RexUUID &inventory_id,
    const RexUUID &asset_id,
    asset_type_t asset_type)
    :
    inventoryID_(inventory_id),
    assetID_(asset_id),
    assetType_(asset_type),
    inventoryType_(GetInventoryTypeFromAssetType(asset_type))
{
}

RexUUID InventoryFolder ::GetParentID() const
{
    if(!parent_)
        return RexUUID();

    return parent_->GetID();
}

InventoryFolder *InventoryFolder::GetFirstSubFolderByName(const char *searchName)
{
    if (name_ == searchName)
        return this;
    for(FolderIter iter = children_.begin(); iter != children_.end(); ++iter)
    {
        InventoryFolder *folder = iter->GetFirstSubFolderByName(searchName);
        if (folder)
            return folder;
    }

    return 0;
}

InventoryFolder *InventoryFolder::GetFirstSubFolderByID(const RexUUID &searchId)
{
    if (id_ == searchId)
        return this;

    for(FolderIter iter = children_.begin(); iter != children_.end(); ++iter)
    {
        InventoryFolder *folder = iter->GetFirstSubFolderByID(id_);
        if (folder)
            return folder;
    }

    return 0;
}

InventoryFolder *InventoryFolder::AddSubFolder(const InventoryFolder &folder)
{
    children_.push_back(folder);
    children_.back().parent_ = this;
    return &children_.back();
}

void InventoryFolder::AddItem(const InventoryItem &item)
{
    if (item.GetAssetID().IsNull())
    {
        std::cout << "Inventory item's asset ID is null. Can't add new item to inventory." << std::endl;
        return;
    }

    if (item.GetInventoryID().IsNull())
    {
        std::cout << "Inventory item's asset ID is null. Can't add new item to inventory." << std::endl;
        return;
    }

    for(ItemIter it = items_.begin(); it != items_.end(); ++ it)
    {
        if(it->GetInventoryID() == item.GetInventoryID())
        {
            std::cout << "Item with the same inventory ID already exists.Can't add new item to inventory." << std::endl;
            return;
        }
    }

    items_.push_back(item);
}

InventoryItem *InventoryFolder::GetItemByInventoryID(const RexUUID &inventory_id)
{
    for(ItemIter it = items_.begin(); it != items_.end(); ++ it)
    {
        if (it->GetInventoryID() == inventory_id)
            return &(*it);
    }

    return 0;
}

InventoryItem *InventoryFolder::GetFirstItemByAssetID(const RexUUID &asset_id)
{
    for(ItemIter it = items_.begin(); it != items_.end(); ++ it)
    {
        if (it->GetAssetID() == asset_id)
            return &(*it);
    }

    return 0;
}

InventoryItem *InventoryFolder::GetItemByBothIDs(const RexUUID &asset_id, const RexUUID &inventory_id)
{
    for(ItemIter it = items_.begin(); it != items_.end(); ++ it)
    {
        if (it->GetInventoryID() == inventory_id && it->GetAssetID() == asset_id )
            return &(*it);
    }

    return 0;
}

#ifdef _DEBUG
void InventoryFolder::DebugDumpInventoryFolderStructure(int indentationLevel)
{
    for(int i = 0; i < indentationLevel; ++i)
        std::cout << " ";
    std::cout << name_ << " " << std::endl;

    for(FolderIter iter = children_.begin(); iter != children_.end(); ++iter)
        iter->DebugDumpInventoryFolderStructure(indentationLevel + 3);

    for(ItemIter iter = items_.begin(); iter != items_.end(); ++iter)
        std::cout << "      " << iter->GetName() << std::endl;
}
#endif

InventoryFolder *Inventory::GetFirstSubFolderByName(const char *name)
{
    return root.GetFirstSubFolderByName(name);
}

InventoryFolder *Inventory::GetFirstSubFolderByID(const RexUUID &searchId)
{
    return root.GetFirstSubFolderByID(searchId);
}

InventoryFolder *Inventory::GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent)
{
    // Return an existing folder if one with the given id is present.
    InventoryFolder *existing = GetFirstSubFolderByID(id);
    if (existing)
        return existing;

    // Create a new folder.
    InventoryFolder newFolder;
    newFolder.SetID(id);
    newFolder.SetName("New Folder");
    return parent.AddSubFolder(newFolder);
}

#ifdef _DEBUG
void Inventory::DebugDumpInventoryFolderStructure()
{
    root.DebugDumpInventoryFolderStructure(0);
}
#endif
