// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreDefines.h"
#include "AssetFwd.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>

/// A base class for a database or a collection of assets in a single source.
class TUNDRACORE_API IAssetStorage : public QObject, public enable_shared_from_this<IAssetStorage>
{
    Q_OBJECT
    Q_ENUMS(ChangeType)
    Q_ENUMS(TrustState)

public:
    IAssetStorage()
    :writable(true),
    liveUpdate(true),
    liveUpload(false),
    autoDiscoverable(true),
    isReplicated(true),
    trustState(StorageAskTrust)
    {
    }

    virtual ~IAssetStorage() {}

    /// Points to the asset provider that is used to communicate with this storage.
    AssetProviderWeakPtr provider;

    /// Different type of asset changes.
    /** @see AssetChanged */
    enum ChangeType
    {
        AssetCreate,
        AssetModify,
        AssetDelete
    };

    /// Specifies how the assets from this source are to be treated security-wise.
    enum TrustState
    {
        StorageUntrusted,
        StorageTrusted,
        StorageAskTrust
    };


    void SetReplicated(bool isReplicated_) { isReplicated = isReplicated_; }

    static QString TrustStateToString(TrustState s)
    {
        if (s == StorageTrusted) return "true";
        if (s == StorageAskTrust) return "ask";
        return "false";
    }
    static TrustState TrustStateFromString(const QString &s)
    {
        if (!s.compare("true", Qt::CaseInsensitive)) return StorageTrusted;
        if (!s.compare("ask", Qt::CaseInsensitive)) return StorageAskTrust;
        return StorageUntrusted;
    }

    /// Specifies whether to trust content from this asset storage.
    /// \important This function shall never be exposed for scene scripts to use.
    void SetTrustState(TrustState trustState_) { trustState = trustState_; }

public slots:
    /// Returns all assetrefs currently known to exist in this asset storage. Does not load the assets, and does not refresh the list automatically
    /// @deprecated Do not call this. Not guaranteed to be implemented by all asset storages. Rather query for assets through AssetAPI.
    virtual QStringList GetAllAssetRefs() { return QStringList(); }

    /// Refresh assetrefs. Depending on storage type, may either finish immediately or take some time. AssetChanged signals will be emitted.
    virtual void RefreshAssetRefs() {}

    /// Starts a new asset upload to this storage. If the given asset exists already in the storage, it is replaced.
    /// @param url The desired name for the asset.
    /// @return A pointer to the newly created transfer.
//    virtual IAssetTransfer *UploadAsset(const char *data, size_t numBytes, QString url) { return 0; }

    /// Specifies whether data can be uploaded to this asset storage.
    virtual bool Writable() const { return writable; }

    /// Specifies whether the assets in the storage should be subject to live update, once loaded
    virtual bool HasLiveUpdate() const { return liveUpdate; }
    
    /// Specifies whether the assets in the storage should be automatically re-uploaded when edited in the cache
    virtual bool HasLiveUpload() const { return liveUpload; }
    
    /// Specifies whether the asset storage has automatic discovery of new assets enabled
    virtual bool AutoDiscoverable() const { return autoDiscoverable; }

    /// If we are a server, this field specifies whether this storage will be passed on to all clients when they connect.
    bool IsReplicated() const { return isReplicated; }

    /// If this function returns true, we have approved the content inside this storage to be trusted and safe.
    /// Local content is always assumed safe (content from LocalAssetProvider).
    /// Content from HttpAssetProvider is assumed safe if it points to the local system.
    /// Other storages need to be approved by other means, e.g. by the user, or by some other configuration or authentication.
    virtual bool Trusted() const { return trustState == StorageTrusted; }

    // Returns the current trust state of this storage.
    virtual TrustState GetTrustState() const { return trustState; }

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString & UNUSED_PARAM(localName)) { return ""; }

    /// Returns the type identifier for this storage type, e.g. "LocalAssetStorage" or "HttpAssetStorage".
    virtual QString Type() const = 0;

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return ""; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return ""; }

    /// Returns a human-readable description of this asset storage.
    virtual QString ToString() const { return Name() + " (" + BaseURL() + ")"; }

    /// Serializes this storage to a string for machine transfer.
    /// @param networkTransfer If false, the storage configuration is to be serialized for saving the configuration to disk (all values need to be stored).
    ///                        If true, the storage configuration is to be serialized for transfer to another computer via network.
    virtual QString SerializeToString(bool networkTransfer = false) const = 0;

signals:
    /// Asset refs have changed, either as a result of refresh, or upload / delete
    ///\todo Delete, superseded by the AssetChanged signal.
    void AssetRefsChanged(AssetStoragePtr storage);

    /// Asset has changed in the the storage.
    /** @param localName Name of the asset local to the storage. Use GetFullAssetURL to get the full qualifier.
        @param diskSource If the original copy of this asset exists on the local system, this string points to the original disk copy.
        @param change Type of change. */
    void AssetChanged(QString localName, QString diskSource, IAssetStorage::ChangeType change);

protected:
    /// If true, assets can be written to the storage.
    bool writable;

    /// If true, assets in this storage are subject to live update after loading.
    bool liveUpdate;
    
    /// If true, assets in this storage are subject to reupload if edited in the asset cache.
    bool liveUpload;
    
    /// If true, storage has automatic discovery of new assets enabled.
    bool autoDiscoverable;

    /// Specifies if this storage is replicated over the network.
    bool isReplicated;

    /// Specifies whether assets from this storage are trusted security-wise.
    TrustState trustState;
};
