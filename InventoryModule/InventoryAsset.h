/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryAsset.h
 *  @brief  A class representing asset in inventory.
 */

#ifndef incl_InventoryModule_InventoryAsset_h
#define incl_InventoryModule_InventoryAsset_h

#include "AbstractInventoryItem.h"
#include "RexUUID.h"

namespace Inventory
{
    class InventoryFolder;

    /// A class representing asset in inventory.
    class InventoryAsset : public AbstractInventoryItem
    {
        Q_OBJECT
        Q_PROPERTY(QString assetReference_ READ GetAssetReference WRITE SetAssetReference)
        Q_PROPERTY(QString description_ READ GetDescription WRITE SetDescription)
        Q_PROPERTY(asset_type_t assetType_ READ GetAssetType WRITE SetAssetType)
        Q_PROPERTY(inventory_type_t inventoryType_ READ GetInventoryType WRITE SetInventoryType)

    public:
        /** Constructor.
            @param id ID.
            @param asset_reference Asset reference.
            @param name Name.
            @param parent Parent folder.

            @note If you pass parent folder, the asset isn't currently added to parent's child automatically.
        */
        InventoryAsset(const QString &id, const QString &asset_reference = "", const QString &name = "New Item",
            InventoryFolder *parent = 0);

        /// Destructor.
        virtual ~InventoryAsset();

        /// AbstractInventoryItem override
        QString GetName() const {return name_; }

        /// AbstractInventoryItem override
        void SetName(const QString &name) { name_ = name; }

        /// AbstractInventoryItem override
        QString GetID() const { return id_; }

        /// AbstractInventoryItem override
        void SetID(const QString &id) { id_ = id; }

        /// AbstractInventoryItem override
        AbstractInventoryItem *GetParent() const { return parent_; }

        /// AbstractInventoryItem override
        void SetParent(AbstractInventoryItem *parent) { parent_ = parent; }

        /// AbstractInventoryItem override
        bool IsEditable() const { return editable_; }

        /// AbstractInventoryItem override
        void SetEditable(const bool editable) { editable_ = editable; }

        /// AbstractInventoryItem override
        bool IsLibraryItem() const { return libraryItem_; }

        /// AbstractInventoryItem override
        void SetIsLibraryItem(const bool value) { libraryItem_ = value; }

        /// AbstractInventoryItem override
        InventoryItemType GetItemType() const { return itemType_; }

        // ===== InventoryAsset API ===== //

        /// Is this folder descendent of spesific folder.
        /// @param searchFolder Folder to be investigated.
        bool IsDescendentOf(AbstractInventoryItem *searchFolder) const;

        /// @return asset reference.
        QString GetAssetReference() const { return assetReference_; }

        /// @param asset_reference New asset reference.
        void SetAssetReference(const QString &asset_reference) { assetReference_ = asset_reference; }

        /// @return Description.
        QString GetDescription() const { return description_; }

        /// @param description New description.
        void SetDescription(const QString &description) { description_ = description; }

        /// Get/set for the description.
        void SetAssetType(const asset_type_t asset_type) { assetType_ = asset_type; }
        asset_type_t GetAssetType() const { return assetType_;}

        /// @return Inventory type (see RexTypes.h).
        inventory_type_t GetInventoryType() const { return inventoryType_; }

        /// @param inventory_type New inventory type (see RexTypes.h).
        void SetInventoryType(const inventory_type_t inventory_type) { inventoryType_ = inventory_type; }

        /// Get/set for creator ID.
        void SetCreatorId(const RexUUID &creator_id) { creatorId_ = creator_id; uuids_.push_back(creator_id); }
        RexUUID GetCreatorId() const { return creatorId_; }

        /// Get/set owner ID.
        void SetOwnerId(const RexUUID &owner_id) { ownerId_ = owner_id; uuids_.push_back(owner_id); }
        RexUUID GetOwnerId() const { return ownerId_; }

        /// Get/set for group ID.
        void SetGroupId(const RexUUID &group_id) { groupId_ = group_id; uuids_.push_back(group_id); }
        RexUUID GetGroupId() const { return groupId_; }

        /// Get/set creation time.
        void SetCreationTime(time_t time) { creationTime_ = time; }
        time_t GetCreationTime() const { return creationTime_; }

        /// Returns creation time as a string.
        QString GetCreationTimeString() const;

        /// @return Row number of this inventory asset.
//        int Row() const;

    private:
        Q_DISABLE_COPY(InventoryAsset);

        /// Type of item (folder or asset)
        InventoryItemType itemType_;

        /// Asset this item reference to. Can be either uuid or url.
        QString assetReference_;

        /// Description of this inventory asset.
        QString description_;

        /// Asset type.
        asset_type_t assetType_;

        /// Inventory type.
        inventory_type_t inventoryType_;

        /// Creator ID.
        RexUUID creatorId_;

        /// Owner ID.
        RexUUID ownerId_;

        /// Group ID.
        RexUUID groupId_;

        /// Time of creation.
        time_t creationTime_;

        /// Library asset flag.
        bool libraryItem_;

        // List of UUID's related to this asset (e.g. owner, creator and group)
        QList<RexUUID> uuids_;
    };
}

#endif
