// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.h
 *  @brief 
 */

#ifndef InventoryAsset_h
#define InventoryAsset_h

#include "OpenSimProtocolModuleApi.h"
#include "InventoryItemBase.h"

#include <QList>
#include <QVariant>

using namespace RexTypes;

namespace OpenSimProtocol
{
    class OSPROTO_MODULE_API InventoryAsset : public InventoryItemBase
    {
    public:
        /// Constructor.
        /// @param id ID.
        /// @param name Name.
        /// @param parent Parent folder.
        InventoryAsset(
            const RexUUID &inventory_id,
            const RexUUID &asset_id,
            const std::string &name = "New Item",
            InventoryFolder *parent = 0);

        /// Destructor.
        virtual ~InventoryAsset();

        ///
        int ColumnCount() const;

        ///
        QVariant Data(int column) const;

        /// @return Row number of this inventory asset.
        int Row() const;

        /// @return ID of the asset this inventory item refers to.
        const RexUUID & GetAssetID() const { return assetID_; }

        /// Sets the description.
        /// @param description New description.
        void SetDescription(const std::string &description) { description_ = description; }

        /// @return Description.
        const std::string &GetDescription() const { return description_;}

        /// @return Asset type.
        const asset_type_t GetAssetType() const { return assetType_; }

        /// @return Inventory type.
        const inventory_type_t GetInventoryType() const { return inventoryType_; }

    private:
        ///
        QList<QVariant> itemData_;

        /// ID of the asset this inventory item refers to.
        RexUUID assetID_;

        /// Description if this inventory asset.
        std::string description_;

        /// Asset type.
        asset_type_t assetType_;

        /// Inventory type.
        inventory_type_t inventoryType_;
    };
}

#endif
