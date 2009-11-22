// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.cpp
 *  @brief Data model providing logic for working with the hierarchy of an OpenSim inventory.
 */

#include "StableHeaders.h"
#include "OpenSimInventoryDataModel.h"
#include "InventoryModule.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"
#include "Inventory/InventorySkeleton.h"
#include "Inventory/InventoryEvents.h"
#include "AssetEvents.h"
#include "ResourceInterface.h"
#include "TextureResource.h"
#include "AssetUploader.h"

#include <QFile>
#include <QImage>
#include <QStringList>

namespace Inventory
{

OpenSimInventoryDataModel::OpenSimInventoryDataModel(
    Foundation::Framework *framework,
    ProtocolUtilities::InventorySkeleton *inventory_skeleton) :
    framework_(framework),
    rootFolder_(0),
    worldLibraryOwnerId_(""),
    assetUploader_(0)
{
    SetupModelData(inventory_skeleton);
    assetUploader_ = new AssetUploader(framework_, this);
}

OpenSimInventoryDataModel::~OpenSimInventoryDataModel()
{
    SAFE_DELETE(rootFolder_);
    SAFE_DELETE(assetUploader_);
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

AbstractInventoryItem *OpenSimInventoryDataModel::GetOrCreateNewFolder(
    const QString &id,
    AbstractInventoryItem &parentFolder,
    const QString &name,
    const bool &notify_server)
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
        currentWorldStream_->SendCreateInventoryFolderPacket(
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
    InventoryFolder *parent = static_cast<InventoryFolder *>(&parentFolder);
    if (!parent)
        return 0;

    // Return an existing asset if one with the given id is present.
    InventoryAsset *existing = dynamic_cast<InventoryAsset *>(parent->GetChildAssetById(inventory_id));
    if (existing)
        return existing;

    // Create a new asset.
    InventoryAsset *newAsset = new InventoryAsset(inventory_id, asset_id, name, parent);

    if (parent->IsDescendentOf(GetOpenSimLibraryFolder()))
        newAsset->SetIsLibraryItem(true);

    return parent->AddChild(newAsset);
}

void OpenSimInventoryDataModel::FetchInventoryDescendents(AbstractInventoryItem *item)
{
    if(item->GetItemType() != AbstractInventoryItem::Type_Folder)
        return;

    if (item->IsDescendentOf(GetOpenSimLibraryFolder()))
        currentWorldStream_->SendFetchInventoryDescendentsPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(worldLibraryOwnerId_));
    else
        currentWorldStream_->SendFetchInventoryDescendentsPacket(QSTR_TO_UUID(item->GetID()));
}

void OpenSimInventoryDataModel::NotifyServerAboutItemMove(AbstractInventoryItem *item)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        currentWorldStream_->SendMoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()));

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
        currentWorldStream_->SendMoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()), item->GetName().toStdString());
}

void OpenSimInventoryDataModel::NotifyServerAboutItemCopy(AbstractInventoryItem *item)
{
    if (item->GetItemType() != AbstractInventoryItem::Type_Asset)
        return;

    currentWorldStream_->SendCopyInventoryItemPacket(QSTR_TO_UUID(worldLibraryOwnerId_),
        QSTR_TO_UUID(item->GetID()), QSTR_TO_UUID(item->GetParent()->GetID()), item->GetName().toStdString());
}

void OpenSimInventoryDataModel::NotifyServerAboutItemRemove(AbstractInventoryItem *item)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        currentWorldStream_->SendRemoveInventoryFolderPacket(QSTR_TO_UUID(item->GetID()));

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
        currentWorldStream_->SendRemoveInventoryItemPacket(QSTR_TO_UUID(item->GetID()));
}

void OpenSimInventoryDataModel::NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name)
{
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        currentWorldStream_->SendUpdateInventoryFolderPacket(QSTR_TO_UUID(item->GetID()),
            QSTR_TO_UUID(item->GetParent()->GetID()), 127, item->GetName().toStdString());

    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
    {
        InventoryAsset *asset = static_cast<InventoryAsset *>(item);
        currentWorldStream_->SendUpdateInventoryItemPacket(QSTR_TO_UUID(asset->GetID()),
            QSTR_TO_UUID(asset->GetParent()->GetID()), asset->GetAssetType(), asset->GetInventoryType(),
            asset->GetName().toStdString(), asset->GetDescription().toStdString());
    }
}

