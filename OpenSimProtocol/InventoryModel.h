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
        InventoryModel::InventoryModel();

        /// Destructor
        virtual ~InventoryModel();

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

        ///
        bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

        /// @return First folder by the requested name or null if the folder isn't found.
        InventoryFolder *GetFirstChildFolderByName(const char *name) const;

        /// @return 
        InventoryFolder *GetChildFolderByID(const RexUUID &searchId) const;

        /// @return Pointer to "My Inventory" folder or null if not found.
        InventoryFolder *GetMyInventoryFolder() const;

        /// @return 
        InventoryFolder *GetOrCreateNewFolder(const RexUUID &id, InventoryFolder &parent);

        /// @return Inventory root folder.
        InventoryFolder *GetRoot() { return inventoryTreeRoot_; }

    private:
        ///
//        void SetupModelData(Inventory *inventory, InventoryFolder *parent);

        ///
        InventoryItemBase *getItem(const QModelIndex &index) const;

        /// Inventory tree root folder.
        InventoryFolder *inventoryTreeRoot_;
    };
}

#endif
