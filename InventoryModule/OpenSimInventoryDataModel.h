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
        OpenSimInventoryDataModel(OpenSimProtocol::InventorySkeleton *inventory_skeleton);

        /// Destructor.
        virtual ~OpenSimInventoryDataModel();

        /// AbstractInventoryDataModel override.
        //void AddFolder(InventoryFolder newFolder, InventoryFolder *parent);

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested name or null if the folder isn't found.
        AbstractInventoryItem *GetFirstChildFolderByName(const QString &searchName) const;

        /// AbstractInventoryDataModel override.
        /// @return First folder by the requested id or null if the folder isn't found.
        AbstractInventoryItem *GetChildFolderByID(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        /// Returns folder by requested id, or creates a new one if the folder doesnt exist.
        /// @param id ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created folder.
        AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder);

        /// AbstractInventoryDataModel override.
        /// Returns asset requested id, or creates a new one if the folder doesnt exist.
        /// @param inventory_id Inventory ID.
        /// @param asset_id Asset ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created asset.
        AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parenFolder, const QString &name = "New Asset");

        /// @return Inventory root folder.
        AbstractInventoryItem *GetRoot() const { return rootFolder_; }

        /// @return Pointer to "My Inventory" folder or null if not found.
        AbstractInventoryItem *GetMyInventoryFolder() const;

        /// @return Pointer to "Trash" folder or null if not found.
        AbstractInventoryItem *GetTrashFolder() const;

        void DebugDumpInventoryFolderStructure()
        {
            rootFolder_->DebugDumpInventoryFolderStructure(0);
        }

    private:
        Q_DISABLE_COPY(OpenSimInventoryDataModel);

        /// Utility function for creating new folders from the folder skeletons. Used recursively.
        /// parent_folder Parent folder.
        /// folder_skeleton Folder skeleton for the folder to be created.
        void CreateNewFolderFromFolderSkeleton(
            InventoryFolder *parent_folder,
            OpenSimProtocol::InventoryFolderSkeleton *folder_skeleton);

        /// Creates the tree model data for inventory.
        /// @param inventory_skeleton OpenSim inventory skeleton.
        void SetupModelData(OpenSimProtocol::InventorySkeleton *inventory_skeleton);

        /// The root folder.
        InventoryFolder *rootFolder_;
    };
}

#endif
