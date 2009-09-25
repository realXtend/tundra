// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModel.cpp
 *  @brief A tree model for showing inventory contains.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#include "StableHeaders.h"
#include "InventoryModel.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>

namespace OpenSimProtocol
{

InventoryModel::InventoryModel()
{
    // InventoryModel's root item is not the same as inventory's root item ("My Inventory" folder).
    inventoryTreeRoot_ = new InventoryFolder(RexUUID::CreateRandom(), "Inventory");
}

InventoryModel::~InventoryModel()
{
    delete inventoryTreeRoot_;
}

int InventoryModel::columnCount(const QModelIndex &parent) const
{
    ///\note We probably won't have more than one column.
/*
    if (parent.isValid())
        return static_cast<InventoryAsset *>(parent.internalPointer())->ColumnCount();
    else
*/
        return inventoryTreeRoot_->ColumnCount();
}

QVariant InventoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    InventoryItemBase *item = static_cast<InventoryItemBase *>(index.internalPointer());
    InventoryFolder *i = static_cast<InventoryFolder *>(index.internalPointer());
    //QList<QVariant> data;

    return i->Data(index.column());
    //return item->Data(index.column());
    //data << QString(item->GetName().c_str());
    //data << "Ttest" << "123234";
    //return data;
}

Qt::ItemFlags InventoryModel::flags(const QModelIndex &index) const
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

QVariant InventoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return inventoryTreeRoot_->Data(section);

    return QVariant();
}

QModelIndex InventoryModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ///\todo Use AbstractItem?
    InventoryFolder *parentItem;

    if (!parent.isValid())
        parentItem = inventoryTreeRoot_;
    else
        parentItem = static_cast<InventoryFolder *>(parent.internalPointer());

    InventoryItemBase *childItem = parentItem->Child(row);
    ///\todo cast?
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

bool InventoryModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    ///\todo Make work for assets also.
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!folder)
        return false;

    beginInsertRows(parent, position, position + rows - 1);
    InventoryFolder *newFolder = new InventoryFolder(RexUUID::CreateRandom(), "New Folder", folder);
    folder->AddChild(newFolder);
    endInsertRows();

    return true;
}
int xxx = 0;
InventoryFolder *InventoryModel::InsertRow(int position, const QModelIndex &parent)
{
    ///\todo Make work for assets also.
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!folder)
        return 0;

    beginInsertRows(parent, position, position);
    std::string name = "New Folder";
    name += Core::ToString(xxx);
    ++xxx;
    InventoryFolder *newFolder = new InventoryFolder(RexUUID::CreateRandom(), name, folder);
    folder->AddChild(newFolder);
    endInsertRows();

    return newFolder;
}

bool InventoryModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    ///\todo Make work for assets also.
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!folder)
        return false;

    beginRemoveRows(parent, position, position + rows - 1);
    folder->DeleteChild(folder);
    endRemoveRows();

    return true;
}

/*
void InventoryModel::DeleteChild(int position, const QModelIndex &parent)
{
    ///\todo Make work for assets also.
    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(parent));
    if (!folder)
        return 0;

    beginInsertRows(parent, position, position);
    InventoryFolder *newFolder = new InventoryFolder(RexUUID::CreateRandom(), "New Folder", folder);
    folder->AddChild(newFolder);
    endInsertRows();

    return newFolder;
}
*/

QModelIndex InventoryModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    InventoryItemBase *childItem = static_cast<InventoryItemBase *>(index.internalPointer());
    InventoryFolder *parentItem = childItem->GetParent();

    if (parentItem == inventoryTreeRoot_)
        return QModelIndex();

    return createIndex(parentItem->Row(), 0, parentItem);
}

int InventoryModel::rowCount(const QModelIndex &parent) const
{
    InventoryFolder *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = inventoryTreeRoot_;
    else
        parentItem = static_cast<InventoryFolder *>(parent.internalPointer());

    return parentItem->ChildCount();
}

bool InventoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    InventoryFolder *folder = dynamic_cast<InventoryFolder *>(GetItem(index));
    if (!folder)
        return false;

//    bool result = InventoryItemBase->setData(index.column(), value);
//    if (result)
    emit dataChanged(index, index);

    return true;
}

InventoryFolder *InventoryModel::GetFirstChildFolderByName(const char *searchName) const
{
    return inventoryTreeRoot_->GetFirstChildFolderByName(searchName);
}

InventoryFolder *InventoryModel::GetChildFolderByID(const RexUUID &searchId) const
{
    return inventoryTreeRoot_->GetChildFolderByID(searchId);
}

InventoryFolder *InventoryModel::GetMyInventoryFolder() const
{
    return inventoryTreeRoot_->GetFirstChildFolderByName("My Inventory");
}

InventoryFolder *InventoryModel::GetTrashFolder() const
{
    return inventoryTreeRoot_->GetFirstChildFolderByName("Trash");
}

InventoryFolder *InventoryModel::GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent)
{
    // Return an existing folder if one with the given id is present.
    InventoryFolder *existing = GetChildFolderByID(id);
    if (existing)
        return existing;

    // Create a new folder.
    InventoryFolder *newFolder = new InventoryFolder(id, "New Folder", &parent);
    return static_cast<InventoryFolder *>(parent.AddChild(newFolder));
}

/*
void InventoryModel::SetupModelData(Inventory *inventory, InventoryFolder *parent)
{
    QList<InventoryFolder *> parents;
    parents << parent;
    QList<QVariant> columnData;
    columnData << "Testing123";

    std::list<InventoryFolder> folders;
    std::list<InventoryFolder>::iterator it;

    InventoryFolder *root = inventory->GetRoot();
    if (!root)
        return;

    ///\todo NEEDS A GENERIC LOGIC!!!
    // My Inventory
    InventoryFolder *my_inventory = new InventoryFolder(columnData, root->GetID(), root->GetName(), parents.last());
    parent->AppendChild(my_inventory);

    // My Inventory's children
    folders = root->GetChildren();
    for(it = folders.begin(); it != folders.end(); ++it)
        my_inventory->AppendChild(new InventoryFolder(columnData, it->GetID(), it->GetName(), my_inventory));
        //parents.last()->AppendChild(new InventoryFolder(columnData, it->GetID(), it->GetName(), parents.last()));
}
*/

InventoryItemBase *InventoryModel::GetItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        InventoryItemBase *item = static_cast<InventoryItemBase *>(index.internalPointer());
        if (item)
            return item;
    }

    return static_cast<InventoryItemBase *>(inventoryTreeRoot_);
}

}
