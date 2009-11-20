// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryItemModel.cpp
 *  @brief Common inventory item tree model for different inventory data models.
 */

#include "StableHeaders.h"
#include "InventoryModule.h"
#include "InventoryItemModel.h"
#include "AbstractInventoryDataModel.h"
#include "OpenSimInventoryDataModel.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"
#include "RexUUID.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>
#include <QMimeData>
#include <QDataStream>
#include <QPointer>
#include <QIcon>
#include <QItemSelection>

namespace Inventory
{

InventoryItemModel::InventoryItemModel(AbstractInventoryDataModel *data_model) :
    dataModel_(data_model), useTrash_(false), itemMoveFlag_(false)
{
}

InventoryItemModel::~InventoryItemModel()
{
//    SAFE_DELETE(dataModel_);
}

QVariant InventoryItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AbstractInventoryItem *item = GetItem(index);

    if (role == Qt::DecorationRole)
    {
        if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
            return QIcon(":images/iconFolderClosed.png");

        if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
            return QIcon(":images/iconResource.png");
    }

    if (role != Qt::DisplayRole)
        return QVariant();

    return QVariant(item->GetName());
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

    if (item->GetName() == value.toString())
        return false;

    QString oldName = item->GetName();
    item->SetName(value.toString());

    // Is this needed anymore?
    emit dataChanged(index, index);

    // Notify server. Give updated item and old name (webdav needs this at least)
    dataModel_->NotifyServerAboutItemUpdate(item, oldName);

    return true;
}

QVariant InventoryItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return QVariant(dataModel_->GetRoot()->GetName());

    return QVariant();
}

Qt::ItemFlags InventoryItemModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled;
    if (!index.isValid())
        return flags;

    flags |= Qt::ItemIsSelectable;

    AbstractInventoryItem *item = GetItem(index);
    if (item->GetItemType() == AbstractInventoryItem::Type_Asset)
    {
        if (!item->IsLibraryItem())
            flags |= Qt::ItemIsDropEnabled;
        flags |= Qt::ItemIsDragEnabled;
    }

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        if (!item->IsLibraryItem())
            flags |= Qt::ItemIsDropEnabled;

    if (!item->IsEditable())
        return flags;

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
        flags |= Qt::ItemIsDragEnabled;

    flags |= Qt::ItemIsEditable;

    return flags;
}

Qt::DropActions InventoryItemModel::supportedDropActions() const
{
    ///\todo: | QtCopyAction?
    return Qt::MoveAction; 
}

QStringList InventoryItemModel::mimeTypes() const
{
    QStringList types;
    ///\todo Different MIME type for each asset?
    types << "application/vnd.inventory.item";
    return types;
}

QMimeData *InventoryItemModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData();
    QByteArray encodedData;
    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    foreach(QModelIndex index, indexes)
    {
        if (index.isValid())
        {
            AbstractInventoryItem *item = GetItem(index);
            stream << item->GetID();

/*
            InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
            if (folder)
                foreach(QString id, folder->GetDescendentIds())
                    stream << id;
*/
        }
    }

    mimeData->setData("application/vnd.inventory.item", encodedData);
    return mimeData;
}

bool InventoryItemModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row,
    int column, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (!data->hasFormat("application/vnd.inventory.item"))
        return false;

    if (column > 0)
        return false;

    int beginRow = 0;
    if (row != -1)
        beginRow = row;
    else if(parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QByteArray encodedData = data->data("application/vnd.inventory.item");
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    QList<AbstractInventoryItem *> itemList;
    while(!stream.atEnd())
    {
        QString id;
        stream >> id;
        AbstractInventoryItem *item = dataModel_->GetChildById(id);
        assert(item);
        itemList << item;
    }

    AbstractInventoryItem *newParent = GetItem(parent);

    foreach(AbstractInventoryItem *item, itemList)
    {
        if (InsertExistingItem(beginRow, newParent, item))
        {
            ++beginRow;
            itemsToBeMoved_ << item->GetID();
        }
    }

    itemMoveFlag_ = true;

    return true;
}

