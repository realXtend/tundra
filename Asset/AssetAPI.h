// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetAPI_h
#define incl_Asset_AssetAPI_h

#include <QObject>
#include <vector>
#include <map>

#include "CoreTypes.h"
#include "AssetFwd.h"
/*
class IAssetProvider
{
};
*/
//class LocalAssetProvider : public IAssetProvider;
//class HttpAssetProvider : public IAssetProvider;
//class KNetAssetProvider : public IAssetProvider;

class AssetAPI : public QObject
{
    Q_OBJECT

public:
    explicit AssetAPI(Foundation::Framework *owner);

    /// Requests the given asset to be downloaded. The transfer will go to the pending transfers queue
    /// and will be processed when possible.
    /// @param assetRef The asset ID, or full URL to request.
    /// @param assetType The type of the asset to request. This can be null if the assetRef itself identifies the asset type.
    IAssetTransfer *RequestAsset(QString assetRef, QString assetType = "");

    /// Same as RequestAsset(QString assetRef, QString assetType), but provided for convenience with AssetReference type.
    IAssetTransfer *RequestAsset(const AssetReference &ref);

    /// Returns the given asset by full URL ref if it exists, or null otherwise.
    IAsset *GetAsset(QString assetRef);
    
    /// Queries if any existing Asset Storage contains an asset with the given name, and returns it.
//    IAsset *GetAssetByName(QString assetRef);

    /// Returns all assets known to the asset system.
//    std::vector<IAsset*> GetAllAssets() const;

    /// Returns the asset provider of the given type.
    template<typename T>
    boost::shared_ptr<T> GetAssetProvider();

    /// Returns all the asset providers that are registered to the Asset API.
    std::vector<Foundation::AssetProviderPtr> GetAssetProviders();

    /// Returns the known asset storage instances in the system.
    std::vector<IAssetStorage*> GetAssetStorages();

    /// Creates a new empty asset of the given type and with the given name.
//    IAsset *CreateAsset(QString assetType, QString assetRef);

    /// Removes the given asset from the system and frees up all resources related to it. The asset will
    /// stay in the disk cache for later access.
//    void DeleteAsset(IAsset *asset);

    /// Uploads an asset to an asset storage.
    /// @param filename The source file to load the asset from.
    /// @param destination The asset storage to upload the asset to.
    /// @param assetName The name to give to the asset in the storage.
    /// @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process. \note This function will never return 0, but
    ///        throws an Exception if the data that was passed in was bad.
    IAssetUploadTransfer *UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName);

    /// Uploads an asset from the given data pointer in memory to an asset storage.
    /// @param data A pointer to raw source data in memory.
    /// @param numBytes The amount of data in the data array.
    /// @param destination The asset storage to upload the asset to.
    /// @param assetName The name to give to the asset in the storage.
    /// @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process. \note This function will never return 0, but
    ///        throws an Exception if the data that was passed in was bad.
    IAssetUploadTransfer *UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<IAssetTransfer*> PendingTransfers() const;

    /// This function is implemented for legacy purposes to help transition period to new Asset API. Will be removed. Do NOT call this. -jj
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

private:

    /// This is implemented for legacy purposes to help transition period to new Asset API. Will be removed. -jj
    std::map<request_tag_t, IAssetTransfer*> currentTransfers;

    Foundation::Framework *framework;

    std::vector<boost::shared_ptr<IAssetStorage> > storages;

    /// For now, the Asset API holds a weak reference to each provider.
//    std::vector<AssetProviderInterface*> providers;

};

#include "AssetAPI.inl"

#endif
