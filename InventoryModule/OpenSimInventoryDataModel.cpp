// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.cpp
 *  @brief Data model representing the hierarchy of an OpenSim inventory.
 */

#include "StableHeaders.h"
#include "OpenSimInventoryDataModel.h"
#include "AbstractInventoryItem.h"
#include "InventoryModule.h"
#include "Inventory.h"
#include "RexUUID.h"

namespace Inventory
{

OpenSimInventoryDataModel::OpenSimInventoryDataModel(OpenSimProtocol::InventorySkeleton *inventory_skeleton) :
    rootFolder_(0)
{
    SetupModelData(inventory_skeleton);
}

// virtual
OpenSimInventoryDataModel::~OpenSimInventoryDataModel()
{
    delete rootFolder_;
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetFirstChildFolderByName(const QString &searchName) const
{
    return rootFolder_->GetFirstChildFolderByName(searchName);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetChildFolderByID(const QString &searchId) const
{
    return rootFolder_->GetChildFolderByID(searchId);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetMyInventoryFolder() const
{
    return rootFolder_->GetFirstChildFolderByName("My Inventory");
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetTrashFolder() const
{
    return rootFolder_->GetFirstChildFolderByName("Trash");
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder)
{
    // Return an existing folder if one with the given id is present.
    InventoryFolder *existing = dynamic_cast<InventoryFolder *>(GetChildFolderByID(id));
    if (existing)
        return existing;

    // Create a new folder.
    InventoryFolder *parent = dynamic_cast<InventoryFolder *>(&parentFolder);
    InventoryFolder *newFolder = new InventoryFolder(id, "New Folder", true, parent);

    return parent->AddChild(newFolder);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetOrCreateNewAsset(
    const QString &inventory_id,
    const QString &asset_id,
    AbstractInventoryItem &parentFolder,
    const QString &name)
{
    // Return an existing asset if one with the given id is present.
    InventoryFolder *parent = dynamic_cast<InventoryFolder *>(&parentFolder);
    InventoryAsset *existing = dynamic_cast<InventoryAsset *>(parent->GetChildAssetByID(inventory_id));
    if (existing)
        return existing;

    // Create a new asset.
    InventoryAsset *newAsset = new InventoryAsset(inventory_id, asset_id, name, parent);

    return parent->AddChild(newAsset);
}

void OpenSimInventoryDataModel::CreateNewFolderFromFolderSkeleton(
    InventoryFolder *parent_folder,
    OpenSimProtocol::InventoryFolderSkeleton *folder_skeleton)
{
    using namespace OpenSimProtocol;

    InventoryFolder *newFolder= new InventoryFolder(QString(folder_skeleton->id.ToString().c_str()),
        QString(folder_skeleton->name.c_str()), folder_skeleton->editable, parent_folder);

    if (!parent_folder)
        rootFolder_ = newFolder;
    else
        parent_folder->AddChild(newFolder);
/*
    if (parent_folder)
        std::cout << parent_folder->GetName().toStdString() << std::endl; 
    std::cout << "   " << newFolder->GetName().toStdString() << std::endl;
*/
    InventoryFolderSkeleton::FolderIter iter = folder_skeleton->children.begin();
    while(iter != folder_skeleton->children.end())
    {
        CreateNewFolderFromFolderSkeleton(parent_folder, &*iter);
        ++iter;
    }
}

void OpenSimInventoryDataModel::SetupModelData(OpenSimProtocol::InventorySkeleton *inventory_skeleton)
{
    using namespace OpenSimProtocol;

//    inventory_skeleton->DebugDumpInventoryFolderStructure();

    InventoryFolderSkeleton *root_skel = inventory_skeleton->GetRoot();
    if (!root_skel)
    {
        InventoryModule::LogError("Couldn't find inventory root folder skeleton. Can't create OpenSim inventory data model.");
        return;
    }

    CreateNewFolderFromFolderSkeleton(0, root_skel);
}

} // namespace Inventory
