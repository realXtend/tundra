// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.h
 *  @brief Data model representing the hierarchy of an OpenSim inventory.
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
    class EventDataInterface;
}

namespace RexLogic
{
    class RexLogicModule;
}

namespace ProtocolUtilities
{
    class InventorySkeleton;
    class InventoryFolderSkeleton;
}

namespace Inventory
{
    class InventoryFolder;

    class OpenSimInventoryDataModel : public AbstractInventoryDataModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param rexlogicmodule RexLogicModule pointer.
        OpenSimInventoryDataModel(RexLogic::RexLogicModule *rexlogicmodule);

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
        /// In OpesimInventoryDataModel this function doesn't perform the actual download.
        /// This just request the asset from server using texture and asset services.
        /// @param store_folder
        /// @param selected_item
        void DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item);

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetRoot() const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetTrashFolder() const;

        /// Set World Stream to current
        void SetWorldStream(const ProtocolUtilities::WorldStreamPtr world_stream);

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetMyInventoryFolder() const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetOpenSimLibraryFolder() const;

        /// Saves asset to disk.
        /// @param data Event data. Can be ResourceReady for textures and AssetReady for other assets.
        void SaveAssetToDisk(Foundation::EventDataInterface *data);

        ///
        bool HasPendingDownloadRequests() const { return downloadRequests_.size() > 0; }

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

        /// RexLogicModule pointer.
        RexLogic::RexLogicModule *rexLogicModule_;

        /// The root folder.
        InventoryFolder *rootFolder_;

        /// World Library owner id.
        QString worldLibraryOwnerId_;

        /// Pointer to WorldStream
        ProtocolUtilities::WorldStreamPtr CurrentWorldStream;
//        QMap<Qstring, Core::RequestTagVector> requestTags_;

        /// Download request map.
        AssetRequestMap downloadRequests_;
    };
}

#endif
