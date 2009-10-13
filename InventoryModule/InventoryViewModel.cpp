// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryViewModel.cpp
 *  @brief Common view for different inventory data models.
 */

#include "StableHeaders.h"
#include "InventoryViewModel.h"
#include "AbstractInventoryDataModel.h"
#include "RexUUID.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>

namespace Inventory
{

InventoryViewModel::InventoryViewModel(AbstractInventoryDataModel *dataModel) :
    dataModel_(dataModel)
{
}

InventoryViewModel::~InventoryViewModel()
{
    SAFE_DELETE(dataModel_);
}

int InventoryViewModel::columnCount(const QModelIndex &parent) const
{
    ///\note We probably won't have more than one column.
    //if (parent.isValid())
    //    return static_cast<InventoryAsset *>(parent.internalPointer())->ColumnCount();

    //return dynamic_cast<InventoryFolder *>(dataModel_->GetRoot())->ColumnCount();
    return 1;
}

QVariant InventoryViewModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    AbstractInventoryItem *item = GetItem(index);
    return QVariant(item->GetName().toStdString().c_str());
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

    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(index));
    if (folder)
        if (folder->IsEditable())
            return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

Qt::DropActions InventoryViewModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QVariant InventoryViewModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
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
        parentItem = dynamic_cast<InventoryFolder *>(dataModel_->GetRoot());
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
    ///\todo Make work for assets also.
    AbstractInventoryItem *parentFolder = GetItem(parent);
    if (!parentFolder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    dataModel_->GetOrCreateNewFolder(QString(RexTypes::RexUUID::CreateRandom().ToString().c_str()), *parentFolder);
    endInsertRows();

    return true;
}

bool InventoryViewModel::insertRows(int position, int rows, const QModelIndex &parent,
    OpenSimProtocol::InventoryFolderEventData *folder_data)
{
    ///\todo Make work for assets also.
    //AbstractInventoryItem *parentFolder = GetItem(parent);
    AbstractInventoryItem *parentFolder = dataModel_->GetChildFolderByID(QString(folder_data->parentId.ToString().c_str()));
    if (!parentFolder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    AbstractInventoryItem *newFolder = dataModel_->GetOrCreateNewFolder(QString(folder_data->folderId.ToString().c_str()),
        *parentFolder, false);
    endInsertRows();

    newFolder->SetName(QString(folder_data->name.c_str()));
    ///\todo newFolder->SetType(folder_data->type);


    return true;
}

bool InventoryViewModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ///\todo Make work for assets also.
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    InventoryFolder *childFolder = dynamic_cast<InventoryFolder *>(parentFolder->Child(position));
    if (!childFolder)
        return false;

    if (!childFolder->IsEditable())
        return false;

    dataModel_->NotifyServerAboutFolderRemoval(childFolder);

    beginRemoveRows(parent, position, position + rows - 1);
    bool success = parentFolder->RemoveChildren(position, rows);
    endRemoveRows();

    return success;
}

QModelIndex InventoryViewModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractInventoryItem *childItem = GetItem(index);
    InventoryFolder *parentItem = static_cast<InventoryFolder *>(childItem->GetParent());
    if (parentItem == static_cast<InventoryFolder *>(dataModel_->GetRoot()))
        return QModelIndex();

    return createIndex(parentItem->Row(), 0, parentItem);
}

int InventoryViewModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
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
    if (role != Qt::EditRole)
        return false;

    AbstractInventoryItem *item = GetItem(index);
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    ///\todo Make work also for assets.
    if (!folder)
        return false;

    if(!folder->IsEditable())
        return false;

    item->SetName(value.toString());
    emit dataChanged(index, index);

    return true;
}

void InventoryViewModel::FetchInventoryDescendents(const QModelIndex &index)
{
    AbstractInventoryItem *item = GetItem(index);
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    if (!folder)
        return;

    ///\todo Send FetchInventoryDescendents only if our model is "dirty" (new items are uploaded)
//    if (!folder->IsDirty())
//        return;

    dataModel_->FetchInventoryDescendents(item);
}

AbstractInventoryItem *InventoryViewModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        AbstractInventoryItem *item = static_cast<AbstractInventoryItem *>(index.internalPointer());
        if (item)
            return item;
    }

    return dataModel_->GetRoot();
}

}
