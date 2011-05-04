// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetTransfer_h
#define incl_Asset_IAssetTransfer_h

#include <boost/enable_shared_from_this.hpp>
#include <QObject>
#include <vector>

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include <QByteArray>

class IAssetTransfer : public QObject, public boost::enable_shared_from_this<IAssetTransfer>
{
    Q_OBJECT

public:
    IAssetTransfer()
    :cachingAllowed(true)
    {
    }

    virtual ~IAssetTransfer() {}

    /// Points to the actual asset if it has been loaded in.
    AssetPtr asset;

    /// Specifies the full source URL where this asset was loaded from.
    AssetReference source;

    /// Specifies the actual type of the asset.
    QString assetType;

    /// Specifies the internal resource name. Will not be filled until the resource is loaded.
    QString internalResourceName;

    /// Specifies the provider this asset is being downloaded from.
    AssetProviderWeakPtr provider;

    /// Specifies the storage this asset is being downloaded from.
    AssetStorageWeakPtr storage;

    void EmitAssetDownloaded();

    void EmitTransferSucceeded();

    void EmitAssetFailed(QString reason);

    /// Stores the raw asset bytes for this asset.
    std::vector<u8> rawAssetData;

public slots:
    /// Returns the current transfer progress in the range [0, 1].
    // float Progress() const;

    /// Contract between IAssetProvider and AssetAPI: IAssetProvider is expected to call this upon completion of the transfer,
    /// to specify whether the results of this transfer are allowed to be cached, and to specify an existing local disk source
    /// for the asset, if exists.
    /// @param cachingAllowed If true, the Asset API will create a new cached file to the system asset cache folder that will be
    ///         used if this asset is requested in the future. This cache persists on disk over application shutdown.
    /// @param diskSource_ If specified, this local filename will be used as a cache file for the resulting asset. When a reload
    ///         of this asset is requested (locally), this file path will be used to reload the asset. If allowCaching==true,
    ///         this field has no effect, as diskSource will be created to be a filename in the asset cache.
    void SetCachingBehavior(bool cachingAllowed_, QString diskSource_) { cachingAllowed = cachingAllowed_; diskSource = diskSource_; }

    QString DiskSource() const { return diskSource; }

    bool CachingAllowed() const { return cachingAllowed; }

    // Script getters for public attributes
    QByteArray GetRawData() { return QByteArray::fromRawData((const char*)&rawAssetData[0], rawAssetData.size()); }
    QString GetSourceUrl() { return source.ref; }
    QString GetAssetType() { return assetType; }
    AssetPtr GetAsset() { return asset; }

signals:
    /// Emitted when the raw byte download of this asset finishes. The asset pointer is set at this point
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when the transfer succeeded and the asset is ready to be used in the system, with all of its dependencies loaded
    void Succeeded(AssetPtr asset);

    /// Emitted when this transfer failed.
    void Failed(IAssetTransfer *transfer, QString reason);

private:
    bool cachingAllowed;

    QString diskSource;
};

/// Virtual asset transfer for assets that have already been loaded, but are re-requested
class VirtualAssetTransfer : public IAssetTransfer
{
};

#endif
