#ifndef incl_Asset_AssetAPI_h
#define incl_Asset_AssetAPI_h

class IAssetProvider
{
};

//class LocalAssetProvider : public IAssetProvider;
//class HttpAssetProvider : public IAssetProvider;
//class KNetAssetProvider : public IAssetProvider;

class IAssetTransfer
{
public:
    /// Stores the raw 
    std::vector<char> rawAssetData;

    /// Stores the actual asset if it has been loaded in.
    IAsset *asset;


signals:
    /// Emitted when the download of this asset finishes.
    void Downloaded();

    /// Emitted when a decoder plugin has decoded this asset.
    void Decoded();

    /// Emitted when this asset is ready to be used in the system.
    void Prepared();
};

//class LocalAssetTransfer : public IAssetTransfer;
//class HttpAssetTransfer : public IAssetTransfer;
//class KNetAssetTransfer : public IAssetProvider;

class IAsset
{
public:

    /// Stores this asset to disk to the given file.
    void SaveToFile(QString filename);

    /// Returns all the assets this asset refers to (but not the references those assets refer to).
    std::vector<IAsset*> FindReferences() const;

    /// Returns all the assets this asset refers to, and the assets those assets refer to, and so on.
    std::vector<IAsset*> FindReferencesDeep() const;

private:
    /// Specifies the provider this asset was downloaded from.
    IAssetProvider *provider;
};

class AssetAPI
{
public:
    /// Requests the given asset to be downloaded. The transfer will go to the pending transfers queue
    /// and will be processed when possible.
    IAssetTransfer *RequestAsset(QString assetRef);

    /// Returns the given asset if it exists, or null otherwise.
    IAsset *GetAsset(QString assetRef);

    /// Creates a new empty asset of the given type and with the given name.
    IAsset *CreateAsset(QString assetType, QString assetRef);

    /// Removes the given asset from the system and frees up all resources related to it. The asset will
    /// stay in the disk cache for later access.
    void DeleteAsset(IAsset *asset);

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<IAssetTransfer*> PendingTransfers(); const;

private:

};


#endif
