// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"
#include "IAsset.h"

#include <QObject>
#include <vector>
#include <QByteArray>

/// Represents a currently ongoing asset download operation.
class TUNDRACORE_API IAssetTransfer : public QObject, public enable_shared_from_this<IAssetTransfer>
{
Q_OBJECT

public:
    IAssetTransfer();
    virtual ~IAssetTransfer();

    /// Points to the actual asset if it has been loaded in.
    AssetPtr asset;

    /// Specifies the full source URL where this asset was loaded from.
    AssetReference source;

    /// Specifies the actual type of the asset.
    QString assetType;

    /// Specifies the internal resource name. Will not be filled until the resource is loaded.
    QString internalResourceName;

    /// Specifies the disk source type to set for the asset once this transfer completes.
    IAsset::SourceType diskSourceType;
    
    /// Specifies the provider this asset is being downloaded from.
    AssetProviderWeakPtr provider;

    /// Specifies the storage this asset is being downloaded from.
    AssetStorageWeakPtr storage;

    /// Emits Downloaded signal.
    void EmitAssetDownloaded();

    /// Emits Succeeded signal.
    void EmitTransferSucceeded();

    /// Emits Failed signal with reason.
    void EmitAssetFailed(QString reason);

    /// Stores the raw asset bytes for this asset.
    std::vector<u8> rawAssetData;

public slots:
    /// Aborts the transfer immediately. Override this function in a subclass implementation.
    /** @note Default IAssetTransfer implementation logs a not implemented warning and return false.
        @return True if abort was successful, false otherwise. */
    virtual bool Abort();

    /// Set caching behavior and disk source. Can be set after the transfer has been requested to determine disk caching.
    /** Contract between IAssetProvider and AssetAPI: IAssetProvider is expected to call this upon completion of the transfer,
        to specify whether the results of this transfer are allowed to be cached, and to specify an existing local disk source
        for the asset, if exists.
        @param cachingAllowed If true, the Asset API will create a new cached file to the system asset cache folder that will be
        used if this asset is requested in the future. This cache persists on disk over application shutdown.
        @param diskSource_ If specified, this local filename will be used as a cache file for the resulting asset. When a reload
        of this asset is requested (locally), this file path will be used to reload the asset. If allowCaching==true,
        this field has no effect, as diskSource will be created to be a filename in the asset cache. */
    void SetCachingBehavior(bool cachingAllowed, QString diskSource);

    /// Returns the disk source of this transfer.
    QString DiskSource() const;

    /// Returns the disk source type of this transfer.
    IAsset::SourceType DiskSourceType() const;
    
    /// Returns if this transfer allows caching to a disk source.
    bool CachingAllowed() const;

    /// Return the transfers raw data as a script friendly QByteArray.
    /** @note Will be empty until Downloaded is emitted */
    QByteArray RawData() const;
    
    /// Returns source URL.
    QString SourceUrl() const;
    
    /// Return asset type.
    QString AssetType() const;
    
    /// Return asset object.
    /** @note Will be null until Succeeded is emitted */
    AssetPtr Asset() const;

    /// @todo Returns the current transfer progress in the range [0, 1].
    // float Progress() const;

signals:
    /// Emitted when the raw byte download of this asset finishes. The asset pointer is set at this point.
    void Downloaded(IAssetTransfer *transfer);

    /// Emitted when the transfer succeeded and the asset is ready to be used in the system, with all of its dependencies loaded
    void Succeeded(AssetPtr asset);

    /// Emitted when this transfer failed.
    void Failed(IAssetTransfer *transfer, QString reason);

private:
    QString diskSource;
    bool cachingAllowed;
    
};

/// Virtual asset transfer for assets that have already been loaded, but are re-requested
class VirtualAssetTransfer : public IAssetTransfer
{
};
