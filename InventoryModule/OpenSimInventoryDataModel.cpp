// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.cpp
 *  @brief Data model representing the hierarchy of an OpenSim inventory.
 */

#include "StableHeaders.h"
#include "OpenSimInventoryDataModel.h"
#include "AbstractInventoryItem.h"
#include "InventoryModule.h"
#include "RexLogicModule.h"
#include "InventorySkeleton.h"
#include "RexUUID.h"

namespace Inventory
{

OpenSimInventoryDataModel::OpenSimInventoryDataModel(RexLogic::RexLogicModule *rex_logic_module) :
    rexLogicModule_(rex_logic_module),
    rootFolder_(0),
    worldLibraryOwnerId_("")
{
    SetupModelData(rexLogicModule_->GetInventory().get());
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

AbstractInventoryItem *OpenSimInventoryDataModel::GetChildFolderById(const QString &searchId) const
{
    return rootFolder_->GetChildFolderById(searchId);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetChildAssetById(const QString &searchId) const
{
    return rootFolder_->GetChildAssetById(searchId);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetChildById(const QString &searchId) const
{
    return rootFolder_->GetChildById(searchId);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetTrashFolder() const
{
    return rootFolder_->GetFirstChildFolderByName("Trash");
}

InventoryFolder *OpenSimInventoryDataModel::GetMyInventoryFolder() const
{
    return rootFolder_->GetFirstChildFolderByName("My Inventory");
}

InventoryFolder *OpenSimInventoryDataModel::GetOpenSimLibraryFolder() const
{
    return rootFolder_->GetFirstChildFolderByName("OpenSim Library");
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
    const QString &name, const bool &notify_server)
{
    InventoryFolder *parent = dynamic_cast<InventoryFolder *>(&parentFolder);
    if (!parent)
        return 0;

    // Return an existing folder if one with the given id is present.
    InventoryFolder *existing = dynamic_cast<InventoryFolder *>(parent->GetChildFolderById(id));
    if (existing)
        return existing;

    // Create a new folder.
    InventoryFolder *newFolder = new InventoryFolder(id, name, parent);

    if (GetOpenSimLibraryFolder())
        if (parent->IsDescendentOf(GetOpenSimLibraryFolder()))
            newFolder->SetIsLibraryItem(true);

    // Inform the server.
    // We don't want to notify server if we're creating folders "ordered" by server via InventoryDescecendents packet.
    if (notify_server)
        rexLogicModule_->GetServerConnection()->SendCreateInventoryFolderPacket(
            RexUUID(parent->GetID().toStdString()), RexUUID(newFolder->GetID().toStdString()),
            255, newFolder->GetName().toStdString().c_str());

    return parent->AddChild(newFolder);
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetOrCreateNewAsset(
    const QString &inventory_id,
    const QString &asset_id,
    AbstractInventoryItem &parentFolder,
    const QString &name)
{
    // Return an existing asset if one with the given id is present.
    InventoryFolder *parent = static_cast<InventoryFolder *>(&parentFolder);
    if (!parent)
        return 0;

    InventoryAsset *existing = dynamic_cast<InventoryAsset *>(parent->GetChildAssetById(inventory_id));
    if (existing)
        return existing;

    // Create a new asset.
    InventoryAsset *newAsset = new InventoryAsset(inventory_id, asset_id, name, parent);

    if (parent->IsDescendentOf(GetOpenSimLibraryFolder()))
        newAsset->SetIsLibraryItem(true);

    return parent->AddChild(newAsset);
}

void OpenSimInventoryDataModel::FetchInventoryDescendents(AbstractInventoryItem *folder)
{
    if (folder->IsDescendentOf(GetOpenSimLibraryFolder()))
        rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendentsPacket(QSTR_TO_UUID(folder->GetID()),
            QSTR_TO_UUID(worldLibraryOwnerId_));
    else
        rexLogicModule_->GetServerConnection()->SendFetchInventoryDescendentsPacket(QSTR_TO_UUID(folder->GetID()));
}

void OpenSimInventoryDataModel::NotifyServerAboutItemMove(AbstractInventoryItem *item)
{
    std::cout << "Moving " << item->GetName().toStdString() << " to " << item->GetParent()->GetName().toStdString() << std::endl;

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        rexLogicModule_->GetServerConnection()->SendMoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()));

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
        rexLogicModule_->GetServerConnection()->SendMoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()), item->GetName().toStdString());
}

void OpenSimInventoryDataModel::NotifyServerAboutItemCopy(AbstractInventoryItem *item)
{
    if (item->GetItemType() != AbstractInventoryItem::Type_Asset)
        return;

    rexLogicModule_->GetServerConnection()->SendCopyInventoryItemPacket(QSTR_TO_UUID(worldLibraryOwnerId_),
        QSTR_TO_UUID(item->GetID()), QSTR_TO_UUID(item->GetParent()->GetID()), item->GetName().toStdString());
}

void OpenSimInventoryDataModel::NotifyServerAboutItemRemove(AbstractInventoryItem *item)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        rexLogicModule_->GetServerConnection()->SendRemoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()));

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
        rexLogicModule_->GetServerConnection()->SendRemoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()));

    // When deleting items, we move them first to the Trash folder.
    // If the folder is already in the trash folder, delete it for good.
    ///\todo Move the "deleted" folder to the Trash folder and update the view.
    /*InventoryFolder *trashFolder = GetTrashFolder();

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        if (!trashFolder)
        {
            InventoryModule::LogError("Can't find Trash folder. Moving folder to Trash not possible. Deleting folder.");
            rexLogicModule_->GetServerConnection()->SendRemoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()));
            return;
        }

        if (item->GetParent() == trashFolder)
            rexLogicModule_->GetServerConnection()->SendRemoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()));
        else
            rexLogicModule_->GetServerConnection()->SendMoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()),
                QSTR_TO_UUID(trashFolder->GetID()));
        return;
    }

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
    {
        if (!trashFolder)
        {
            InventoryModule::LogError("Can't find Trash folder. Moving asset to Trash not possible. Deleting asset.");
            rexLogicModule_->GetServerConnection()->SendRemoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()));
            return;
        }

        if (item->GetParent() == trashFolder)
            rexLogicModule_->GetServerConnection()->SendRemoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()));
        else
            rexLogicModule_->GetServerConnection()->SendMoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()),
                QSTR_TO_UUID(trashFolder->GetID()), item->GetName().toStdString());
    }
    */
}

