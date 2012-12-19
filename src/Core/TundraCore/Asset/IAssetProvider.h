// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "AssetFwd.h"
#include "CoreTypes.h"
#include "CoreDefines.h"

#include <QString>

/// A common interface for all classes that implement downloading and uploading assets via different protocols.
/** Asset providers receive asset download requests through the RequestAsset() function. It should
    return true if the asset id was of such format that the request can be handled, false if it could not be handled.

    Additionally, the asset provider can post events of the progress of an asset download. */
class TUNDRACORE_API IAssetProvider
{
public:
    IAssetProvider() {}
    virtual ~IAssetProvider() {}

    /// Returns name of asset provider for identification purposes
    virtual QString Name() = 0;

    /// Queries this asset provider whether the assetRef is a valid assetRef this provider can handle.
    /** @param assetType The type of the asset. This field is optional, and the ref itself can specify the type,
               or if the provider in question does not need the type information, this can be left blank. */
    virtual bool IsValidRef(QString assetRef, QString assetType) = 0;

    /// Request asset with assetRef and assetType from this provider.
    /** @return Initiated asset transfer. Note that this can be null. */
    virtual AssetTransferPtr RequestAsset(QString assetRef, QString assetType) = 0;
    
    /// Aborts the ongoing transfer, returns true if successful and false otherwise.
    /** Override this function in a provider implementation if it supports aborting. */
    virtual bool AbortTransfer(IAssetTransfer * UNUSED_PARAM(transfer)) { return false; }

    /// Performs time-based update of asset provider, to for example handle timeouts.
    /** The system will call this periodically for all registered asset providers, so
        it does not need to be called manually.
        @param frametime Seconds since last frame */
    virtual void Update(f64 UNUSED_PARAM(frametime)) {}

    /// Issues an asset deletion request to the asset storage and provider this asset resides in.
    /** If the asset provider supports this feature, it will delete the asset from the source. */
    virtual void DeleteAssetFromStorage(QString assetRef) = 0;

    /// Removes the storage with the given name from this provider, or returns false if it doesn't exist.
    virtual bool RemoveAssetStorage(QString UNUSED_PARAM(storageName)) { return false; }

    /// Returns the list of all asset storages registered into this asset provider.
    virtual std::vector<AssetStoragePtr> GetStorages() const = 0;

    virtual AssetStoragePtr GetStorageByName(const QString &name) const = 0;

    virtual AssetStoragePtr GetStorageForAssetRef(const QString &assetRef) const = 0;

    /// Starts an asset upload from the given file in memory to the given storage.
    /** The default implementation fails all upload attempts and returns 0 immediately. */
    virtual AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 * UNUSED_PARAM(data), size_t UNUSED_PARAM(numBytes),
        AssetStoragePtr UNUSED_PARAM(destination), const QString & UNUSED_PARAM(assetName))
    {
        return AssetUploadTransferPtr();
    }

    /// Reads the given storage string and tries to deserialize it to an asset storage in this provider.
    /** Returns a pointer to the newly created storage, or 0 if the storage string is not of the type of this asset provider. */
    virtual AssetStoragePtr TryDeserializeStorageFromString(const QString &storage, bool fromNetwork) = 0;
};
