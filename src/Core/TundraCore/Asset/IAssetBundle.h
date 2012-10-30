// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include <QObject>
#include <vector>

/// Base class for all asset bundles that provide sub assets.
class TUNDRACORE_API IAssetBundle : public QObject
{
    Q_OBJECT

public:
    IAssetBundle(AssetAPI *assetAPI, const QString &type, const QString &name);

    /// The base class destructor does nothing.
    virtual ~IAssetBundle() {}
    
public slots:
    /// Returns true if this asset bundle is loaded.
    virtual bool IsLoaded() const = 0;

    /// Returns if the asset bundle requires a disk source to operate.
    /** @note Default implementation returns false. */
    virtual bool RequiresDiskSource() = 0;

    /// Loads this asset bundle from the disk source.
    /** @return Return true if loading can be done from disk source, false otherwise eg. if disk source is empty.
        @note Implementations need to emit Loaded when completed. Do not emit Failed if you return false, Failed signal is reserved
        when this function return true and asynch loading fails. */
    virtual bool DeserializeFromDiskSource() = 0;

    /// Loads this asset bundle by deserializing it from the given data.
    /** @return Return true if loading can be done from data, false otherwise.
        @note Implementations need to emit Loaded when completed. Do not emit Failed if you return false, Failed signal is reserved
        when this function return true and asynch loading fails. */
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes) = 0;

    /// Provides a sub asset data from this bundle.
    /** This function returns the raw data of the sub asset. If the asset cannot be found 
        or fails to be unpacked return a empty vector.
        @return std::vector<u8> data. */
    virtual std::vector<u8> GetSubAssetData(const QString &subAssetName) = 0;

    /// Provides a sub asset disk source from this bundle.
    /** This function returns the disk source of the specified sub asset, this function will always be queried first by AssetAPI.
        If there is no available disk source empty string is returned. After this the GetSubAssetData will be queried.
        @return Absolute disk source path if available, empty string otherwise.*/
    virtual QString GetSubAssetDiskSource(const QString &subAssetName) = 0;

    /// Returns the type of this asset bundle. The type of an asset cannot change during the lifetime of the instance of an asset.
    QString Type() const;

    /// Returns the unique name of this asset bundle. The name of an asset cannot change during the lifetime of the instance of an asset.
    QString Name() const;

    /// Specifies the file from which this asset can be reloaded, if it is unloaded in between. 
    void SetDiskSource(QString diskSource);

    /// Returns the absolute path name to the file that contains the disk-cached version of this asset.
    /** For some assets, this is the cache file containing this asset, but for assets from some providers (e.g. LocalAssetProvider),
        this is the actual source filename of the asset. */
    QString DiskSource() const;

    /// Unloads this asset bundle from memory.
    void Unload();

    /// Returns true if the asset is empty. An empty asset is unloaded, and has an empty disk source.
    bool IsEmpty() const;

signals:
    /// This signal is emitted when the bundle is loaded and ready to provide sub assets.
    void Loaded(IAssetBundle *assetBundle);

    /// This signal is emitted when the contents of this asset is unloaded.
    void Unloaded(IAssetBundle *assetBundle);

    /// This signal is emitted when a loading error occurs after the deserialize functions have returned true (asynch loading).
    void Failed(IAssetBundle *assetBundle);

protected:
    /// Private function that implements this bundles unloading. Called from Unload().
    /** @note You don't need to unload the individual assets that the bundle introduced
        if they were loaded as IAssets that AssetAPI is aware of. IAsset unloading will take care of it. */
    virtual void DoUnload() = 0;

    /// AssetAPI ptr.
    AssetAPI *assetAPI_;

    /// Specifies the type of this asset.
    QString type_;

    /// Specifies the name of this asset, which for most assets is the source URL ref of where the asset was loaded.
    QString name_;

    /// This path specifies a local filename from which this asset can be reloaded if necessary.
    QString diskSource_;
};

/// @cond PRIVATE

// Object for tracking asset bundle parent child relations. This class is for private AssetAPI usage only.
class AssetBundleMonitor : QObject
{
Q_OBJECT

public:
    AssetBundleMonitor(AssetAPI *owner, AssetTransferPtr bundleTransfer);
    ~AssetBundleMonitor();

    /// Adds transfer as a child sub asset transfer for monitoring.
    void AddSubAssetTransfer(AssetTransferPtr transfer);

    /// Returns existing transfer for a specific asset reference.
    AssetTransferPtr SubAssetTransfer(const QString &fullSubAssetRef);
    
    /// Return all tracked child sub asset transfers.
    std::vector<AssetTransferPtr> SubAssetTransfers();

    /// Return the bundles own asset transfer.
    AssetTransferPtr BundleTransfer();
    
    /// Return the bundles own asset reference.
    QString BundleAssetRef();

private:
    QString bundleRef_;
    AssetTransferPtr bundleTransfer_;
    std::vector<AssetTransferPtr> childTransfers_;
    AssetAPI *assetAPI_;

private slots:
    void BundleFailed();
};

struct SubAssetLoader
{
    QString parentBundleRef;
    AssetTransferPtr subAssetTransfer;
    
    SubAssetLoader(const QString _parentBundleRef, AssetTransferPtr _subAssetTransfer)
    {
        parentBundleRef = _parentBundleRef;
        subAssetTransfer = _subAssetTransfer;
    }
};

/// @endcond
