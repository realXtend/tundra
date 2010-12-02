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

/// Loads the given local file into the specified vector. Clears all data previously in the vector.
/// Returns true on success.
bool LoadFileToVector(const char *filename, std::vector<u8> &dst);

/// Returns an asset type name of the given asset file name.
QString GetResourceTypeFromResourceFileName(const char *name);

/// Adds a trailing slash to the given string representing a directory path if it doesn't have one at the end already.
QString GuaranteeTrailingSlash(const QString &source);

class AssetAPI : public QObject
{
    Q_OBJECT

public:
    explicit AssetAPI(Foundation::Framework *owner);

    /// Requests the given asset to be downloaded. The transfer will go to the pending transfers queue
    /// and will be processed when possible.
    /// @param assetRef The asset ID, or full URL to request.
    /// @param assetType The type of the asset to request. This can be null if the assetRef itself identifies the asset type.
    AssetTransferPtr RequestAsset(QString assetRef, QString assetType = "");

    /// Same as RequestAsset(QString assetRef, QString assetType), but provided for convenience with AssetReference type.
    AssetTransferPtr RequestAsset(const AssetReference &ref);

    /// Returns the asset provider that is used to fetch assets from the given full URL.
    /// Example: GetProviderForAssetRef("local://my.mesh") will return an instance of LocalAssetProvider.
    /// @param assetRef The asset reference name to query a provider for.
    /// @param assetType An optionally specified asset type. Some providers can only handle certain asset types. This parameter can be 
    ///                  used to more completely specify the type.
    Foundation::AssetProviderPtr GetProviderForAssetRef(QString assetRef, QString assetType = "");

    /// Registers a type factory for creating assets of the type governed by the factory.
    void RegisterAssetTypeFactory(AssetTypeFactoryPtr factory);

    /// Creates a new empty unloaded asset of the given type and name.
    /// This function uses the Asset type factories to create an instance of the proper asset class.
    AssetPtr CreateNewAsset(QString type, QString name);

    /// Returns the asset type factory that can create assets of the given type, or null, if no asset type provider of the given type exists.
    AssetTypeFactoryPtr GetAssetTypeFactory(QString typeName);

    /// Returns the given asset by full URL ref if it exists, or null otherwise.
    AssetPtr GetAsset(QString assetRef);
    
    /// Queries if any existing Asset Storage contains an asset with the given name, and returns it.
//    IAsset *GetAssetByName(QString assetRef);

    /// Returns all assets known to the asset system.
//    std::vector<IAsset*> GetAllAssets() const;

    /// Returns the asset provider of the given type.
    template<typename T>
    boost::shared_ptr<T> GetAssetProvider();

    /// Returns all the asset providers that are registered to the Asset API.
    std::vector<Foundation::AssetProviderPtr> GetAssetProviders() const;

    /// Returns the asset storage of the given @c name
    AssetStoragePtr GetAssetStorage(const QString &name) const;

    /// Returns the known asset storage instances in the system.
    std::vector<AssetStoragePtr> GetAssetStorages() const;

    /// Specifies the different possible results for AssetAPI::QueryFileLocation.
    enum FileQueryResult
    {
        FileQueryLocalFileFound, ///< The asset reference specified a local filesystem file, and the absolute path name for it was found.
        FileQueryLocalFileMissing, ///< The asset reference specified a local filesystem file, but there was no file in that location.
        FileQueryExternalFile ///< The asset reference points to a file in an external source, which cannot be checked for existence (too costly performance-wise).
    };

    /// Performs a lookup of the given source asset reference, and returns in outFilePath the absolute path of that file, if it was found.
    /// @param baseDirectory You can give a single base directory to this function to use as a "current directory" for the local file lookup. This is
    ///           usually the local path of the scene content that is being added.
    FileQueryResult QueryFileLocation(QString sourceRef, QString baseDirectory, QString &outFilePath);

    /// Tries to find the filename in an url/assetref. For example, all "my.mesh", "C:\files\my.mesh", "local://path/my.mesh", "http://www.web.com/my.mesh" will return "my.mesh".
    /// \todo It is the intent that "local://collada.dae/subMeshName" would return "collada.dae" and "file.zip/path1/path2/my.mesh" would return "file.zip", but this hasn't been
    /// implemented (since those aren't yet supported).
    static QString ExtractFilenameFromAssetRef(QString ref);

    /// Recursively iterates through the given path and all its subdirectories and tries to find the given file.
    /// Returns the absolute path for that file, if it exists. The path contains the filename, i.e. it is of form "C:\folder\file.ext" or "/home/username/file.ext".
    static QString RecursiveFindFile(QString basePath, QString filename);

    /// Creates a new empty asset of the given type and with the given name.
//    IAsset *CreateAsset(QString assetType, QString assetRef);

    /// Removes the given asset from the system and frees up all resources related to it. The asset will
    /// stay in the disk cache for later access.
//    void DeleteAsset(IAsset *asset);

    /// Uploads an asset to an asset storage.
    /** @param filename The source file to load the asset from.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This function will never return 0, but throws an Exception if the data that was passed in was bad.
    */
    IAssetUploadTransfer *UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName);

    /// Uploads an asset from the given data pointer in memory to an asset storage.
    /** @param data A pointer to raw source data in memory.
        @param numBytes The amount of data in the data array.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This function will never return 0, but throws an Exception if the data that was passed in was bad.
    */
    IAssetUploadTransfer *UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    /// Unloads all known assets, and removes them from the list of internal assets known to the Asset API.
    /// Use this to clear the client's memory from all assets.
    /// \note There may be any number of strong references to assets in other parts of code, in which case the assets are not deleted
    /// until the refcounts drop to zero.
    void ForgetAllAssets();

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<AssetTransferPtr> PendingTransfers() const;

    /// This function is implemented for legacy purposes to help transition period to new Asset API. Will be removed. Do NOT call this. -jj
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);

private slots:
    void AssetDownloaded(IAssetTransfer *transfer);

private:
    /// This is implemented for legacy purposes to help transition period to new Asset API. Will be removed. -jj
    std::map<request_tag_t, AssetTransferPtr> currentTransfers;

    Foundation::Framework *framework;

    /// Contains all known asset storages in the system.
    std::vector<boost::shared_ptr<IAssetStorage> > storages;

    /// Stores all the registered asset type factories in the system.
    std::vector<AssetTypeFactoryPtr> assetTypeFactories;

    /// Stores all the assets in the system.
    std::vector<AssetPtr> assets;

    /// For now, the Asset API holds a weak reference to each provider.
//    std::vector<AssetProviderInterface*> providers;

};

#include "AssetAPI.inl"

#endif
