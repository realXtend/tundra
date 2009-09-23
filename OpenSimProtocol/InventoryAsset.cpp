// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryAsset.h
 *  @brief 
 */

#include "StableHeaders.h"
#include "InventoryAsset.h"
#include "InventoryFolder.h"

namespace OpenSimProtocol
{

InventoryAsset::InventoryAsset(const RexTypes::RexUUID &id, const std::string &name, InventoryFolder *parent) :
    InventoryItemBase(Type_Asset, id, name, parent)
{
    ///\todo if parent == null insert to root or terminate?
    itemData_ << "data";
}

// virtual
InventoryAsset::~InventoryAsset()
{
}

int InventoryAsset::ColumnCount() const
{
    return itemData_.count();
}

QVariant InventoryAsset::Data(int column) const
{
    return itemData_.value(column);
}

int InventoryAsset::Row() const
{
    if (GetParent())
        return GetParent()->Children().indexOf(const_cast<InventoryAsset *>(this));

    return 0;
}

}
