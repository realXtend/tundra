// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeModel.cpp
 *  @brief A tree model for showing inventory contains.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#include "StableHeaders.h"
#include "InventoryTreeModel.h"
#include "InventoryTreeItem.h"
#include "Inventory.h"

#include <QModelIndex>
#include <QVariant>
#include <QStringList>

/*
InventoryTreeModel::InventoryTreeModel(
    const QString &data,
    const RexTypes::RexUUID &root_id,
    QObject *parent) :
    QAbstractItemModel(parent)
{
    QList<QVariant> rootData;
    rootData << "Title" << "Summary";
    inventoryTreeRoot_ = new InventoryTreeFolder(rootData, root_id);
//    SetupModelData(data.split(QString("\n")), inventoryTreeRoot_);
}
*/
InventoryTreeModel::InventoryTreeModel(Inventory *inventory)
{
    QList<QVariant> rootData;
    rootData << "Title" << "Summary";

    // We need a bogus "root of root" to get nicer tree layout for the inventory.
    RexTypes::RexUUID rootItemID;
    rootItemID.Random();
    inventoryTreeRoot_ = new InventoryTreeFolder(rootData, rootItemID, "Inventory");
}

InventoryTreeModel::~InventoryTreeModel()
{
    delete inventoryTreeRoot_;
}

int InventoryTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<InventoryTreeItem*>(parent.internalPointer())->ColumnCount();
    else
        return inventoryTreeRoot_->ColumnCount();
}

QVariant InventoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    AbstractInventoryTreeItem *item = static_cast<AbstractInventoryTreeItem *>(index.internalPointer());
    InventoryTreeFolder *i = static_cast<InventoryTreeFolder *>(index.internalPointer());
    //QList<QVariant> data;

    return i->Data(index.column());
    //return item->Data(index.column());
    //data << QString(item->GetName().c_str());
    //data << "Ttest" << "123234";
    //return data;
}

Qt::ItemFlags InventoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    ///\todo If inventory "core" folder (Textures, Objects etc.) return blaah blaah...
    //return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant InventoryTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return inventoryTreeRoot_->Data(section);

    return QVariant();
}

QModelIndex InventoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    ///\todo Use AbstractItem?
    InventoryTreeFolder *parentItem;

    if (!parent.isValid())
        parentItem = inventoryTreeRoot_;
    else
        parentItem = static_cast<InventoryTreeFolder *>(parent.internalPointer());

    AbstractInventoryTreeItem *childItem = parentItem->Child(row);
    ///\todo cast?
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex InventoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    AbstractInventoryTreeItem *childItem = static_cast<AbstractInventoryTreeItem *>(index.internalPointer());
    InventoryTreeFolder *parentItem = childItem->Parent();

    if (parentItem == inventoryTreeRoot_)
        return QModelIndex();

    return createIndex(parentItem->Row(), 0, parentItem);
}

int InventoryTreeModel::rowCount(const QModelIndex &parent) const
{
    InventoryTreeFolder *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = inventoryTreeRoot_;
    else
        parentItem = static_cast<InventoryTreeFolder *>(parent.internalPointer());

    return parentItem->ChildCount();
}

void InventoryTreeModel::SetupModelData(Inventory *inventory, InventoryTreeFolder *parent)
{
/*
    QList<InventoryTreeFolder *> parents;
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
    InventoryTreeFolder *my_inventory = new InventoryTreeFolder(columnData, root->GetID(), root->GetName(), parents.last());
    parent->AddChild(my_inventory);

    // My Inventory's children
    folders = root->GetChildren();
    for(it = folders.begin(); it != folders.end(); ++it)
        my_inventory->AddChild(new InventoryTreeFolder(columnData, it->GetID(), it->GetName(), my_inventory));
        //parents.last()->AddChild(new InventoryTreeFolder(columnData, it->GetID(), it->GetName(), parents.last()));
*/
}

/*AbstractInventoryTreeItem *TreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item) return item;
    }

    return rootItem;
}*/

