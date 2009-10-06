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

InventoryAsset::InventoryAsset(
    const RexTypes::RexUUID &inventory_id,
    const RexTypes::RexUUID &asset_id,
    const std::string &name,
    InventoryFolder *parent) :
    InventoryItemBase(Type_Asset, inventory_id, name, parent)
{
    ///\todo if parent == null insert to root or terminate?
//    itemData_ << name.c_str();
    ///\ todo emit somekind of signal declaring creation of new asset?
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
*/

/*
bool InventoryAsset::SetData(int column, const QVariant &value)
{
    if (column < 0 || column >= itemData_.size())
        return false;

    if (itemData_[column] == value || value.toString().toStdString() == "")
        return false;

    itemData_[column] = value;
    return true;
}
*/

/*
Variant InventoryAsset::Data(int column) const
{
    return itemData_.value(column);
}
*/

/*int InventoryAsset::Row() const
{
    if (GetParent())
        return GetParent()->Children().indexOf(const_cast<InventoryAsset *>(this));

    return 0;
}
*/

}
