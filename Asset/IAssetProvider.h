// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetProviderInterface_h
#define incl_Interfaces_AssetProviderInterface_h

#include "AssetFwd.h"
#include "CoreTypes.h"

#include <QString>

/** Asset provider. Can be registered to the AssetService to add possibility of downloading assets by different
    means (legacy UDP, http etc.)
    
    Asset providers receive asset download requests through the RequestAsset() function. It should
    return true if the asset id was of such format that the request can be handled (for example
    a valid UUID for legacy UDP assets), false if it could not be handled.
    
    When an asset download is ready, the asset provider has the responsibility of storing it to the asset cache
    by calling StoreAsset() function in the AssetService.
    
    Additionally, the asset provider can post events of the progress of an asset download.
 */
class IAssetProvider
{
public:
    IAssetProvider() {}
    virtual ~IAssetProvider() {}   

    /// Returns name of asset provider for identification purposes
    virtual QString Name() = 0;

    /// Queries this asset provider whether the assetRef is a valid assetRef this provider can handle.
    /// @param assetType The type of the asset. This field is optional, and the ref itself can specify the type,
    ///        or if the provider in question does not need the type information, this can be left blank.
    virtual bool IsValidRef(QString assetRef, QString assetType) = 0;

    virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType) = 0;

    /// Performs time-based update of asset provider, to for example handle timeouts.
    /** The asset service will call this periodically for all registered asset providers, so
        it does not need to be called manually.
        \param frametime Seconds since last frame */
    virtual void Update(f64 frametime) {}

    /// Issues an asset deletion request to the asset storage and provider this asset resides in.
    /// If the asset provider supports this feature, it will delete the asset from the source.
    virtual void DeleteAssetFromStorage(QString assetRef) {}

    /// Add a storage for this provider, returns the created storage
    /// @param location Location of the new storage
    /// @param name of the new storage
    /// @return AssetStoragePtr The created provider
    virtual AssetStoragePtr AddStorage(const QString &location, const QString &name) { return AssetStoragePtr(); }

    /// Returns the list of all asset storages registered into this asset provider.
    virtual std::vector<AssetStoragePtr> GetStorages() const = 0;

    /// Starts an asset upload from the given file in memory to the given storage.
    /// The default implementation fails all upload attempts and returns 0 immediately.
    virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName) { return AssetUploadTransferPtr(); }
};


#endif