QModelIndex InventoryItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ///\todo Use AbstractInventoryItem?
    InventoryFolder *parentItem = 0;

    if (!parent.isValid())
        parentItem = static_cast<InventoryFolder *>(dataModel_->GetRoot());
    else
        parentItem = static_cast<InventoryFolder *>((AbstractInventoryItem *)(parent.internalPointer()));

    AbstractInventoryItem *childItem = parentItem->Child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();

    return QModelIndex();
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

    bool removeRow = false;
    if (itemMoveFlag_)
    {
        // We don't want to notify server if we're just moving (i.e. deleting temporarily from one location).
        int idx = itemsToBeMoved_.indexOf(childItem->GetID());
        if (idx != -1)
        {
            removeRow = true;
            itemsToBeMoved_.remove(idx);
        }
    }
    else
    {
        removeRow = true;

        if(useTrash_)
        {
            // When deleting items, we move them first to the Trash folder.
            InventoryFolder *trashFolder = static_cast<InventoryFolder *>(dataModel_->GetTrashFolder());
            if (parentFolder == trashFolder)
            {
                // If the folder is already in the trash folder, delete it for good.
                dataModel_->NotifyServerAboutItemRemove(childItem);
            }
            else
            {
                // Move to the Trash folder.
                dataModel_->NotifyServerAboutItemMove(childItem);
                InsertExistingItem(position, trashFolder, childItem);
            }
        }
        else
            dataModel_->NotifyServerAboutItemRemove(childItem);
    }

    if (itemsToBeMoved_.size() <= 0)
        itemMoveFlag_ = false;

    if (removeRow)
    {
        beginRemoveRows(parent, position, position + rows - 1);
        bool success = parentFolder->RemoveChildren(position, rows);
        endRemoveRows();
        return success;
    }

    return false;
}

bool InventoryItemModel::InsertFolder(int position, const QModelIndex &parent, const QString &name)
{
    InventoryFolder *parentFolder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!parentFolder)
        return false;

    if (parentFolder->IsLibraryItem())
        return false;

    beginInsertRows(parent, position, position /*+ rows - 1*/);

#ifdef _DEBUG
    RexUUID id;
    bool unique = false;
    while(!unique)
    {
        id.Random();
        AbstractInventoryItem *existing = dataModel_->GetChildFolderById(STD_TO_QSTR(id.ToString()));
        if (!existing)
            unique = true;
        else
            InventoryModule::LogWarning("While creating new inventory folder generated an UUID that already exists! Generating a new one...");
    }

    dataModel_->GetOrCreateNewFolder(STD_TO_QSTR(id.ToString()), *parentFolder, name);
#else
    dataModel_->GetOrCreateNewFolder(STD_TO_QSTR(RexUUID::CreateRandom().ToString()), *parentFolder, name);
#endif

    endInsertRows();

    return true;
}

bool InventoryItemModel::InsertItem(int position, const QModelIndex &parent, InventoryItemEventData *item_data)
{
    AbstractInventoryItem *parentFolder = dataModel_->GetChildFolderById(STD_TO_QSTR(item_data->parentId.ToString()));
    if (!parentFolder)
        return false;

    AbstractInventoryItem *existing = dataModel_->GetChildById(STD_TO_QSTR(item_data->id.ToString()));
    if (existing)
        return false;

    // If the inventory is not visible the index might be non-valid.
    // Happens e.g. when you upload with console command.
    ///\todo This is maybe a bit hackish. Find a better way.
    if (parent.isValid())
        beginInsertRows(parent, position, position /*+ rows - 1*/);

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

    if (parent.isValid())
        endInsertRows();

    return true;
}

