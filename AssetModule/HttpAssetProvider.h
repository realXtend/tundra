// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetProvider_h
#define incl_Asset_HttpAssetProvider_h

#include <boost/enable_shared_from_this.hpp>
#include "ThreadTaskManager.h"
#include "AssetModuleApi.h"
#include "IAssetProvider.h"
#include "AssetFwd.h"
#include "HttpAssetTransfer.h"
#include "HttpAssetStorage.h"

#include <QNetworkReply>
class QNetworkAccessManager;
class QNetworkRequest;

class HttpAssetStorage;
typedef boost::shared_ptr<HttpAssetStorage> HttpAssetStoragePtr;

/// HttpAssetProvider adds support for downloading assets that have the http:// protocol specifier in them.
class ASSET_MODULE_API HttpAssetProvider : public QObject, public IAssetProvider, public boost::enable_shared_from_this<HttpAssetProvider>
{
    Q_OBJECT;

public:
    explicit HttpAssetProvider(Foundation::Framework *framework);
    
    virtual ~HttpAssetProvider();
    
    /// Returns the name of this asset provider.
    virtual QString Name();
    
    /// Checks an asset id for validity
    /** @return true if this asset provider can handle the id */
    virtual bool IsValidRef(QString assetRef, QString assetType = "");
            
    virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType);
            
    /// Adds the given http URL to the list of current asset storages.
    void AddStorageAddress(const std::string &address, const std::string &storageName);

    virtual std::vector<AssetStoragePtr> GetStorages() const;

    /// Starts an asset upload from the given file in memory to the given storage.
    virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    /// Issues a http DELETE request for the given asset.
    virtual void DeleteAssetFromStorage(QString assetRef);

private slots:
    void OnHttpTransferFinished(QNetworkReply *reply);

private:
    Foundation::Framework *framework;
    
    /// Specifies the currently added list of HTTP asset storages.
    std::vector<HttpAssetStoragePtr> storages;

    /// The top-level Qt object that manages all network gets.
    QNetworkAccessManager *networkAccessManager;

    /// Maps each Qt Http download transfer we start to Asset API internal HttpAssetTransfer struct.
    typedef std::map<QNetworkReply*, HttpAssetTransferPtr> TransferMap;
    TransferMap transfers;

    /// Maps each Qt Http upload transfer we start to Asset API internal HttpAssetTransfer struct.
    typedef std::map<QNetworkReply*, AssetUploadTransferPtr> UploadTransferMap;
    UploadTransferMap uploadTransfers;

};

#endif
