// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AssetModuleApi.h"
#include "IAssetProvider.h"
#include "AssetFwd.h"

#include <QSet>

class LocalAssetStorage;

typedef shared_ptr<LocalAssetStorage> LocalAssetStoragePtr;

/// Provides access to files on the local file system using the 'local://' URL specifier.
class ASSET_MODULE_API LocalAssetProvider : public QObject, public IAssetProvider, public enable_shared_from_this<LocalAssetProvider>
{
    Q_OBJECT

public:
    explicit LocalAssetProvider(Framework* framework);
    
    virtual ~LocalAssetProvider();
    
    /// Returns name of asset provider
    virtual QString Name();
    
    /// Checks an asset id for validity
    /** @return true if this asset provider can handle the id */
    virtual bool IsValidRef(QString assetRef, QString assetType);
     
    /// Requests a local asset, returns resulted transfer.
    virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType);
    
    /// Aborts the ongoing local transfer.
    virtual bool AbortTransfer(IAssetTransfer *transfer);

    /// Performs time-based update 
    /** @param frametime Seconds since last frame */
    virtual void Update(f64 frametime);

    /// Deletes this asset from file.
    virtual void DeleteAssetFromStorage(QString assetRef);

    /// @param storageName An identifier for the storage. Remember that Asset Storage names are case-insensitive.
    virtual bool RemoveAssetStorage(QString storageName);

    /// Adds the given directory as an asset storage.
    /** @param directory The path name for the directory to add.
        @param storageName An identifier for the storage. Remember that Asset Storage names are case-insensitive.
        @param recursive If true, all the subfolders of the given folder are added as well.
        @param writable If true, assets can be uploaded to the storage.
        @param liveUpdate If true, assets will be reloaded when the underlying file changes.
        @param autoDiscoverable If true, a recursive directory search will be initially performed to know which assets reside inside the storage.
        Returns the newly created storage, or 0 if a storage with the given name already existed, or if some other error occurred. */
    LocalAssetStoragePtr AddStorageDirectory(QString directory, QString storageName, bool recursive, bool writable = true, bool liveUpdate = true, bool autoDiscoverable = true);

    virtual std::vector<AssetStoragePtr> GetStorages() const;

    virtual AssetStoragePtr GetStorageByName(const QString &name) const;

    virtual AssetStoragePtr GetStorageForAssetRef(const QString &assetRef) const;

    virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName);

    virtual AssetStoragePtr TryDeserializeStorageFromString(const QString &storage, bool fromNetwork);

    QString GenerateUniqueStorageName() const;

    /// Returns LocalAssetStorage for specific @c path. The @c path can be root directory of storage or any of its subdirectories.
    LocalAssetStoragePtr FindStorageForPath(const QString &path) const;

private:
    Q_DISABLE_COPY(LocalAssetProvider)

    /// Finds a path where the file localFilename can be found. Searches through all local storages.
    /// @param storage [out] Receives the local storage that contains the asset.
    QString GetPathForAsset(const QString &localFilename, LocalAssetStoragePtr *storage) const;

    /// Takes all the pending file download transfers and finishes them.
    void CompletePendingFileDownloads();

    /// Takes all the pending file upload transfers and finishes them.
    void CompletePendingFileUploads();

    /// Checks for pending file systems changes and updates 
    void CheckForPendingFileSystemChanges();

    Framework *framework;
    std::vector<LocalAssetStoragePtr> storages; ///< Asset directories to search, may be recursive or not
    std::vector<AssetUploadTransferPtr> pendingUploads; ///< The following asset uploads are pending to be completed by this provider.
    std::vector<AssetTransferPtr> pendingDownloads; ///< The following asset downloads are pending to be completed by this provider.
    QSet<QString> changedFiles; ///< Pending file changes.
    QSet<QString> changedDirectories; ///< Pending directory changes.

    /// If true, assets outside any known local storages are allowed. Otherwise, requests to them will fail.
    bool enableRequestsOutsideStorages;

private slots:
    void OnFileChanged(const QString &path);
    void OnDirectoryChanged(const QString &path);
};