bool InventoryItemModel::InsertExistingItem(int position, AbstractInventoryItem *new_parent, AbstractInventoryItem *item)
{
    if (new_parent == item->GetParent())
        return false;

    InventoryFolder *newParentFolder = dynamic_cast<InventoryFolder *>(new_parent);
    if (!newParentFolder)
        return false;

    //beginInsertRows(parent, position, position);

    if (item->GetItemType() == AbstractInventoryItem::Type_Folder)
    {
        dataModel_->GetOrCreateNewFolder(item->GetID(), *newParentFolder, item->GetName(), false);

        InventoryFolder *newFolder = static_cast<InventoryFolder *>(dataModel_->GetOrCreateNewFolder(
            item->GetID(), *newParentFolder, item->GetName(), false));
        //newFolder->SetDirty(true);

        // When moving folders with descendents, we don't need to notify server about every descendent,
        // just the root item we're moving i.e. when the current parent is different from the new parent.
        InventoryFolder *currentParent = static_cast<InventoryFolder *>(item->GetParent());
        if (currentParent != newParentFolder)
            dataModel_->NotifyServerAboutItemMove(newFolder);

        /*
        if (newFolder->HasChildren())
        {
            AbstractInventoryItem childItem = newFolder->Child(0);
            InsertExistingItem(0, newFolder, item);
        }
        */
    }

    if (item->GetItemType()== AbstractInventoryItem::Type_Asset)
    {
        InventoryAsset *oldAsset= static_cast<InventoryAsset *>(item);

        if (oldAsset->IsLibraryItem())
        {
            // Library asset can only be copied, moving not possible.
            // Server is authorative for copy operation so we don't create the new asset right here.
            // If the copy as legal, server sends us packet and we create the asset after that.
            InventoryAsset newTempAsset(oldAsset->GetID(), oldAsset->GetAssetReference(), oldAsset->GetName(),
                newParentFolder);

            dataModel_->NotifyServerAboutItemCopy(&newTempAsset);
        }
        else
        {
            InventoryAsset *newAsset = static_cast<InventoryAsset *>(dataModel_->GetOrCreateNewAsset(
                oldAsset->GetID(), oldAsset->GetAssetReference(), *newParentFolder, oldAsset->GetName()));
            newAsset->SetDescription(oldAsset->GetDescription());
            newAsset->SetInventoryType(oldAsset->GetInventoryType());
            newAsset->SetAssetType(oldAsset->GetAssetType());

            // When moving folders with descendents, we don't need to notify server about every descendent,
            // just the root item we're moving i.e. when the current parent is different from the new parent.
            InventoryFolder *currentParent = static_cast<InventoryFolder *>(item->GetParent());
            if (currentParent != newParentFolder)
                dataModel_->NotifyServerAboutItemMove(newAsset);
        }
    }

    //endInsertRows();

    return true;
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
    //InventoryFolder *folder = dynamic_cast<InventoryFolder *>(item);
    //if (!folder)
//        return;

    // Fetch inventory descendents only if the folder is "dirty".
//    if (!folder->IsDirty())
//        return;

    dataModel_->FetchInventoryDescendents(item);

//    folder->SetDirty(false);
}

void InventoryItemModel::Download(const QString &store_path, const QItemSelection &selection)
{
    QListIterator<QModelIndex> it(selection.indexes());
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        InventoryAsset *asset = dynamic_cast<InventoryAsset *>(GetItem(index));
        if (asset)
        {
            InventoryModule::LogInfo("Requesting item " + asset->GetName().toStdString() + " for storage from the server");
//            assetDownloadRequests_[asset->GetAssetReference()] = asset->GetName();
            dataModel_->DownloadFile(store_path, asset);
        }
    }
}

void InventoryItemModel::Upload(const QModelIndex &index, QStringList filenames)
{
    if(!index.isValid())
        return;

    AbstractInventoryItem *parentItem = GetItem(index);
    assert(parentItem);

    QStringListIterator it(filenames);
    while(it.hasNext())
    {
        QString filename = it.next();
        if (!filename.isEmpty())
            dataModel_->UploadFile(filename, parentItem);
    }
}

/*
void InventoryItemModel::CurrentSelectionChanged(const QModelIndex &index)
{
    emit(AbstractInventoryItemSelected(GetItem(index)));
}
*/

AbstractInventoryItem *InventoryItemModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
        return static_cast<AbstractInventoryItem *>(index.internalPointer());

    return dataModel_->GetRoot();
}

}
