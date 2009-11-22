// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.h
 *  @brief Data model providing logic for working with the hierarchy of an OpenSim inventory.
 */

#ifndef incl_InventoryModule_OpenSimInventoryDataModel_h
#define incl_InventoryModule_OpenSimInventoryDataModel_h

#include "AbstractInventoryDataModel.h"
#include "CoreTypes.h"
#include "RexTypes.h"
#include "WorldStream.h"

#include <QMap>
#include <QPair>

namespace Foundation
{
    class Framework;
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class InventorySkeleton;
    class InventoryFolderSkeleton;
}

namespace Inventory
{
    class InventoryFolder;
    class AssetUploader;

    class OpenSimInventoryDataModel : public AbstractInventoryDataModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Framework pointer.
        /// @inventory_skeleton Inventory skeleton pointer.
        OpenSimInventoryDataModel(
            Foundation::Framework *framework,
            ProtocolUtilities::InventorySkeleton *inventory_skeleton);

        /// Destructor.
        virtual ~OpenSimInventoryDataModel();

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetFirstChildFolderByName(const QString &searchName) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildFolderById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildAssetById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name = "New Folder", const bool &notify_server = true);

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name = "New Asset");

        /// AbstractInventoryDataModel override.
        void FetchInventoryDescendents(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemMove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemCopy(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemRemove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name);

        /// AbstractInventoryDataModel override.
        void UploadFile(const QString &filename, AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        void UploadFiles(QStringList &filenames, AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        void UploadFilesFromBuffer(QStringList &filenames, QVector<QVector<uchar> > &buffers,
            AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        /// In OpesimInventoryDataModel this function doesn't perform the actual download.
        /// This just request the asset from server using texture and asset services.
        void DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item);

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetRoot() const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetTrashFolder() const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetMyInventoryFolder() const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetOpenSimLibraryFolder() const;

        /// OpenSim inventory uses trash folder. Returns true.
        bool GetUseTrashFolder() const { return true; }

        /// Set World Stream.
        /// @param world_stream WorldStream pointer.
        void SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream);

        ///@return True if inventory has pending downloads.
        bool HasPendingDownloadRequests() const { return downloadRequests_.size() > 0; }

        /// @return Asset uploader.
        AssetUploader *GetAssetUploader() const {return assetUploader_; }

        /// Handles INVENTORY_DESCENDENTS event.
        /// @param data Event data.
        void HandleInventoryDescendents(Foundation::EventDataInterface *data);

        /// Handles RESOURCE_READY event.
        /// @param data Event data.
        void HandleResourceReady(Foundation::EventDataInterface *data);

        /// Handles ASSET_READY event.
        /// @param data Event data.
        void HandleAssetReady(Foundation::EventDataInterface *data);

#ifdef _DEBUG
        /// Prints the inventory tree structure to std::cout.
        void DebugDumpInventoryFolderStructure();
#endif

        typedef QMap<QPair<Core::request_tag_t, RexTypes::asset_type_t>, QString> AssetRequestMap;

    private:
        Q_DISABLE_COPY(OpenSimInventoryDataModel);

        /// Utility function for creating new folders from the folder skeletons. Used recursively.
        /// @param parent_folder Parent folder.
        /// @param folder_skeleton Folder skeleton for the folder to be created.
        void CreateNewFolderFromFolderSkeleton(
            InventoryFolder *parent_folder,
            ProtocolUtilities::InventoryFolderSkeleton *folder_skeleton);

        /// Creates the tree model data for inventory.
        /// @param inventory_skeleton OpenSim inventory skeleton.
        void SetupModelData(ProtocolUtilities::InventorySkeleton *inventory_skeleton);

        /// Framework pointer
        Foundation::Framework *framework_;

        /// The root folder.
        InventoryFolder *rootFolder_;

        /// World Library owner id.
        QString worldLibraryOwnerId_;

        /// Pointer to WorldStream
        ProtocolUtilities::WorldStreamPtr currentWorldStream_;

        /// Download request map.
        AssetRequestMap downloadRequests_;

        /// Asset uploader.
        AssetUploader *assetUploader_;
    };
}

#endif
