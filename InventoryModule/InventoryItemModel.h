// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   InventoryItemModel.h
 *  @brief  Common inventory item tree model for different inventory data models.
 */

#ifndef incl_InventoryModule_InventoryItemModel_h
#define incl_InventoryModule_InventoryItemModel_h

#include "AbstractInventoryItem.h"

#include <QAbstractItemModel>
#include <QModelIndex>
#include <QVector>
#include <QStringList>

class QMimeData;
class QItemSelection;

namespace Inventory
{
    /// Drag and Drop data structure
    ///\todo Move somewhere else, because other modules cannot depend directly on InventoryModule.
    struct DragAndDropData
    {
    public:
        DragAndDropData(QString &mimedata) : valid_(false)
        {
            QStringList list = mimedata.split(";", QString::SkipEmptyParts);
            if (list.size() == 4)
            {
                assetType = list.at(0);
                itemId = list.at(1);
                name = list.at(2);  
                assetReference = list.at(3);
                valid_ = true;
            }
            else
                valid_ = false;
        }

        /// Is the data valid (all parameters exist).
        bool IsValid() const { return valid_; }

        /// Asset type.
        QString assetType;

        /// Item id.
        QString itemId;

        /// Item name.
        QString name;

        /// Asset reference (can be uuid or url).
        QString assetReference;

    private:
        /// Is the data valid.
        bool valid_;
    };

    class AbstractInventoryDataModel;
    class InventoryTreeView;
    class InventoryWindow;

    class InventoryItemModel : public QAbstractItemModel
    {
        Q_OBJECT

        friend class InventoryTreeView;
        friend class InventoryWindow;

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

        // InventoryItemModel API

        /// Returns the type of the item at the spesific index.
        /// @param index Model index.
        /// @return Type of the item.
        AbstractInventoryItem::InventoryItemType GetItemType(const QModelIndex &index) const;

        /// Returns if item at the spesific index is library item.
        /// @param index Model index.
        /// @return Is the item at the spesific index is library item.
        bool IsLibraryItem(const QModelIndex &index) const;

        /// Inserts new empty folder to the model.
        /// @param position
        /// @param parent Index of the parent folder.
        /// @param name Name.
        bool InsertFolder(int position, const QModelIndex &parent, const QString &name);

        /// Used when moving items in inventory model.
        /// @param position Position (row) in new parent destination index.
        /// @param new_parent New parent for the item.
        /// @param item Item to be moved.
        /// @param parent_index Parent item model index, if applicable.
        bool InsertExistingItem(int position, AbstractInventoryItem *new_parent, AbstractInventoryItem* item,
            const QModelIndex &parent_index = QModelIndex());

        /// Opens inventory item (e.g. opens a folder and shows its children or opens an asset for preview).
        /// @param index Model index.
        /// @return True if the item could be opened, false otherwise.
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
        void Upload(const QModelIndex &index, QStringList &filenames, QStringList &item_names);

        /// Copies the asset reference of the item at current index to the clipboard.
        /// @param index Model inedx.
        void CopyAssetReferenceToClipboard(const QModelIndex &index);

        void CheckTreeForDirtys();

        void CheckChildrenForDirtys(QList<AbstractInventoryItem*> children);

    signals:
        void UploadStarted(const QString &filename);

        void IndexModelIsDirty(const QModelIndex &index_model);

    private:
        /// Sets up view from data.
        void SetupModelData();

        /// @param index Model index of the wanted item.
        /// @return pointer to inventory item.
        AbstractInventoryItem *GetItem(const QModelIndex &index) const;

        /// Data model pointer.
        AbstractInventoryDataModel *dataModel_;

        /// Does this model use trash folder.
        bool useTrash_;

        /// Item move flag.
        bool itemMoveFlag_;

        /// List used temporarily id's of items to be moved in the inventory model.
        QVector<QString> itemsToBeMoved_;
    };
}

#endif