void OpenSimInventoryDataModel::UploadFile(const QString &filename, AbstractInventoryItem *parent_folder)
{
    if (!assetUploader_)
    {
        InventoryModule::LogError("Asset uploader not set. Can't upload.");
        return;
    }

    ///\note For now, we're not interested about destination folder as they're hardcoded.
    QStringList list;
    list << filename;
    assetUploader_->UploadFiles(list);
}

void OpenSimInventoryDataModel::UploadFiles(QStringList &filenames, AbstractInventoryItem *parent_folder)
{
    assetUploader_->UploadFiles(filenames);
}

void OpenSimInventoryDataModel::UploadFilesFromBuffer(QStringList &filenames, QVector<QVector<uchar> > &buffers,
    AbstractInventoryItem *parent_folder)
{
    assetUploader_->UploadBuffers(filenames, buffers);
}

void OpenSimInventoryDataModel::DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item)
{
    using namespace Foundation;

    InventoryAsset *asset = dynamic_cast<InventoryAsset *>(selected_item);
    if (!asset)
        return;

    std::string id = asset->GetAssetReference().toStdString();
    asset_type_t asset_type = asset->GetAssetType();

    // Create full filename.
    QString fullFilename = store_folder; 
    fullFilename += "/";
    fullFilename += asset->GetName();
    fullFilename += QString(RexTypes::GetFileExtensionFromAssetType(asset_type).c_str());

    ServiceManagerPtr service_manager = framework_->GetServiceManager();
    switch(asset_type)
    {
    case RexAT_Texture:
    {
        // Request textures from texture decoder.
        if (service_manager->IsRegistered(Service::ST_Texture))
        {
            boost::shared_ptr<TextureServiceInterface> texture_service =
                service_manager->GetService<TextureServiceInterface>(Service::ST_Texture).lock();

            Core::request_tag_t tag = texture_service->RequestTexture(id);
            if (tag)
                downloadRequests_[qMakePair(tag, asset_type)] = fullFilename;
        }
    }
    case RexAT_Mesh:
    case RexAT_Skeleton:
    case RexAT_MaterialScript:
    case RexAT_ParticleScript:
    {
        // Request other assets from asset system.
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = 
                service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();

            Core::request_tag_t tag = asset_service->RequestAsset(id, GetTypeNameFromAssetType(asset_type));
            if (tag) 
                downloadRequests_[qMakePair(tag, asset_type)] = fullFilename;
        }
        break;
    }
    case RexAT_GenericAvatarXml:
    case RexAT_FlashAnimation:
        InventoryModule::LogError("Non-supported asset type for download: " + GetTypeNameFromAssetType(asset_type));
        break;
    case RexAT_None:
    default:
        InventoryModule::LogError("Invalid asset type for download.");
        break;
    }
}

AbstractInventoryItem *OpenSimInventoryDataModel::GetRoot() const
{
    return rootFolder_;
}

void OpenSimInventoryDataModel::SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream)
{
    currentWorldStream_ = world_stream;
    assetUploader_->SetWorldStream(world_stream);
}

void OpenSimInventoryDataModel::HandleResourceReady(Foundation::EventDataInterface *data)
{
    Resource::Events::ResourceReady* resourceReady = checked_static_cast<Resource::Events::ResourceReady *>(data);
    RexTypes::asset_type_t asset_type = RexAT_Texture;
    Core::request_tag_t tag = resourceReady->tag_;
//    QString asset_id = resourceReady->id_.c_str();

    AssetRequestMap::iterator i = downloadRequests_.find(qMakePair(tag, asset_type));
    if (i == downloadRequests_.end())
        return;

    TextureDecoder::TextureResource *tex = checked_static_cast<TextureDecoder::TextureResource *>(resourceReady->resource_.get());
    if (tex->GetLevel() != 0)
        return;

    uint size = tex->GetWidth() * tex->GetHeight() * tex->GetComponents();
    QImage img = QImage::fromData(tex->GetData(), size);
    img.save(i.value());

    InventoryModule::LogInfo("File " + i.value().toStdString() + " succesfully saved.");

    downloadRequests_.erase(i);
}

