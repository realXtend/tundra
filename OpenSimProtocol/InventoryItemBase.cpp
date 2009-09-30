// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryItemBase.cpp
 *  @brief Base class for inventory items.
 */

#include "StableHeaders.h"
#include "InventoryItemBase.h"

namespace OpenSimProtocol
{

InventoryItemBase::InventoryItemBase() :
    itemType_(Type_Unknown),
    id_(RexTypes::RexUUID()),
    name_("Unknown"),
    editable_(true),
    parent_(0),
    index_(QModelIndex())
{
}

InventoryItemBase::InventoryItemBase(
    const InventoryItemType &type,
    const RexTypes::RexUUID &id,
    const std::string &name,
    const bool &editable,
    InventoryFolder *parent,
    const QModelIndex &index) :
    itemType_(type),
    id_(id),
    name_(name),
    editable_(editable),
    parent_(parent),
    index_(index)
{
}

InventoryItemBase::InventoryItemBase(const InventoryItemBase &rhs)
{
    itemType_ = rhs.itemType_;
    id_ = rhs.id_;
    name_ = rhs.name_;
    parent_ = rhs.parent_;
    index_ = rhs.index_;
}

InventoryItemBase &InventoryItemBase::operator =(const InventoryItemBase &rhs)
{
    itemType_ = rhs.itemType_;
    id_ = rhs.id_;
    name_ = rhs.name_;
    parent_ = rhs.parent_;
    index_ = rhs.index_;
    return *this;
}

}
