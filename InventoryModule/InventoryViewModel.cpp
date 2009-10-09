// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryViewModel.cpp
 *  @brief Common view for different inventory data models.
 */

#include "StableHeaders.h"
#include "InventoryViewModel.h"
#include "OpenSimInventoryDataModel.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>

namespace Inventory
{

InventoryViewModel::InventoryViewModel(AbstractInventoryDataModel *dataModel) :
    dataModel_(dataModel)
{
    // InventoryViewModel's root item is not the same as inventory's root item ("My Inventory" folder).
    //rootFolder_ = new InventoryFolder(RexUUID::CreateRandom(), "Inventory");
    //SetupModelData();
}

InventoryViewModel::~InventoryViewModel()
{
    delete dataModel_;
}

int InventoryViewModel::columnCount(const QModelIndex &parent) const
{
    ///\note We probably won't have more than one column.
    //if (parent.isValid())
    //    return static_cast<InventoryAsset *>(parent.internalPointer())->ColumnCount();

//    return rootFolder_->ColumnCount();
    return 1;
}

QVariant InventoryViewModel::data(const QModelIndex &index, int role) const
{
/*
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    InventoryItemBase *item = GetItem(index);
    return QVariant(item->GetName().c_str());
*/
    return QVariant();
}
/*
Qt::ItemFlags InventoryViewModel::flags(const QModelIndex& index)const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;
    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    if (mColumns.at(index.column()).isEditable)
        flags |= Qt::ItemIsEditable;

    return flags;
}
*/

Qt::ItemFlags InventoryViewModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

/*
    InventoryItemBase *item = GetItem(index);
    if (item->IsEditable())
        return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
*/
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

Qt::DropActions InventoryViewModel::supportedDropActions() const
{
    return Qt::MoveAction; // | Qt::CopyAction
}

QVariant InventoryViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
//        return rootFolder_->Data(section);
        return QVariant(dynamic_cast<InventoryFolder *>(dataModel_->GetRoot())->GetName()); 

    return QVariant();
}

QModelIndex InventoryViewModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ///\todo Use AbstractInventoryItem?
    InventoryFolder *parentItem;

    if (!parent.isValid())
        parentItem = dynamic_cast<InventoryFolder *>(dataModel_->GetRoot());//rootFolder_;
    else
        parentItem = dynamic_cast<InventoryFolder *>((AbstractInventoryItem *)(parent.internalPointer()));

    AbstractInventoryItem *childItem = parentItem->Child(row);
    ///\todo cast?
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();

    return QModelIndex();
}

bool InventoryViewModel::insertRows(int position, int rows, const QModelIndex &parent)
{
/*
    ///\todo Make work for assets also.
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!folder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    InventoryFolder *newFolder = new InventoryFolder(RexUUID::CreateRandom(), "New Folder", folder);
    folder->AddChild(newFolder);
    endInsertRows();

    return true;
*/
    return false;
}

bool InventoryViewModel::removeRows(int position, int rows, const QModelIndex &parent)
{
/*
    ///\todo Make work for assets also.
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    InventoryFolder *childFolder = dynamic_cast<InventoryFolder *>(parentFolder->Child(position));
    if (!childFolder)
        return false;

    if (!childFolder->IsEditable())
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    bool success = parentFolder->RemoveChildren(position, rows);
    endRemoveRows();

    return success;
*/
    return false;
}

QModelIndex InventoryViewModel::parent(const QModelIndex &index) const
{
/*
    if (!index.isValid())
        return QModelIndex();

    InventoryItemBase *childItem = static_cast<InventoryItemBase *>(index.internalPointer());
    InventoryFolder *parentItem = childItem->GetParent();

    if (parentItem == rootFolder_)
        return QModelIndex();

    return createIndex(parentItem->Row(), 0, parentItem);
*/
    return QModelIndex();
}

int InventoryViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
//        return rootFolder_->ChildCount();
        dynamic_cast<InventoryFolder *>(dataModel_->GetRoot())->ChildCount();

    AbstractInventoryItem *item = GetItem(parent);
    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        InventoryFolder *parentFolder = static_cast<InventoryFolder *>(item);
        return parentFolder->ChildCount();
    }

    return 0;
}

bool InventoryViewModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
/*
    if (role != Qt::EditRole)
        return false;

    InventoryItemBase *item = GetItem(index);
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    ///\todo Make work also for assets.
    if (!folder)
        return false;

    if(!folder->IsEditable())
        return false;

    bool result = folder->SetData(index.column(), value);
    //item->setData(index.column(), value);
    if (result)
        emit dataChanged(index, index);

    return true;
*/
    return false;
}

AbstractInventoryItem *InventoryViewModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        AbstractInventoryItem *item = static_cast<AbstractInventoryItem *>(index.internalPointer());
        if (item)
            return item;
    }

    //return static_cast<InventoryItemBase *>(rootFolder_);
    return dataModel_->GetRoot();
}

}
