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
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    dataModel_->GetOrCreateNewFolder(STD_TO_QSTR(RexTypes::RexUUID::CreateRandom().ToString()), *parentFolder, "New Folder");
    endInsertRows();

    return true;
}

bool InventoryViewModel::insertRows(int position, int rows, const QModelIndex &parent, InventoryItemEventData *item_data)
{
    //AbstractInventoryItem *parentFolder = GetItem(parent);
    AbstractInventoryItem *parentFolder = dataModel_->GetChildFolderByID(STD_TO_QSTR(item_data->parentId.ToString()));
    if (!parentFolder)
        return false;

    ///\todo Use these signals somewhere?
    ///    emit layoutAboutToBeChanged();
    ///    emit layoutChanged();

    beginInsertRows(parent, position, position + rows - 1);

    if (item_data->item_type == IIT_Folder)
    {
        InventoryFolder *newFolder = static_cast<InventoryFolder *>(dataModel_->GetOrCreateNewFolder(
            STD_TO_QSTR(item_data->id.ToString()), *parentFolder, false));
        newFolder->SetName(STD_TO_QSTR(item_data->name));
        ///\todo newFolder->SetType(item_data->type);
        newFolder->SetDirty(true);
    }
    if (item_data->item_type == IIT_Asset)
    {
        InventoryAsset *newAsset = static_cast<InventoryAsset *>(dataModel_->GetOrCreateNewAsset(
            STD_TO_QSTR(item_data->id.ToString()), STD_TO_QSTR(item_data->assetId.ToString()),
            *parentFolder, STD_TO_QSTR(item_data->name)));
        newAsset->SetDescription(STD_TO_QSTR(item_data->description));
        ///\todo newAsset->SetInventoryType & AssetType
    }

    endInsertRows();

    return true;
}

bool InventoryViewModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    AbstractInventoryItem *childItem = parentFolder->Child(position);
    if (!childItem)
        return false;

    if (childItem->GetItemType() == AbstractInventoryItem::Type_Folder)
        if (!static_cast<InventoryFolder *>(childItem)->IsEditable())
            return false;

    dataModel_->NotifyServerAboutItemRemoval(childItem);

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

    // Send FetchInventoryDescendents only if the folder is "dirty".
    if (!folder->IsDirty())
        return;

    dataModel_->FetchInventoryDescendents(item);

    folder->SetDirty(false);
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
