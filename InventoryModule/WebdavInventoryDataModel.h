// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   WebDavInventoryDataModel.h
 *  @brief  Data model providing the WebDAV inventory model backend functionality.
 */

#ifndef incl_InventoryModule_WebDavInventoryDataModel_h
#define incl_InventoryModule_WebDavInventoryDataModel_h

#include "AbstractInventoryDataModel.h"
#include "InventoryFolder.h"

#include "PythonQt.h"

namespace Inventory
{
    class WebDavInventoryDataModel : public AbstractInventoryDataModel
    {
        Q_OBJECT

    public:
        /// Constructor
        /// @param identityUrl
        /// @param hostUrl
        WebDavInventoryDataModel(const QString &identity, const QString &host, const QString &password);

        /// Destructor.
        virtual ~WebDavInventoryDataModel();

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetFirstChildFolderByName(const QString &searchName) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildFolderById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildAssetById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetChildById(const QString &searchId) const;

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetOrCreateNewFolder(const QString &id, AbstractInventoryItem &parentFolder,
            const QString &name = "New Folder", const bool &notify_server = true);

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetOrCreateNewAsset(const QString &inventory_id, const QString &asset_id,
            AbstractInventoryItem &parentFolder, const QString &name = "New Asset");

    public slots:
        /// AbstractInventoryDataModel override.
        bool FetchInventoryDescendents(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemMove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemCopy(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemRemove(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void NotifyServerAboutItemUpdate(AbstractInventoryItem *item, const QString &old_name);

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetRoot() const { return rootFolder_; }

        /// AbstractInventoryDataModel override.
        AbstractInventoryItem *GetTrashFolder() const { return 0; };

        /// AbstractInventoryDataModel override.
        bool OpenItem(AbstractInventoryItem *item);

        /// AbstractInventoryDataModel override.
        void UploadFile(const QString &filename, AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        void UploadFiles(QStringList &filenames, QStringList &item_names, AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        void UploadFilesFromBuffer(QStringList &filenames, QVector<QVector<uchar> > &buffers,
            AbstractInventoryItem *parent_folder);

        /// AbstractInventoryDataModel override.
        void DownloadFile(const QString &store_folder, AbstractInventoryItem *selected_item);

        /// AbstractInventoryDataModel override.
        bool GetUseTrashFolder() const { return false; }

    private:
        Q_DISABLE_COPY(WebDavInventoryDataModel);

        // Init PythonQt for usage
        bool InitPythonQt();

        // Upload data from buffer
        void UploadBuffer(const QString &filename, QByteArray& buffer, AbstractInventoryItem *parent_folder);
        
        /// Fetch webdav inventory url with users identity
        /// @return bool true if succeeded otherwise false
        bool FetchWebdavUrlWithIdentity();

        /// Fetch initial root folder
        void FetchRootFolder();

        /// Fallback on error situations
        void ErrorOccurredCreateEmptyRootFolder();

        /// Validate folder path, prepares it for pthon webdav library usage
        /// @param path Path to be validated.
        QString ValidateFolderPath(QString path);

        /// Related urls to store for fetching webdav url and accessing webdav
        QString webdav_identity_;
        QString webdav_host_;
        QString webdav_password_;
        QString fetched_webdav_identity_;
        QString fetched_webdav_host_;
        
        /// The root folder.
        InventoryFolder *rootFolder_;

        /// Pointer to PythonQt main module
        PythonQtObjectPtr pythonQtMainModule_;

        /// WebDAV client pointer.
        PythonQtObjectPtr webdavclient_;
    };
}

#endif // incl_InventoryModule_WebDavInventoryDataModel_h
