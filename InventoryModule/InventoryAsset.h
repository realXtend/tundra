// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.h
 *  @brief A class representing asset in inventory.
 */

#ifndef incl_InventoryModule_InventoryAsset_h
#define incl_InventoryModule_InventoryAsset_h

#include "AbstractInventoryItem.h"

namespace Inventory
{
    class InventoryFolder;

    class InventoryAsset : public AbstractInventoryItem
    {
        Q_OBJECT
        Q_PROPERTY(QString assetReference_ READ GetAssetReference WRITE SetAssetReference)
        Q_PROPERTY(QString description_ READ GetDescription WRITE SetDescription)

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

    private:
        Q_DISABLE_COPY(InventoryAsset);

        /// Asset this item reference to. Can be either uuid or url.
        QString assetReference_;

        /// Description of this inventory asset.
        QString description_;
    };
}

#endif
