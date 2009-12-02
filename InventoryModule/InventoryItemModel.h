// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryItemModel.h
 *  @brief  Common inventory item tree model for different inventory data models.
 */

#ifndef incl_InventoryModule_InventoryItemModel_h
#define incl_InventoryModule_InventoryItemModel_h

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVector>
#include <QStringList>

class QMimeData;
class QItemSelection;

namespace Inventory
{
    class AbstractInventoryItem;
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
        bool canFetchMore(const QModelIndex & parent) const;

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
        void fetchMore(const QModelIndex & parent) const;

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

        // AbstractInventoryDataModel API

        /// Inserts new empty folder to the model.
        /// @param position
        /// @param parent Index of the parent folder.
        /// @param name Name.
        bool InsertFolder(int position, const QModelIndex &parent, const QString &name);

        /// Used when moving items in inventory model.
        /// @param position Position (row) in new parent destination index.
        /// @param new_parent New parent for the item.
        /// @param item Item to be moved.
        bool InsertExistingItem(int position, AbstractInventoryItem *new_parent, AbstractInventoryItem* item);

        /// 
        /// @param index Model index.
        /// @return 
        bool Open(const QModelIndex &index);

        /// @return Inventory data model pointer.
        AbstractInventoryDataModel *GetInventory() const { return dataModel_; }

        /// @return Does this model use trash folder.
        bool GetUseTrash() const { return useTrash_; }

        /// Sets if this model use trash folder or not.
        void SetUseTrash(const bool &value) { useTrash_ = value; }

        /// Downloads assets.
        /// @param index store_path Store path for the downloaded files.
        /// @param selection Selected assets.
        void Download(const QString &store_path, const QItemSelection &selection);

        /// Uploads an asset.
        /// @param index Upload destination index.
        /// @param filenames List of filenames.
        void Upload(const QModelIndex &index, QStringList filenames);

    private:
        /// Sets up view from data.
        void SetupModelData();

        /// Data model pointer.
        AbstractInventoryDataModel *dataModel_;

        /// @param index Index of the wanted item.
        /// @return pointer to inventory item.
        AbstractInventoryItem *GetItem(const QModelIndex &index) const;

        /// Does this model use trash folder.
        bool useTrash_;

        /// Item move flag.
        bool itemMoveFlag_;

        /// List used temporarily id's of items to be moved in the inventory model.
        QVector<QString> itemsToBeMoved_;
    };
}

#endif
