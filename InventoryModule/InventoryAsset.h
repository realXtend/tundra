// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.h
 *  @brief A class representing asset in inventory.
 */

#ifndef incl_InventoryModule_InventoryAsset_h
#define incl_InventoryModule_InventoryAsset_h

#include "AbstractInventoryItem.h"
#include "RexTypes.h"

namespace Inventory
{
    using namespace RexTypes;

    class InventoryFolder;

    class InventoryAsset : public AbstractInventoryItem
    {
        Q_OBJECT
        Q_PROPERTY(QString assetReference_ READ GetAssetReference WRITE SetAssetReference)
        Q_PROPERTY(QString description_ READ GetDescription WRITE SetDescription)
        Q_PROPERTY(asset_type_t assetType_ READ GetAssetType WRITE SetAssetType)
        Q_PROPERTY(inventory_type_t inventoryType_ READ GetInventoryType WRITE SetInventoryType)

    public:
        /// Constructor.
        /// @param data_model Data model.
        /// @param id ID.
        /// @param asset_reference Asset reference.
        /// @param name Name.
        /// @param parent Parent folder.
        InventoryAsset(
            //const InventoryDataModel &data_model,
            const QString &id,
            const QString &asset_reference = "",
            const QString &name = "New Item",
            InventoryFolder *parent = 0);

        /// Destructor.
        virtual ~InventoryAsset();

        /// @return Row number of this inventory asset.
        int Row() const;

        /// Get/set for asset reference.
        QString GetAssetReference() const { return assetReference_; }
        void SetAssetReference(const QString &assetReference) { assetReference_ = assetReference; }

        /// Get/set for the description.
        void SetDescription(const QString &description) { description_ = description; }
        QString GetDescription() const { return description_;}

        /// Get/set for the description.
        void SetAssetType(const asset_type_t &asset_type) { assetType_ = asset_type; }
        asset_type_t GetAssetType() const { return assetType_;}

        /// Get/set for the description.
        void SetInventoryType(const inventory_type_t &inventory_type) { inventoryType_ = inventory_type; }
        inventory_type_t GetInventoryType() const { return inventoryType_;}

    private:
        Q_DISABLE_COPY(InventoryAsset);

        /// Asset this item reference to. Can be either uuid or url.
        QString assetReference_;

        /// Description of this inventory asset.
        QString description_;

        /// Asset type.
        asset_type_t assetType_;

        /// Inventory type.
        inventory_type_t inventoryType_;
    };
}

#endif