void OpenSimInventoryDataModel::NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        rexLogicModule_->GetServerConnection()->SendUpdateInventoryFolderPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()), 127, item->GetName().toStdString());

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
    {
        InventoryAsset *asset = static_cast<InventoryAsset *>(item);
        rexLogicModule_->GetServerConnection()->SendUpdateInventoryItemPacket(QSTR_TO_UUID(asset->GetID()),
            QSTR_TO_UUID(asset->GetParent()->GetID()), asset->GetAssetType(), asset->GetInventoryType(),
            asset->GetName().toStdString(), asset->GetDescription().toStdString());
    }
}

void OpenSimInventoryDataModel::DebugDumpInventoryFolderStructure()
{
    rootFolder_->DebugDumpInventoryFolderStructure(0);
}

void OpenSimInventoryDataModel::CreateNewFolderFromFolderSkeleton(
    InventoryFolder *parent_folder,
    OpenSimProtocol::InventoryFolderSkeleton *folder_skeleton)
{
    using namespace OpenSimProtocol;

    InventoryFolder *newFolder = new InventoryFolder(STD_TO_QSTR(folder_skeleton->id.ToString()),
        STD_TO_QSTR(folder_skeleton->name), parent_folder, folder_skeleton->editable);
    //if (!folder_skeleton->HasChildren())
    newFolder->SetDirty(true);

    if (!rootFolder_ && !parent_folder)
        rootFolder_ = newFolder;

    if (parent_folder)
    {
        parent_folder->AddChild(newFolder);

        if (newFolder == GetOpenSimLibraryFolder())
            newFolder->SetIsLibraryItem(true);

        if (GetOpenSimLibraryFolder())
            if (newFolder->IsDescendentOf(GetOpenSimLibraryFolder()))
                newFolder->SetIsLibraryItem(true);
        // Flag Library folders. They have some special behavior.
    }

    InventoryFolderSkeleton::FolderIter iter = folder_skeleton->children.begin();
    while(iter != folder_skeleton->children.end())
    {
        parent_folder = newFolder;
        CreateNewFolderFromFolderSkeleton(parent_folder, &*iter);
        ++iter;
    }
}

void OpenSimInventoryDataModel::SetupModelData(OpenSimProtocol::InventorySkeleton *inventory_skeleton)
{
    OpenSimProtocol::InventoryFolderSkeleton *root_skel = inventory_skeleton->GetRoot();
    if (!root_skel)
    {
        InventoryModule::LogError("Couldn't find inventory root folder skeleton. Can't create OpenSim inventory data model.");
        return;
    }

    worldLibraryOwnerId_ = STD_TO_QSTR(inventory_skeleton->worldLibraryOwnerId.ToString());

    CreateNewFolderFromFolderSkeleton(0, root_skel);
}

} // namespace Inventory
