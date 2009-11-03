// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file OpenSimInventoryDataModel.h
 *  @brief Data model representing the hierarchy of an OpenSim inventory.
 */

#ifndef incl_InventoryModule_OpenSimInventoryDataModel_h
#define incl_InventoryModule_OpenSimInventoryDataModel_h

#include "AbstractInventoryDataModel.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"

namespace RexLogic
{
    class RexLogicModule;
}

namespace OpenSimProtocol
{
    class InventorySkeleton;
    class InventoryFolderSkeleton;
}

namespace Inventory
{
    class OpenSimInventoryDataModel : public AbstractInventoryDataModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param rex_logic_module RexLogicModule pointer.
        OpenSimInventoryDataModel(RexLogic::RexLogicModule *rex_logic_module);

        /// Destructor.
        virtual ~OpenSimInventoryDataModel();

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested name or null if the folder isn't found.
        AbstractInventoryItem *GetFirstChildFolderByName(const QString &searchName) const;

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested id or null if the folder isn't found.
        AbstractInventoryItem *GetChildFolderById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// @return First item by the requested id or null if the item isn't found.
        AbstractInventoryItem *GetChildAssetById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// Returns pointer to requested child item.
        /// @param searchId Search ID.
        /// @return Pointer to the requested item, or null if not found.
        AbstractInventoryItem *GetChildById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// Returns folder by requested id, or creates a new one if the folder doesnt exist,
        /// or returns null if the parent folder is invalid.
        /// @param id ID.
        /// @param parent Parent folder.
        /// @param notify_server Do we want to notify server.
        /// @return Pointer to the existing or just created folder.
        AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name = "New Folder", const bool &notify_server = true);

        /// AbstractInventoryDataModel override.
        /// Returns asset requested id, or creates a new one if the folder doesnt exist.
        /// @param inventory_id Inventory ID.
        /// @param asset_id Asset ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created asset.
        AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name = "New Asset");

        /// AbstractInventoryDataModel override.
        void FetchInventoryDescendents(AbstractInventoryItem *folder);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemMove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemCopy(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemRemove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemUpdate(AbstractInventoryItem *item);

        /// @return Inventory root folder.
        AbstractInventoryItem *GetRoot() const { return rootFolder_; }

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder  *GetMyInventoryFolder() const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder  *GetOpenSimLibraryFolder() const;

        /// @return Pointer to "Trash" folder or null if not found.
        InventoryFolder  *GetTrashFolder() const;

        /// Prints the inventory tree structure to std::cout.
        void DebugDumpInventoryFolderStructure();

    private:
        Q_DISABLE_COPY(OpenSimInventoryDataModel);

        /// Utility function for creating new folders from the folder skeletons. Used recursively.
        /// @param parent_folder Parent folder.
        /// @param folder_skeleton Folder skeleton for the folder to be created.
        void CreateNewFolderFromFolderSkeleton(
            InventoryFolder *parent_folder,
            OpenSimProtocol::InventoryFolderSkeleton *folder_skeleton);

        /// Creates the tree model data for inventory.
        /// @param inventory_skeleton OpenSim inventory skeleton.
        void SetupModelData(OpenSimProtocol::InventorySkeleton *inventory_skeleton);

        /// RexLogicModule pointer.
        RexLogic::RexLogicModule *rexLogicModule_;

        /// The root folder.
        InventoryFolder *rootFolder_;

        /// World Library owner id.
        QString worldLibraryOwnerId_;
    };
}

#endif
