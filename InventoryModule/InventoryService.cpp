/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InventoryService.h
 *  @brief  InventoryService provides inventory-related services for other modules.
 */

#include "StableHeaders.h"
#include "InventoryService.h"
#include "AbstractInventoryDataModel.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"

namespace Inventory
{

InventoryService::InventoryService(AbstractInventoryDataModel *inventory) : inventory_(inventory)
{
}

InventoryService::~InventoryService()
{
}

QList<const InventoryAsset *> InventoryService::GetAssetsByAssetType(asset_type_t type) const
{
    return static_cast<InventoryFolder *>(inventory_->GetRoot())->GetChildAssetsByAssetType(type);
}

QList<const InventoryAsset *> InventoryService::GetAssetsByInventoryType(inventory_type_t type) const
{
    return static_cast<InventoryFolder *>(inventory_->GetRoot())->GetChildAssetsByInventoryType(type);
}

const AbstractInventoryItem *InventoryService::GetItemByInventoryId(const QString &id) const
{
    return inventory_->GetChildById(id);
}

const InventoryAsset *InventoryService::GetAssetByInventoryId(const QString &id) const
{
    return dynamic_cast<InventoryAsset *>(inventory_->GetChildById(id));
}

const InventoryAsset *InventoryService::GetFirstAssetByAssetId(const QString &id)
{
    return static_cast<InventoryFolder *>(inventory_->GetRoot())->GetFirstAssetByAssetId(id);
}

QString InventoryService::GetFirstAssetNameForAssetId(const QString &id)
{
    InventoryAsset *asset = static_cast<InventoryFolder *>(inventory_->GetRoot())->GetFirstAssetByAssetId(id);
    if (asset)
        return asset->GetName();
    else
        return QString();
}

QString InventoryService::GetItemNameForInventoryId(const QString &id) const
{
    AbstractInventoryItem *item = inventory_->GetChildById(id);
    if (item)
        return item->GetName();
    else
        return QString();
}

QString InventoryService::GetAssetIdForInventoryId(const QString &id) const
{
    InventoryAsset *asset = dynamic_cast<InventoryAsset *>(inventory_->GetChildById(id));
    if (asset)
        return asset->GetAssetReference();
    else
        return QString();
}

QString InventoryService::GetFirstInventoryIdForAssetId(const QString &id)
{
    InventoryAsset *asset = static_cast<InventoryFolder *>(inventory_->GetRoot())->GetFirstAssetByAssetId(id);
    if (asset)
        return asset->GetID();
    else
        return QString();
}

}
