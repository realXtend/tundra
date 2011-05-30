// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <vector>
#include "AssetFwd.h"
#include <boost/enable_shared_from_this.hpp>

class IAssetStorage : public QObject, public boost::enable_shared_from_this<IAssetStorage>
{
    Q_OBJECT
public:
    virtual ~IAssetStorage() {}

    /// Points to the asset provider that is used to communicate with this storage.
    AssetProviderWeakPtr provider;

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
    void AssetRefsChanged(AssetStoragePtr storage);
};

