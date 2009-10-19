// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryItemModel.cpp
 *  @brief Common view for different inventory data models.
 */

#include "StableHeaders.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "RexUUID.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>

namespace Inventory
{

InventoryItemModel::InventoryItemModel(AbstractInventoryDataModel *dataModel) :
    dataModel_(dataModel)
{
}

InventoryItemModel::~InventoryItemModel()
{
    SAFE_DELETE(dataModel_);
}

QVariant InventoryItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    AbstractInventoryItem *item = GetItem(index);
    return QVariant(item->GetName().toStdString().c_str());
}

bool InventoryItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    AbstractInventoryItem *item = GetItem(index);

    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    if (folder)
        if(!folder->IsEditable())
            return false;

    if(item->GetName() == value.toString())
        return false;

    item->SetName(value.toString());

    ///\todo is this needed anymore?
    //emit dataChanged(index, index);

    // Notify server.
    dataModel_->NotifyServerAboutItemUpdate(item);

    return true;
}

QVariant InventoryItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(dynamic_cast<InventoryFolder *>(dataModel_->GetRoot())->GetName()); 

    return QVariant();
}

/*
Qt::ItemFlags InventoryItemModel::flags(const QModelIndex& index)const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled;
    if (mColumns.at(index.column()).isEditable)
        flags |= Qt::ItemIsEditable;

    return flags;
}
*/

Qt::ItemFlags InventoryItemModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsDropEnabled;

    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(index));
    if (folder)
        if (!folder->IsEditable())
            return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    // | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

Qt::DropActions InventoryItemModel::supportedDropActions() const
{
    ///\todo: | QtCopyAction?
    return Qt::MoveAction; 
}

/*
QStringList InventoryItemModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list";
    return types;
}
*/

/*
QMimeData *DragDropListModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach (QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            QString text = data(index, Qt::DisplayRole).toString();
            stream << text;
        }
    }

    mimeData->setData("application/vnd.text.list", encodedData);
    return mimeData;
    }
*/

/*
bool DragDropListModel::dropMimeData(const QMimeData *data, Qt::DropAction action,
    int row, int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.text.list"))
        return false;

    if (column > 0)
        return false;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("application/vnd.text.list");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);
    QStringList newItems;
    int rows = 0;

    while (!stream.atEnd())
    {
        QString text;
        stream >> text;
        newItems << text;
        ++rows;
    }

    insertRows(beginRow, rows, QModelIndex());
    foreach (QString text, newItems)
    {
        QModelIndex idx = index(beginRow, 0, QModelIndex());
        setData(idx, text);
        beginRow++;
    }

     return true;
}
*/

QModelIndex InventoryItemModel::index(int row, int column, const QModelIndex &parent) const
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

bool InventoryItemModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    ///\todo Make work for assets also?
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    dataModel_->GetOrCreateNewFolder(STD_TO_QSTR(RexTypes::RexUUID::CreateRandom().ToString()), *parentFolder, "New Folder");
    endInsertRows();

    return true;
}

bool InventoryItemModel::insertRows(int position, int rows, const QModelIndex &parent, InventoryItemEventData *item_data)
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
        newAsset->SetInventoryType(item_data->inventoryType);
        newAsset->SetAssetType(item_data->assetType);
    }

    endInsertRows();

    return true;
}

bool InventoryItemModel::removeRows(int position, int rows, const QModelIndex &parent)
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

QModelIndex InventoryItemModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractInventoryItem *childItem = GetItem(index);
    InventoryFolder *parentItem = static_cast<InventoryFolder *>(childItem->GetParent());
    if (parentItem == static_cast<InventoryFolder *>(dataModel_->GetRoot()))
        return QModelIndex();

    return createIndex(parentItem->Row(), 0, parentItem);
}

int InventoryItemModel::rowCount(const QModelIndex &parent) const
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

int InventoryItemModel::columnCount(const QModelIndex &parent) const
{
    ///\note We probably won't have more than one column.
    //if (parent.isValid())
    //    return static_cast<InventoryAsset *>(parent.internalPointer())->ColumnCount();

    //return dynamic_cast<InventoryFolder *>(dataModel_->GetRoot())->ColumnCount();
    return 1;
}

void InventoryItemModel::FetchInventoryDescendents(const QModelIndex &index)
{
    AbstractInventoryItem *item = GetItem(index);
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    if (!folder)
        return;

    // Send FetchInventoryDescendents only if the folder is "dirty".
    if (!folder->IsDirty())
        return;

    dataModel_->FetchInventoryDescendents(item);

    folder->SetDirty(true);
}

AbstractInventoryItem *InventoryItemModel::GetItem(const QModelIndex &index) const
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
