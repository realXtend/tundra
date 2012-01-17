// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "AssetAPI.h"
#include "IAssetStorage.h"

class QNetworkReply;
class QBuffer;
class QNetworkAccessManager;

/// Represents a network source storage for assets.
class HttpAssetStorage : public IAssetStorage
{
    Q_OBJECT

public:
    HttpAssetStorage();
    
    QString baseAddress;
    QString storageName;
    ///\todo Evaluate if could be removed. Now both AssetAPI and HttpAssetStorage manage list of asset refs.
    QStringList assetRefs;

    /// If the HttpAssetStorage points to a filesystem that is local to this server, this specifies the absolute path of
    /// the storage.
    QString localDir;

public slots:
    /// HttpAssetStorages are trusted if they point to a web server on the local system.
    virtual bool Trusted() const;

    // Returns the current trust state of this storage.
    virtual TrustState GetTrustState() const;

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString &localName);

    /// Returns the type of this storage: "HttpAssetStorage".
    virtual QString Type() const;

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return storageName; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return baseAddress; }
    
    /// Returns all assetrefs currently known in this asset storage. Does not load the assets
    virtual QStringList GetAllAssetRefs() { return assetRefs; }
    
    /// Refresh http asset refs, issues webdav PROPFIND requests. AssetChanged signals will be emitted if new assets are found.
    virtual void RefreshAssetRefs();

    /// Serializes this storage to a string for machine transfer.
    virtual QString SerializeToString(bool networkTransfer = false) const;

    /// Adds an assetref. Emits AssetChanged() if did not exist already. Called by HttpAssetProvider
    void AddAssetRef(const QString& ref);

    /// Deletes an assetref. Emits AssetChanged() if found. Called by HttpAssetProvider
    void DeleteAssetRef(const QString& ref);
    
    /// Returns the local directory of this storage. Empty if not local.
    const QString& LocalDir() const { return localDir; }
    
private slots:
    void OnHttpTransferFinished(QNetworkReply *reply);

private:
    struct SearchRequest
    {
        QNetworkReply* reply;
    };

    /// Perform a PROPFIND search on a path in the http storage
    void PerformSearch(QString path);

    /// Get QNetworkAccessManager from the parent provider
    QNetworkAccessManager* GetNetworkAccessManager();

    /// Ongoing network requests for querying asset refs
    std::vector<SearchRequest> searches;

    friend class HttpAssetProvider;
};
