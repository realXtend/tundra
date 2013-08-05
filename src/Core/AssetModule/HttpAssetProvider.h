// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AssetModuleApi.h"
#include "IAssetProvider.h"
#include "AssetFwd.h"
#include "HttpAssetTransfer.h"
#include "HttpAssetStorage.h"

#include <QDateTime>
#include <QByteArray>
#include <QPointer>
#include <QRunnable>

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class HttpAssetStorage;
typedef shared_ptr<HttpAssetStorage> HttpAssetStoragePtr;

/// Adds support for downloading assets over the web using the 'http://' specifier.
class ASSET_MODULE_API HttpAssetProvider : public QObject, public IAssetProvider, public enable_shared_from_this<HttpAssetProvider>
{
    Q_OBJECT

public:
    explicit HttpAssetProvider(Framework *framework);
    virtual ~HttpAssetProvider();
    
    /// IAssetProvider override.
    virtual void Update(f64 frametime);

    /// Returns the name of this asset provider.
    virtual QString Name();
    
    /// Checks an asset id for validity
    /** @return true if this asset provider can handle the id */
    virtual bool IsValidRef(QString assetRef, QString assetType = "");
    
    /// Request a http asset, returns resulted transfer.
    virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType);

    /// Aborts the ongoing http transfer.
    virtual bool AbortTransfer(IAssetTransfer *transfer);
    
    /// Adds the given http URL to the list of current asset storages.
    /// Returns the newly created storage, or 0 if a storage with the given name already existed, or if some other error occurred.
    /// @param storageName An identifier for the storage. Remember that Asset Storage names are case-insensitive.
    /// @param liveUpdate Whether assets will be reloaded whenever they change. \todo For HTTP storages, this currently means only watching the disk cache changes
    /// @param autoDiscoverable Whether recursive PROPFIND queries will be immediately performed on the storage to discover assets
    /// @param liveUpload Whether assets modified in cache will be re-uploaded. \todo Not implemented
    HttpAssetStoragePtr AddStorageAddress(const QString &address, const QString &storageName, bool liveUpdate = true, bool autoDiscoverable = false, bool liveUpload = false);

    virtual std::vector<AssetStoragePtr> GetStorages() const;

    virtual AssetStoragePtr GetStorageByName(const QString &name) const;

    virtual AssetStoragePtr GetStorageForAssetRef(const QString &assetRef) const;

    /// Starts an asset upload from the given file in memory to the given storage.
    virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName);

    /// Issues a http DELETE request for the given asset.
    virtual void DeleteAssetFromStorage(QString assetRef);
    
    /// @param storageName An identifier for the storage. Remember that Asset Storage names are case-insensitive.
    virtual bool RemoveAssetStorage(QString storageName);

    virtual AssetStoragePtr TryDeserializeStorageFromString(const QString &storage, bool fromNetwork);

    QString GenerateUniqueStorageName() const;

    /// Returns the network access manager
    QNetworkAccessManager* NetworkAccessManager() const { return networkAccessManager; }

    /// Constructs a QDateTime from a HTTP date string.
    /** Can detect and parse following formats: ANSI C's asctime(), RFC 822, updated by RFC 1123 and RFC 850, obsoleted by RFC 1036. */
    static QDateTime ParseHttpDate(const QByteArray &value);

    /// Constructs a RFC 822 HTTP date string. f.ex. "Sun, 06 Nov 1994 08:49:37 GMT"
    static QByteArray CreateHttpDate(const QDateTime &dateTime);

    /// Threshold size for when to perform async cache write.
    static int AsyncCacheWriteThreshold;

    // DEPRECATED
    QNetworkAccessManager* GetNetworkAccessManager() const { return NetworkAccessManager(); } /**< @deprecated Use NetworkAccessManager instead. */

private slots:
    void AboutToExit();
    void OnHttpTransferFinished(QNetworkReply *reply);
    void OnCacheWriteCompleted(AssetTransferPtr transfer, bool cacheFileWritten);
    
private:
    Framework *framework;
    
    /// Creates our QNetworkAccessManager
    void CreateAccessManager();

    /// Add assetref to http storage(s) after successful upload or discovery
    void AddAssetRefToStorages(const QString& ref);

    /// Delete assetref from http storages after successful delete
    void DeleteAssetRefFromStorages(const QString& ref);
    
    /// Specifies the currently added list of HTTP asset storages.
    /// This array will never store null pointers.
    std::vector<HttpAssetStoragePtr> storages;

    /// The top-level Qt object that manages all network gets.
    QNetworkAccessManager *networkAccessManager;

    /// Maps each Qt Http download transfer we start to Asset API internal HttpAssetTransfer struct.
    typedef std::map<QPointer<QNetworkReply>, HttpAssetTransferPtr> TransferMap;
    TransferMap transfers;

    /// Maps each Qt Http upload transfer we start to Asset API internal HttpAssetTransfer struct.
    typedef std::map<QNetworkReply*, AssetUploadTransferPtr> UploadTransferMap;
    UploadTransferMap uploadTransfers;

    /// Completed transfers to be sent to AssetAPI.
    QList<AssetTransferPtr> completedTransfers;

    /// If true, asset requests outside any registered storages are also accepted, and will appear as
    /// assets with no storage. If false, all requests to assets outside any registered storage will fail.
    bool enableRequestsOutsideStorages;
};

/// Threaded file write operation. Used internally to store assets to cache asynchronously after a trasnfer has completed.
class ASSET_MODULE_API TransferCacheWriteOperation : public QObject, public QRunnable
{
    Q_OBJECT

public:
    TransferCacheWriteOperation(AssetTransferPtr transfer, const QString &path, const QByteArray &data);

    /// QThread override.
    virtual void run();

signals:
    void Completed(AssetTransferPtr transfer, bool cacheFileWritten);

private:
    AssetTransferPtr transfer_;
    QString path_;
    QByteArray data_;
};
