// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.cpp
 *  @brief A class representing asset in inventory.
 */

#include "StableHeaders.h"
#include "InventoryAsset.h"
#include "InventoryFolder.h"

namespace Inventory
{

InventoryAsset::InventoryAsset(
    const QString &id,
    const QString &asset_reference,
    const QString &name,
    InventoryFolder *parent) :
    AbstractInventoryItem(Type_Asset, id, name, parent),
    assetReference_(asset_reference)
{
}

// virtual
InventoryAsset::~InventoryAsset()
{
}

/*
int InventoryAsset::ColumnCount() const
{
    return itemData_.count();
}

int InventoryAsset::Row() const
{
    if (GetParent())
        return GetParent()->Children().indexOf(const_cast<InventoryAsset *>(this));

    return 0;
}
*/

}