void OpenSimInventoryDataModel::HandleAssetReady(Foundation::EventDataInterface *data)
{
    Asset::Events::AssetReady *assetReady = checked_static_cast<Asset::Events::AssetReady*>(data);
    Core::request_tag_t tag = assetReady->tag_;
    asset_type_t asset_type = RexTypes::GetAssetTypeFromTypeName(assetReady->asset_type_);
//    QString asset_id = assetReady->asset_->GetId().c_str();

    // Don't handle AssetReady's for textures. We're interested in ResourceReady event for textures;
    if (asset_type == RexAT_Texture)
        return;

    AssetRequestMap::iterator i = downloadRequests_.find(qMakePair(tag, asset_type));
    if (i == downloadRequests_.end())
        return;

    Foundation::AssetPtr asset = assetReady->asset_;
    QFile file(i.value());
    file.open(QIODevice::WriteOnly);
    file.write((const char*)asset->GetData(), asset->GetSize());
    file.close();

    InventoryModule::LogInfo("File " + i.value().toStdString() + " succesfully saved.");

    downloadRequests_.erase(i);
}

void OpenSimInventoryDataModel::HandleInventoryDescendents(Foundation::EventDataInterface *data)
{

    InventoryItemEventData *item_data = checked_static_cast<InventoryItemEventData *>(data);

    AbstractInventoryItem *parentFolder = GetChildFolderById(STD_TO_QSTR(item_data->parentId.ToString()));
    if (!parentFolder)
        return;

    AbstractInventoryItem *existing = GetChildById(STD_TO_QSTR(item_data->id.ToString()));
    if (existing)
        return;

    if (item_data->item_type == IIT_Folder)
    {
        InventoryFolder *newFolder = static_cast<InventoryFolder *>(GetOrCreateNewFolder(
            STD_TO_QSTR(item_data->id.ToString()), *parentFolder, false));

        newFolder->SetName(STD_TO_QSTR(item_data->name));
        ///\todo newFolder->SetType(item_data->type);
        newFolder->SetDirty(true);
    }
    if (item_data->item_type == IIT_Asset)
    {
        InventoryAsset *newAsset = static_cast<InventoryAsset *>(GetOrCreateNewAsset(
            STD_TO_QSTR(item_data->id.ToString()), STD_TO_QSTR(item_data->assetId.ToString()),
            *parentFolder, STD_TO_QSTR(item_data->name)));

        newAsset->SetDescription(STD_TO_QSTR(item_data->description));
        newAsset->SetInventoryType(item_data->inventoryType);
        newAsset->SetAssetType(item_data->assetType);
    }
}

#ifdef _DEBUG
void OpenSimInventoryDataModel::DebugDumpInventoryFolderStructure()
{
    rootFolder_->DebugDumpInventoryFolderStructure(0);
}
#endif

void OpenSimInventoryDataModel::CreateNewFolderFromFolderSkeleton(
    InventoryFolder *parent_folder,
    ProtocolUtilities::InventoryFolderSkeleton *folder_skeleton)
{
    using namespace ProtocolUtilities;

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

        // Flag Library folders. They have some special behavior.
        if (GetOpenSimLibraryFolder())
            if (newFolder->IsDescendentOf(GetOpenSimLibraryFolder()))
                newFolder->SetIsLibraryItem(true);
    }

    InventoryFolderSkeleton::FolderIter iter = folder_skeleton->children.begin();
    while(iter != folder_skeleton->children.end())
    {
        parent_folder = newFolder;
        CreateNewFolderFromFolderSkeleton(parent_folder, &*iter);
        ++iter;
    }
}

void OpenSimInventoryDataModel::SetupModelData(ProtocolUtilities::InventorySkeleton *inventory_skeleton)
{
    if (!inventory_skeleton->GetRoot())
    {
        InventoryModule::LogError("Couldn't find inventory root folder skeleton. Can't create OpenSim inventory data model.");
        return;
    }

    worldLibraryOwnerId_ = STD_TO_QSTR(inventory_skeleton->worldLibraryOwnerId.ToString());

    CreateNewFolderFromFolderSkeleton(0, inventory_skeleton->GetRoot());
}

} // namespace Inventory
