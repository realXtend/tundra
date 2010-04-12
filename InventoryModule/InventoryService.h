/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryService.h
 *  @brief  InventoryService provides inventory-related services for other modules.
 */

#ifndef incl_InventoryModule_InventoryService_h
#define incl_InventoryModule_InventoryService_h

#include "RexTypes.h"

#include <QObject>
#include <QList>
#include <QString>

namespace Inventory
{
    class AbstractInventoryDataModel;
    class AbstractInventoryItem;
    class InventoryAsset;

    class InventoryService : public QObject
    {
        Q_OBJECT

    public:
        /// Constuctor.
        /// @param inventory Inventory data model.
        InventoryService(AbstractInventoryDataModel *inventory);

        /// Destructor.
        ~InventoryService();

    public slots:
        /// Returns list of assets with spesific asset type.
        /// @param type Asset type.
        QList<const InventoryAsset *> GetAssetsByAssetType(asset_type_t type) const;

        /// Returns list of assets with spesific inventory type.
        /// @param type Inventory type.
        QList<const InventoryAsset *> GetAssetsByInventoryType(inventory_type_t type) const;

        /// Returns inventory item with the requested inventory ID. Searches both assets and folders.
        /// @param id Inventory ID.
        /// @return Inventory item with the requested ID or null if not found.
        const AbstractInventoryItem *GetItemByInventoryId(const QString &id) const;

        /// Returns inventory asset with the requested ID. Searches both assets and folders.
        /// @param id Inventory ID.
        /// @return Inventory item with the requested ID or null if not found.
        const InventoryAsset *GetAssetByInventoryId(const QString &id) const;

        /// Returns the first found inventory asset with the wanted asset reference ID.
        /// @note Mutiple inventory assets can have the same asset reference.
        /// @return First asset found with the requested asset ID or null if not found.
        const InventoryAsset *GetFirstAssetByAssetId(const QString &id);

        /// Returns name of the first found asset with the requested asset ID.
        /// @param id Asset ID.
        /// @return Name of the first matching asset or null string if asset with matching ID not found.
        QString GetFirstAssetNameForAssetId(const QString &id);

        /// Returns name of inventory item with the requested inventory ID. Searches both folders and assets.
        /// @param id Inventory ID.
        /// @return Name of the inventory item or null string if asset item with matching ID not found.
        QString GetItemNameForInventoryId(const QString &id) const;

        /// Returns asset ID for the inventory item with the requested ID.
        /// @param id Inventory ID.
        /// @return Asset ID of inventory item or null string item with matching ID not found.
        QString GetAssetIdForInventoryId(const QString &id) const;

        /// Returns Inventory ID of the first found inventory item with the requested asset ID.
        /// @param id Asset ID.
        /// @return Inventory ID or null string item with matching ID not found.
        /// @note Mutiple inventory assets can have the same asset reference.
        QString GetFirstInventoryIdForAssetId(const QString &id);

    private:
        Q_DISABLE_COPY(InventoryService);
        /// Inventory data model pointer.
        AbstractInventoryDataModel *inventory_;
    };
}

#endif
