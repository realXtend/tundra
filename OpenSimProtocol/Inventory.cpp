// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Inventory.h"

InventoryFolder *InventoryFolder::GetFirstSubFolderByName(const char *searchName)
{
    if (name == searchName)
        return this;
    for(FolderList::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        InventoryFolder *folder = iter->GetFirstSubFolderByName(searchName);
        if (folder)
            return folder;
    }

    return 0;
}

InventoryFolder *InventoryFolder::GetFirstSubFolderByID(const RexTypes::RexUUID &searchId)
{
    if (id == searchId)
        return this;

    for(FolderList::iterator iter = children.begin(); iter != children.end(); ++iter)
    {
        InventoryFolder *folder = iter->GetFirstSubFolderByID(id);
        if (folder)
            return folder;
    }

    return 0;
}

InventoryFolder *InventoryFolder::AddSubFolder(const InventoryFolder &folder)
{
    children.push_back(folder);
    children.back().parent = this;
    return &children.back();
}

#ifdef _DEBUG
void InventoryFolder::DebugDumpInventoryFolderStructure(int indentationLevel)
{
    for(int i = 0; i < indentationLevel; ++i)
        std::cout << " ";
    std::cout << name << " " << id << std::endl;

    for(FolderList::iterator iter = children.begin(); iter != children.end(); ++iter)
        iter->DebugDumpInventoryFolderStructure(indentationLevel + 3);
}
#endif

InventoryFolder *Inventory::GetFirstSubFolderByName(const char *name)
{
    return root.GetFirstSubFolderByName(name);
}

InventoryFolder *Inventory::GetFirstSubFolderByID(const RexTypes::RexUUID &searchId)
{
    return root.GetFirstSubFolderByID(searchId);
}

#ifdef _DEBUG
void Inventory::DebugDumpInventoryFolderStructure()
{
    root.DebugDumpInventoryFolderStructure(0);
}
#endif

InventoryFolder *Inventory::GetOrCreateNewFolder(const RexTypes::RexUUID &id, InventoryFolder &parent)
{
    // Return an existing folder if one with the given id is present.
    InventoryFolder *existing = GetFirstSubFolderByID(id);
    if (existing)
        return existing;

    // Create a new folder.
    InventoryFolder newFolder;
    newFolder.id = id;
    newFolder.name = "New Folder";
    return parent.AddSubFolder(newFolder);
}
