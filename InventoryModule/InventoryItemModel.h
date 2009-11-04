// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryItemModel.h
 *  @brief Common inventory item tree model for different inventory data models.
 */

#ifndef incl_InventoryModule_InventoryItemModel_h
#define incl_InventoryModule_InventoryItemModel_h

#include "AbstractInventoryItem.h"
#include "NetworkEvents.h"
#include "InventoryEvents.h"

#include <QAbstractItemModel>
#include <QModelIndex>

class QMimeData;

namespace Inventory
{
    class AbstractInventoryDataModel;

    class InventoryItemModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param dataModel Inventory data model pointer.
        InventoryItemModel(AbstractInventoryDataModel *data_model);

        /// Destructor.
        virtual ~InventoryItemModel();

        /// QAbstractItemModel override.
        Qt::ItemFlags flags(const QModelIndex &index) const;

        /// QAbstractItemModel override.
        Qt::DropActions supportedDropActions() const;

        /// QAbstractItemModel override.
        QStringList mimeTypes() const;

        /// QAbstractItemModel override.
        QMimeData *mimeData(const QModelIndexList &indexes) const;

        /// QAbstractItemModel override.
        bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);

        /// QAbstractItemModel override.
        QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        QVariant data(const QModelIndex &index, int role) const;

        /// QAbstractItemModel override.
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        /// QAbstractItemModel override.
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

        /// QAbstractItemModel override.
        bool removeRows(int position, int rows, const QModelIndex &parent);

        /// QAbstractItemModel override.
        QModelIndex parent(const QModelIndex &index) const;

        /// QAbstractItemModel override.
        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        int columnCount(const QModelIndex &parent = QModelIndex()) const;

        /// Inserts new empty folder to the model.
        /// @param position
        /// @param parent Index of the parent folder.
        /// @param name Name.
        bool InsertFolder(int position, const QModelIndex &parent, const QString &name);

        /// Used for inserting new item with spesific data to the inventory tree model.
        /// @param folder_data Data for the new folder.
        bool InsertItem(int position, const QModelIndex &parent, InventoryItemEventData *item_data);

        /// Used when moving items in inventory model.
        /// @param position
        /// @param new_parent
        /// @param item
        bool InsertExistingItem(int position, AbstractInventoryItem *new_parent, AbstractInventoryItem* item);

        /// Requests inventory descendents from server.
        /// @param index Model index.
        void FetchInventoryDescendents(const QModelIndex &index);

        /// @return Inventory data model pointer.
        AbstractInventoryDataModel *GetInventory() const { return dataModel_; }

        /// @param index Index of the wanted item.
        /// @return pointer to inventory item.
        AbstractInventoryItem *GetItem(const QModelIndex &index) const;

        /// @return Does this model use trash folder.
        bool GetUseTrash() const { return useTrash_; }

        /// Sets if this model use trash folder or not.
        void SetUseTrash(const bool &value) { useTrash_ = value; }

    public slots:
        void CurrentSelectionChanged(const QModelIndex &index);

    signals:
        void AbstractInventoryItemSelected(AbstractInventoryItem *item);

    private:
        /// Sets up view from data.
        void SetupModelData();

        /// Data model pointer.
        AbstractInventoryDataModel *dataModel_;

        /// Does this model use trash folder.
        bool useTrash_;

        /// Item move flag.
        bool itemMoveFlag_;

        /// Number of items to be moved.
        int movedItemsCount_;
    };
}

#endif
