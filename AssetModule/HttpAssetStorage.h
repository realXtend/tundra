// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_HttpAssetStorage_h
#define incl_Asset_HttpAssetStorage_h

#include "AssetAPI.h"
#include "IAssetStorage.h"

class QNetworkReply;
class QBuffer;
class QNetworkAccessManager;

struct SearchRequest
{
    QNetworkReply* reply;
};

class HttpAssetStorage : public IAssetStorage
{
Q_OBJECT

public:
    QString baseAddress;
    QString storageName;
    QStringList assetRefs;

public slots:
    /// Specifies whether data can be uploaded to this asset storage.
    virtual bool Writable() const { return false; }

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString &localName) { return GuaranteeTrailingSlash(baseAddress) + localName; }

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return storageName; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return baseAddress; }
    
    /// Returns all assetrefs currently known in this asset storage. Does not load the assets
    virtual QStringList GetAllAssetRefs() { return assetRefs; }
    
    /// Refresh http asset refs, issues webdav PROPFIND requests. AssetRefsChanged() will be emitted when complete.
    virtual void RefreshAssetRefs();
    
    /// Add an assetref. Emit AssetRefsChanged() if did not exist already. Called by HttpAssetProvider
    void AddAssetRef(const QString& ref);
    
    /// Delete an assetref. Emit AssetRefsChanged() if found. Called by HttpAssetProvider
    void DeleteAssetRef(const QString& ref);
    
private slots:
    void OnHttpTransferFinished(QNetworkReply *reply);

private:
    /// Perform a PROPFIND search on a path in the http storage
    void HttpAssetStorage::PerformSearch(QString path);

    /// Get QNetworkAccessManager from the parent provider
    QNetworkAccessManager* GetNetworkAccessManager();

    /// Ongoing network requests for querying asset refs
    std::vector<SearchRequest> searches;
};

#endif
