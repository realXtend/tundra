// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeItem.cpp
 *  @brief A container for items of data supplied by the inventory tree model.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#include "StableHeaders.h"

#include <QList>
#include <QVariant>
#include <QStringList>

#include "InventoryTreeItem.h"

/************** InventoryTreeFolder *****************/

InventoryTreeFolder::InventoryTreeFolder(
    const QList<QVariant> &data,
    const RexTypes::RexUUID &id,
    const std::string &name,
    InventoryTreeFolder *parent) :
    AbstractInventoryTreeItem(Type_Folder, id, name, parent)
{
    ///\todo if parent == null insert to root or terminate?
 //   itemData_ = data;
    itemData_ << name.c_str();
}

// virtual
InventoryTreeFolder::~InventoryTreeFolder()
{
    qDeleteAll(childItems_);
}

void InventoryTreeFolder::AppendChild(AbstractInventoryTreeItem *item)
{
    childItems_.append(item);
}

AbstractInventoryTreeItem *InventoryTreeFolder::Child(int row)
{
    return childItems_.value(row);
}

QList<AbstractInventoryTreeItem *> InventoryTreeFolder::Children()
{
    return childItems_;
}

int InventoryTreeFolder::ChildCount() const
{
    return childItems_.count();
}

int InventoryTreeFolder::ColumnCount() const
{
    return itemData_.count();
}

QVariant InventoryTreeFolder::Data(int column) const
{
    return itemData_.value(column);
}

int InventoryTreeFolder::Row() const
{
    if (Parent())
        return Parent()->childItems_.indexOf(const_cast<InventoryTreeFolder*>(this));

    return 0;
}

/************** InventoryTreeItem *****************/

InventoryTreeItem::InventoryTreeItem(
    const QList<QVariant> &data,
    const RexTypes::RexUUID &id,
    const std::string &name, 
    InventoryTreeFolder *parent)
: AbstractInventoryTreeItem(Type_Item, id, name, parent)
{
    ///\todo if parent == null insert to root or terminate?
    itemData_ = data;
}

// virtual
InventoryTreeItem::~InventoryTreeItem()
{
}

int InventoryTreeItem::ColumnCount() const
{
    return itemData_.count();
}

QVariant InventoryTreeItem::Data(int column) const
{
    return itemData_.value(column);
}

int InventoryTreeItem::Row() const
{
    if (Parent())
        return Parent()->Children().indexOf(const_cast<InventoryTreeItem *>(this));

    return 0;
}
