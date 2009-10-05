// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file InventoryModel.h
 *  @brief A tree model for showing inventory contains.
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

///\todo    Refactor into two different class: Inventory and InventoryModel.
///         InventoryModel has only functions dealing with indexes and rows etc.
///         Inventory has has functions with id's, folder pointers etc.

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

        InventoryModel(const InventoryModel &rhs) { rootFolder_ = rhs.rootFolder_; }

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
        InventoryFolder *InsertRow(int position, const QModelIndex &parent);

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

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolder *GetFirstChildFolderByName(const char *name) const;

        /// @return First folder by the requested id or null if the folder isn't found.
        InventoryFolder *GetChildFolderByID(const RexUUID &searchId) const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetMyInventoryFolder() const;

        /// @return Pointer to "Trash" folder or null if not found.
        InventoryFolder *GetTrashFolder() const;

        /// Returns folder by requested id, or creates a new one if the folder doesnt exist.
        /// @param id ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created folder.
        InventoryFolder *GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent);

        /// Returns asset requested id, or creates a new one if the folder doesnt exist.
        /// @param inventory_id Inventory ID.
        /// @param asset_id Asset ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created asset.
        InventoryAsset *GetOrCreateNewAsset(
            const RexUUID &inventory_id,
            const RexUUID &asset_id,
            InventoryFolder &parent,
            const std::string &name = "New Item");

        /// @return Inventory tree model's root folder.
        InventoryFolder *GetRoot() { return rootFolder_; }

        /// @param index Index of the wanted item.
        /// @return pointer to inventory item.
        InventoryItemBase *GetItem(const QModelIndex &index) const;

        /// Tells if the tree item of spesified index is editable.
        /// @param index Index of the tree item.
        /// @return True if editable, false otherwise.
        bool IsEditable(const QModelIndex &index);

    private:
        /// Inventory tree root folder.
        InventoryFolder *rootFolder_;
    };
}

#endif
