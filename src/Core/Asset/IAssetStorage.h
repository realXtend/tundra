// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>
#include "AssetFwd.h"
#include <boost/enable_shared_from_this.hpp>

/// A base class for a database or a collection of assets in a single source.
class IAssetStorage : public QObject, public boost::enable_shared_from_this<IAssetStorage>
{
    Q_OBJECT
    Q_ENUMS(ChangeType)

public:
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

public slots:
    /// Returns all assetrefs currently known to exist in this asset storage. Does not load the assets, and does not refresh the list automatically
    virtual QStringList GetAllAssetRefs() { return QStringList(); }

    /// Refresh assetrefs. Depending on storage type, may either finish immediately or take some time. AssetRefsChanged will be emitted when done
    virtual void RefreshAssetRefs() {}

    /// Starts a new asset upload to this storage. If the given asset exists already in the storage, it is replaced.
    /// @param url The desired name for the asset.
    /// @return A pointer to the newly created transfer.
//    virtual IAssetTransfer *UploadAsset(const char *data, size_t numBytes, QString url) { return 0; }

    /// Specifies whether data can be uploaded to this asset storage.
    virtual bool Writable() const { return false; }

    /// Specifies whether the assets in the storage should be subject to live update, once loaded
    virtual bool HasLiveUpdate() const { return false; }
    
    /// Specifies whether the asset storage has automatic discovery of new assets enabled
    virtual bool AutoDiscoverable() const { return false; }

    /// If this function returns true, we have approved the content inside this storage to be trusted and safe.
    /// Local content is always assumed safe (content from LocalAssetProvider).
    /// Content from HttpAssetProvider is assumed safe if it points to the local system.
    /// Other storages need to be approved by other means, e.g. by the user, or by some other configuration or authentication.
    virtual bool Trusted() const { return false; }

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString &localName) { return ""; }

    /// Returns the type identifier for this storage type, e.g. "LocalAssetStorage" or "HttpAssetStorage".
    virtual QString Type() const = 0;

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return ""; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return ""; }

    /// Returns a human-readable description of this asset storage.
    virtual QString ToString() const { return Name() + " (" + BaseURL() + ")"; }

    /// Serializes this storage to a string for machine transfer.
    virtual QString SerializeToString() const = 0;

signals:
    /// Asset refs have changed, either as a result of refresh, or upload / delete
    ///\todo Delete, superseded by the AssetChanged signal.
    void AssetRefsChanged(AssetStoragePtr storage);

    /// Asset has changed in the the storage.
    /** @param localName Name of the asset local to the storage. Use GetFullAssetURL to get the full qualifier.
        @param diskSource If the original copy of this asset exists on the local system, this string points to the original disk copy.
        @param change Type of change. */
    void AssetChanged(QString localName, QString diskSource, IAssetStorage::ChangeType change);
};
