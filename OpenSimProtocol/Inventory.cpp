// For conditions of distribution and use, see copyright notice in license.txt

/// @file Inventory.cpp
/// @brief Inventory structure skeleton .

#include "StableHeaders.h"
#include "Inventory.h"

namespace OpenSimProtocol
{

InventoryFolderSkeleton::InventoryFolderSkeleton() :
    id(RexUUID()), name(""), type(255), parent(0), editable(true), version(0), type_default(0)
{
}

InventoryFolderSkeleton::InventoryFolderSkeleton(const RexUUID &newid, const std::string &newname) :
    id(newid), name(newname), type(255), parent(0), editable(true), version(0), type_default(0)
{
}

InventoryFolderSkeleton *InventoryFolderSkeleton::AddChildFolder(const InventoryFolderSkeleton &folder)
{
    children.push_back(folder);
    children.back().parent = this;
    return &children.back();
}

InventoryFolderSkeleton *InventoryFolderSkeleton::GetFirstChildFolderByName(const char *searchName)
{
    if (name == searchName)
        return this;

    for(FolderIter iter = children.begin(); iter != children.end(); ++iter)
    {
        InventoryFolderSkeleton *folder = iter->GetFirstChildFolderByName(searchName);
        if (folder)
            return folder;
    }

    return 0;
}

InventoryFolderSkeleton *InventoryFolderSkeleton::GetChildFolderByID(const RexUUID &searchId)
{
    if (id == searchId)
        return this;

    for(FolderIter iter = children.begin(); iter != children.end(); ++iter)
    {
        InventoryFolderSkeleton *folder = iter->GetChildFolderByID(searchId);
        if (folder)
            return folder;
    }

    return 0;
}

void InventoryFolderSkeleton::DebugDumpInventoryFolderStructure(int indentationLevel)
{
    for(int i = 0; i < indentationLevel; ++i)
        std::cout << " ";
    std::cout << name << " " << std::endl;

    for(FolderIter iter = children.begin(); iter != children.end(); ++iter)
        iter->DebugDumpInventoryFolderStructure(indentationLevel + 3);
}

InventorySkeleton::InventorySkeleton()
{
    root_ = InventoryFolderSkeleton(RexUUID::CreateRandom(), "OpenSim Inventory");
}

InventoryFolderSkeleton *InventorySkeleton::GetFirstChildFolderByName(const char *searchName)
{
    return root_.GetFirstChildFolderByName(searchName);
}

InventoryFolderSkeleton *InventorySkeleton::GetChildFolderByID(const RexUUID &searchId)
{
    return root_.GetChildFolderByID(searchId);
}

void InventorySkeleton::DebugDumpInventoryFolderStructure()
{
    root_.DebugDumpInventoryFolderStructure(0);
}

} // namespace OpenSimProtocol
