// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetAPI_h
#define incl_Asset_AssetAPI_h

#include <QObject>
#include <vector>
#include <map>

#include "CoreTypes.h"
#include "AssetFwd.h"

class IAssetProvider
{
};

//class LocalAssetProvider : public IAssetProvider;
//class HttpAssetProvider : public IAssetProvider;
//class KNetAssetProvider : public IAssetProvider;

/*
class IAssetStorage : public QObject
{
    Q_OBJECT
public:

    /// Returns all assets saved in this asset storage.
    std::vector<IAsset*> GetAllAssets() const;

    /// Starts a new asset upload to this storage. If the given asset exists already in the storage, it is replaced.
    /// @param url The desired name for the asset.
    /// @return A pointer to the newly created transfer.
    QPointer<IAssetTransfer> UploadAsset(const char *data, size_t numBytes, QString url);

    /// Returns the address of this storage.
    QString BaseURL() const;
};

*/

class AssetAPI : public QObject
{
    Q_OBJECT

public:
    explicit AssetAPI(Foundation::Framework *owner);

    /// Requests the given asset to be downloaded. The transfer will go to the pending transfers queue
    /// and will be processed when possible.
    /// @param assetRef The asset ID, or URL to request.
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

    /// Returns the known asset storage instances in the system.
//    std::vector<IAssetStorage*> GetAssetStorages() const;

    /// Creates a new empty asset of the given type and with the given name.
    IAsset *CreateAsset(QString assetType, QString assetRef);

    /// Removes the given asset from the system and frees up all resources related to it. The asset will
    /// stay in the disk cache for later access.
    void DeleteAsset(IAsset *asset);

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<IAssetTransfer*> PendingTransfers() const;

    /// This function is implemented for legacy purposes to help transition period to new Asset API. Will be removed. Do NOT call this. -jj
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

private:

    /// This is implemented for legacy purposes to help transition period to new Asset API. Will be removed. -jj
    std::map<request_tag_t, IAssetTransfer*> currentTransfers;

    Foundation::Framework *framework;

};

#endif
