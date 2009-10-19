// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AbstractInventoryDataModel.h
 *  @brief  Abstract inventory data model, pure virtual class. Inherit this class to create your own inventory
 *          data models to use in InventoryItemModel.
 */

#ifndef incl_InventoryModule_AbstractInventoryDataModel_h
#define incl_InventoryModule_AbstractInventoryDataModel_h

#include "AbstractInventoryItem.h"

#include <QObject>

// Few useful defines.
#define STD_TO_QSTR(p) QString(p.c_str())
#define QSTR_TO_UUID(p) RexTypes::RexUUID(p.toStdString())

namespace Inventory
{
    /// Pure virtual class.
    class AbstractInventoryDataModel : public QObject
    {
        Q_OBJECT

    public:
        /// Default constructor.
        AbstractInventoryDataModel() {}

        /// Destructor.
        virtual ~AbstractInventoryDataModel() {}

        /// @return First folder by the requested name or null if the folder isn't found.
        virtual AbstractInventoryItem *GetFirstChildFolderByName(const QString &name) const = 0;

        /// @return First folder by the requested id or null if the folder isn't found.
        virtual AbstractInventoryItem *GetChildFolderByID(const QString &searchId) const = 0;

        /// Returns folder by requested id, or creates a new one if the folder doesnt exist.
        /// @param id ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created folder.
        virtual AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name = "New Folder", const bool &notify_server = true) = 0;

        /// Returns asset requested id, or creates a new one if the folder doesnt exist.
        /// @param inventory_id Inventory ID.
        /// @param asset_id Asset ID.
        /// @param parent Parent folder.
        /// @return Pointer to the existing or just created asset.
        virtual AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name) = 0;

        /// Request inventory descendents for spesific folder from the server.
        /// @param folder Folder.
        virtual void FetchInventoryDescendents(AbstractInventoryItem *folder) = 0;

        /// Notifies server about item removal.
        /// @item Inventory item.
        virtual void NotifyServerAboutItemRemoval(AbstractInventoryItem *item) = 0;

        /// Notifies server about item update (e.g. name changed).
        /// @item Inventory item.
        virtual void NotifyServerAboutItemUpdate(AbstractInventoryItem *item) = 0;

        /// @return Inventory root folder.
        virtual AbstractInventoryItem *GetRoot() const = 0;

    private:
        Q_DISABLE_COPY(AbstractInventoryDataModel);
    };
}
#endif
