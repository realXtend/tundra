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

class QNetworkAccessManager;
class QNetworkRequest;
class QNetworkReply;

class HttpAssetStorage;
typedef shared_ptr<HttpAssetStorage> HttpAssetStoragePtr;

// Uncomment to enable a --disable_http_ifmodifiedsince command line parameter.
// This is used to profile the performance effect the HTTP queries have on scene loading times.
// #define HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE

/// Adds support for downloading assets over the web using the 'http://' specifier.
class ASSET_MODULE_API HttpAssetProvider : public QObject, public IAssetProvider, public enable_shared_from_this<HttpAssetProvider>
{
    Q_OBJECT

public:
    explicit HttpAssetProvider(Framework *framework);
    
    virtual ~HttpAssetProvider();
    
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

#ifdef HTTPASSETPROVIDER_NO_HTTP_IF_MODIFIED_SINCE
    virtual void Update(f64 frametime);
#endif

    // DEPRECATED
    QNetworkAccessManager* GetNetworkAccessManager() const { return NetworkAccessManager(); } /**< @deprecated Use NetworkAccessManager instead. */

private slots:
    void AboutToExit();
    void OnHttpTransferFinished(QNetworkReply *reply);
    
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

    /// If true, asset requests outside any registered storages are also accepted, and will appear as
    /// assets with no storage. If false, all requests to assets outside any registered storage will fail.
    bool enableRequestsOutsideStorages;
};

