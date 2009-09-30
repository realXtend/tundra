// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModel.h
 *  @brief A tree model for showing inventory contains.
 *  @note Code apadted from Qt simpletreemodel demo.
 */

#ifndef InventoryModel_h
#define InventoryModel_h

#include "OpenSimProtocolModuleApi.h"
#include "InventoryFolder.h"
#include "InventoryAsset.h"

#include <QObject>
#include <QAbstractItemModel>

namespace RexTypes
{
    class RexUUID;
}

class QModelIndex;
class QVariant;

namespace OpenSimProtocol
{
    class OSPROTO_MODULE_API InventoryModel : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        /// Constructor.
        InventoryModel();

        /// Destructor
        virtual ~InventoryModel();

        InventoryModel(const InventoryModel &rhs) { inventoryTreeRoot_ = rhs.inventoryTreeRoot_; }

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

        InventoryFolder *InsertRow(int position, const QModelIndex &parent);

        /// QAbstractItemModel override.
        /// Used for removing childs to the inventory tree model.
        bool InventoryModel::removeRows(int position, int rows, const QModelIndex &parent);

        /// QAbstractItemModel override.
        QModelIndex parent(const QModelIndex &index) const;

        /// QAbstractItemModel override.
        int rowCount(const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        int columnCount(const QModelIndex &parent = QModelIndex()) const;

        /// QAbstractItemModel override.
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolder *GetFirstChildFolderByName(const char *name) const;

        /// @return First folder by the requested id or null if the folder isn't found.
        InventoryFolder *GetChildFolderByID(const RexUUID &searchId) const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetMyInventoryFolder() const;

        /// @return Pointer to "Trash" folder or null if not found.
        InventoryFolder *GetTrashFolder() const;

        /// @param id ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created folder.
        InventoryFolder *GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent);

        /// @return Inventory tree model's root folder.
        InventoryFolder *GetRoot() { return inventoryTreeRoot_; }

        /// @param index Index of the wanted item.
        /// @return pointer to inventory item.
        InventoryItemBase *GetItem(const QModelIndex &index) const;

    private:
        /// Inventory tree root folder.
        InventoryFolder *inventoryTreeRoot_;
    };
}

#endif
