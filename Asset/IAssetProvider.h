// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_AssetProviderInterface_h
#define incl_Interfaces_AssetProviderInterface_h

#include "AssetFwd.h"

#include <QString>

namespace Foundation
{
    /// Asset transfer info
    /// \deprecated -jj.
    struct AssetTransferInfo
    {
        std::string id_;
        std::string type_;
        std::string provider_;
        uint size_;
        uint received_;
        uint received_continuous_;
    };
   
    /// \deprecated -jj.
    typedef std::vector<AssetTransferInfo> AssetTransferInfoVector;        
}

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
    virtual const QString &Name() = 0;

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

    /// Returns the list of all asset storages registered into this asset provider.
    virtual std::vector<AssetStoragePtr> GetStorages() const = 0;

    /// Starts an asset upload of the given file to the given storage.
    virtual IAssetUploadTransfer *UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName) = 0;

    /// Starts an asset upload from the given file in memory to the given storage.
    virtual IAssetUploadTransfer *UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName) = 0;
};


#endif
