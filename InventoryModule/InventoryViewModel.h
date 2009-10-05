// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file AbstractInventoryDataModel.h
 *  @brief 
 */

#ifndef incl_InventoryModule_InventoryViewModel_h
#define incl_InventoryModule_InventoryViewModel_h

#include <QAbstractItemModel>
#include <QModelIndex>

namespace Inventory
{
    class AbstractInventoryDataModel;

    class InventoryViewModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param dataModel
        InventoryViewModel(AbstractInventoryDataModel *dataModel);

        /// Destructor.
        virtual ~InventoryViewModel();

        /// QAbstractItemModel override.
        QVariant data(const QModelIndex &index, int role) const;

        /// QAbstractItemModel override.
        Qt::ItemFlags flags(const QModelIndex &index) const;

        /// QAbstractItemModel override.
        Qt::DropActions supportedDropActions() const;

        /// QAbstractItemModel override.
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        /// QAbstractItemModel override.
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        /// Used for inserting new childs to the inventory tree model.
        bool insertRows(int position, int rows, const QModelIndex &parent);

        ///
//      InventoryFolder *InsertRow(int position, const QModelIndex &parent);

        /// QAbstractItemModel override.
        /// Used for removing childs to the inventory tree model.
        bool removeRows(int position, int rows, const QModelIndex &parent);

        /// QAbstractItemModel override.
        QModelIndex parent(const QModelIndex &index) const;

        /// QAbstractItemModel override.
        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        int columnCount(const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        private:
            AbstractInventoryDataModel *dataModel_;
    };
}

#endif
