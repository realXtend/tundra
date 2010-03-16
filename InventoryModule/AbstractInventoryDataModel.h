// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   AbstractInventoryDataModel.h
 *  @brief  Abstract inventory data model, pure virtual class. Inherit this class to create your own inventory
 *          data models to use in InventoryItemModel.
 */

#ifndef incl_InventoryModule_AbstractInventoryDataModel_h
#define incl_InventoryModule_AbstractInventoryDataModel_h

#include "AbstractInventoryItem.h"
#include "WorldStream.h"

#include <QObject>

// Few useful defines.
#define STD_TO_QSTR(p) QString(p.c_str())
#define QSTR_TO_UUID(p) RexUUID(p.toStdString())

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
        virtual AbstractInventoryItem *GetChildFolderById(const QString &searchId) const = 0;

        /// @return First item by the requested id or null if the item isn't found.
        virtual AbstractInventoryItem *GetChildAssetById(const QString &searchId) const = 0;

        /// @return Pointer to the requested item, or null if not found.
        virtual AbstractInventoryItem *GetChildById(const QString &searchId) const = 0;

        /// Returns folder by requested id, or creates a new one if the folder doesnt exist,
        /// or returns null if the parent folder is invalid.
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

    public slots:
        /// Request inventory descendents for spesific folder from the server.
        /// @param item Folder.
        /// @return True, if the folder could be opened.
        virtual bool FetchInventoryDescendents(AbstractInventoryItem *item) = 0;

        /// Notifies server about item move operation.
        /// @item Inventory item.
        virtual void NotifyServerAboutItemMove(AbstractInventoryItem *item) = 0;

        /// Notifies server about item move operation.
        /// @item Inventory item.
        virtual void NotifyServerAboutItemCopy(AbstractInventoryItem *item) = 0;

        /// Notifies server about item remove operation.
        /// @item Inventory item.
        virtual void NotifyServerAboutItemRemove(AbstractInventoryItem *item) = 0;

        /// Notifies server about item update operation(e.g. name changed).
        /// @item Inventory item.
        virtual void NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name) = 0;

        /// Opens inventory item.
        /// @param Inventory item.
        /// @return True if the item at spesific index could be opened, false otherwise.
        virtual bool OpenItem(AbstractInventoryItem *item) = 0;

        /// Uploads file.
        /// @param filename Filename.
        /// @param parent_folder Destination folder in inventory for upload.
        virtual void UploadFile(const QString &filename, AbstractInventoryItem *parent_folder) = 0;

        /// Uploads multiple files.
        /// @param filenames List of filenames.
        /// @param item_names List of names for the items.
        /// @param parent_folder Destination folder in inventory for upload.
        virtual void UploadFiles(QStringList &filenames, QStringList &item_names, AbstractInventoryItem *parent_folder) = 0;

        /// Uploads multiple files, uses data buffers.
        /// @param filenames List of filenames.
        /// @param parent_folder Destination folder in inventory for upload.
        virtual void UploadFilesFromBuffer(QStringList &filenames, QVector<QVector<uchar> > &buffers,
            AbstractInventoryItem *parent_folder) = 0;

        /// Downloads asset/file from inventory.
        /// @param store_folder Store folder.
        /// @param selected_item Selected item in inventory.
        virtual void DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item) = 0;

        /// @return Inventory root folder.
        virtual AbstractInventoryItem *GetRoot() const = 0;

        /// @return Inventory trash folder.
        virtual AbstractInventoryItem *GetTrashFolder() const = 0;

        /// @return Inventory trash folder.
        virtual bool GetUseTrashFolder() const = 0;

    signals:
        /// Indicates that multiupload has started.
        /// @param file_count Number of files to be uploaded.
        void MultiUploadStarted(size_t file_count);

        /// Indicates that asset upload has started.
        /// @param filename Filename.
        void UploadStarted(const QString &filename);

        /// Indicates that asset upload has failed.
        /// @param filename Filename.
        void UploadFailed(const QString &filename);

        /// Indicates that asset upload has completed.
        /// @param filename Filename.
        void UploadCompleted(const QString &filename);

        /// Indicates that multiupload has completed.
        void MultiUploadCompleted();

        /// Indicates that asset download has started.
        /// @param asset_id
        /// @param name
        void DownloadStarted(const QString &asset_id, const QString &name);

        /// Indicates that asset download has aborted.
        /// @param asset_id
        void DownloadAborted(const QString &asset_id);

        /// Indicates that asset download is completed.
        /// @param asset_id
        void DownloadCompleted(const QString &asset_id);

        /// This signal is emitted to show notification on the window.
        /// @param message Message to be shown.
        /// @param int how long until notification is autohidden
        void Notification(QString message, int hide_time);

    private:
        Q_DISABLE_COPY(AbstractInventoryDataModel);
    };
}
#endif
