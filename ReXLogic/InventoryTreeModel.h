// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryTreeModel.h
 *  @brief A tree model for showing inventory contains.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#ifndef InventoryTreeModel_h
#define InventoryTreeModel_h

#include "RexUUID.h"

#include <QObject>
#include <QAbstractItemModel>

class QModelIndex;
class QVariant;

class Inventory;
class AbstractInventoryTreeItem;
class InventoryTreeFolder;

class InventoryTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    ///
    InventoryTreeModel(const QString &data, const RexTypes::RexUUID &root_id, QObject *parent = 0);

    /// Constructor.
    InventoryTreeModel(Inventory *inventory);

    /// Destructor
    virtual ~InventoryTreeModel();

    ///
//    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    ///
    QVariant data(const QModelIndex &index, int role) const;

    ///
    Qt::ItemFlags flags(const QModelIndex &index) const;

    ///
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    ///
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    ///
    QModelIndex parent(const QModelIndex &index) const;

    ///
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    ///
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

private:
    ///
    void SetupModelData(Inventory *inventory, InventoryTreeFolder *parent);

    ///
    AbstractInventoryTreeItem *getItem(const QModelIndex &index) const;

    /// Inventory tree root folder.
    InventoryTreeFolder *inventoryTreeRoot_;
};

#endif
