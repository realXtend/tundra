// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetStorage_h
#define incl_Asset_AssetStorage_h

#include <QObject>
#include <QString>
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
    /// Returns all assets saved in this asset storage.
    virtual std::vector<IAsset*> GetAllAssets() const { return std::vector<IAsset*>(); }

    /// Starts a new asset upload to this storage. If the given asset exists already in the storage, it is replaced.
    /// @param url The desired name for the asset.
    /// @return A pointer to the newly created transfer.
//    virtual IAssetTransfer *UploadAsset(const char *data, size_t numBytes, QString url) { return 0; }

    /// Specifies whether data can be uploaded to this asset storage.
    virtual bool Writable() const { return false; }

    /// Returns the full URL of an asset with the name 'localName' if it were stored in this asset storage.
    virtual QString GetFullAssetURL(const QString &localName) { return ""; }

    /// Returns a human-readable name for this storage. This name is not used as an ID, and may be an empty string.
    virtual QString Name() const { return ""; }

    /// Returns the address of this storage.
    virtual QString BaseURL() const { return ""; }

    /// Returns a human-readable description of this asset storage.
    virtual QString ToString() const { return Name() + " (" + BaseURL() + ")"; }
};

#endif
