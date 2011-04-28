// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetAPI_h
#define incl_Asset_AssetAPI_h

#include <QObject>
#include <vector>
#include <utility>
#include <map>

#include "CoreTypes.h"
#include "AssetFwd.h"

class QFileSystemWatcher;

/// Loads the given local file into the specified vector. Clears all data previously in the vector.
/// Returns true on success.
bool LoadFileToVector(const char *filename, std::vector<u8> &dst);

/// Copies the given source file to the destination file on the local filesystem. Returns true on success.
bool CopyAssetFile(const char *sourceFile, const char *destFile);

/// Saves the given raw data buffer to destFile. Returns true on success.
bool SaveAssetFromMemoryToFile(const u8 *data, size_t numBytes, const char *destFile);

/// Adds a trailing slash to the given string representing a directory path if it doesn't have one at the end already.
QString GuaranteeTrailingSlash(const QString &source);

typedef std::map<QString, AssetPtr> AssetMap;

typedef std::vector<AssetStoragePtr> AssetStorageVector;

class AssetAPI : public QObject
{
    Q_OBJECT

public:
    AssetAPI(bool isHeadless);

    ~AssetAPI();

public:
    /// Registers a type factory for creating assets of the type governed by the factory.
    void RegisterAssetTypeFactory(AssetTypeFactoryPtr factory);

    /// Returns all registered asset type factories. You can use this list to query which asset types the system can handle.
    std::vector<AssetTypeFactoryPtr> GetAssetTypeFactories() { return assetTypeFactories; }

    /// Returns the asset provider of the given type.
    /// The registered asset providers are unique by type. You cannot register two instances of the same provider type to the system.
    template<typename T>
    boost::shared_ptr<T> GetAssetProvider();

    /// Registers a new asset provider to the Asset API. Use this to add a new provider type you have instantiated to the system.
    void RegisterAssetProvider(AssetProviderPtr provider);

    /// Returns all the asset providers that are registered to the Asset API.
    std::vector<AssetProviderPtr> GetAssetProviders() const;

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<AssetTransferPtr> PendingTransfers();

    /// Performs internal tick-based updates of the whole asset system. This function is intended to be called only by the core, do not call
    /// it yourself.
    void Update(f64 frametime);

    /// Called by each AssetProvider to notify the Asset API that an asset transfer has completed. Do not call this function from client code.
    void AssetTransferCompleted(IAssetTransfer *transfer);

    /// Called by each AssetProvider to notify the Asset API that the asset transfer finished in a failure. The Asset API will erase this transfer and
    /// also fail any transfers of assets which depended on this transfer.
    void AssetTransferFailed(IAssetTransfer *transfer, QString reason);

    /// Called by each AssetProvider to notify the Asset API that an asset upload transfer has completed. Do not call this function from client code.
    void AssetUploadTransferCompleted(IAssetUploadTransfer *transfer);

    void AssetDependenciesCompleted(AssetTransferPtr transfer);

    void NotifyAssetDependenciesChanged(AssetPtr asset);

    bool IsHeadless() const { return isHeadless_; }

    /// Returns all the currently loaded assets which depend on the asset dependeeAssetRef.
    std::vector<AssetPtr> FindDependents(QString dependeeAssetRef);

    class QStringLessThanNoCase
    {
    public:
        bool operator()(const QString &a, const QString b) const
        {
            return QString::compare(a, b, Qt::CaseInsensitive) < 0;
        }
    };

    /// Specifies the different possible results for AssetAPI::ResolveLocalAssetPath.
    enum FileQueryResult
    {
        FileQueryLocalFileFound, ///< The asset reference specified a local filesystem file, and the absolute path name for it was found.
        FileQueryLocalFileMissing, ///< The asset reference specified a local filesystem file, but there was no file in that location.
        FileQueryExternalFile ///< The asset reference points to a file in an external source, which cannot be checked for existence in the current function (would require a network lookup).
    };

    enum AssetRefType
    {
        AssetRefInvalid,
        AssetRefLocalPath,      ///< The assetRef points to the local filesystem using an *absolute* pathname, e.g "C:\myassets\texture.png".
        AssetRefRelativePath,   ///< The assetRef is a path relative to some context. "asset.png" or "relativePath/model.mesh".
        AssetRefLocalUrl,       ///< The assetRef points to the local filesystem, using a local URL syntax like local:// or file://. E.g. "local://texture.png".
        AssetRefExternalUrl,    ///< The assetRef points to a location external to the system, using a URL protocol specifier. "http://server.com/asset.png".
        AssetRefNamedStorage    ///< The assetRef points to an explicitly named storage. "storageName:asset.png".
    };

    /// Breaks the given assetRef into pieces, and returns the parsed type.
    /// @param assetRef The assetRef to parse.
    /// @param outProtocolPart [out] Receives the protocol part of the ref, e.g. "http://server.com/asset.png" -> "http". If it doesn't exist, returns an empty string.
    /// @param outNamedStorage [out] Receives the named storage specifier in the ref, e.g. "myStorage:asset.png" -> "myStorage". If it doesn't exist, returns an empty string.
    /// @param outProtocol_Path [out] Receives the "path name" identifying where the asset is stored in.
    ///                                e.g. "http://server.com/path/folder2/asset.png" -> "http://server.com/path/folder2/". 
    ///                                     "myStorage:asset.png" -> "myStorage:". Always has a trailing slash if necessary.
    /// @param outPath_Filename_SubAssetName [out] Gets the combined path name, asset filename and asset subname in the ref.
    ///                                e.g. "local://path/folder/asset.png, subAsset" -> "path/folder/asset.png, subAsset".
    ///                                     "namedStorage:path/folder/asset.png, subAsset" -> "path/folder/asset.png, subAsset".
    /// @param outPath_Filename [out] Gets the combined path name and asset filename in the ref.
    ///                                e.g. "local://path/folder/asset.png, subAsset" -> "path/folder/asset.png".
    ///                                     "namedStorage:path/folder/asset.png, subAsset" -> "path/folder/asset.png, subAsset".
    /// @param outPath [out] Returns the path part of the ref, e.g. "local://path/folder/asset.png, subAsset" -> "path/folder/". Has a trailing slash when necessary.
    /// @param outFilename [out] Returns the base filename of the asset. e.g. "local://path/folder/asset.png, subAsset" -> "asset.png".
    /// @param outSubAssetName [out] Returns the subasset name in the ref. e.g. "local://path/folder/asset.png, subAsset" -> "subAsset".
    /// @param outFullRef [out] Returns a cleaned or "canonicalized" version of the asset ref in full.
    static AssetRefType ParseAssetRef(QString assetRef, QString *outProtocolPart = 0, QString *outNamedStorage = 0, QString *outProtocol_Path = 0, 
        QString *outPath_Filename_SubAssetName = 0, QString *outPath_Filename = 0, QString *outPath = 0, QString *outFilename = 0, QString *outSubAssetName = 0,
        QString *outFullRef = 0);

public slots:
    /// Returns all assets known to the asset system. AssetMap maps asset names to their AssetPtrs.
    AssetMap GetAllAssets() { return assets; }

    /// Returns the known asset storage instances in the system.
    AssetStorageVector GetAssetStorages() const;

    /// Opens the internal Asset API asset cache to the given directory. When the Asset API starts up, the asset cache is not created. This allows
    /// the Asset API to be operated in a mode that does not perform writes to the disk when assets are fetched. This will cause assets fetched from
    /// remote hosts to have a null disk source.
    /// \note Once the asset cache has been created with a call to this function, there is no way to close the asset cache (except to close and restart).
    void OpenAssetCache(QString directory);

    /// Requests the given asset to be downloaded. The transfer will go to a pending transfers queue
    /// and will be processed when possible.
    /** @param assetRef The asset reference (a filename or a full URL) to request. The name of the resulting asset is the same as the asset reference
              that is used to load it.
        @param assetType The type of the asset to request. This can be null if the assetRef itself identifies the asset type.
        @return A pointer to the created asset transfer, or null if the transfer could not be initiated. */
    AssetTransferPtr RequestAsset(QString assetRef, QString assetType = "");

    /// Same as RequestAsset(assetRef, assetType), but provided for convenience with the AssetReference type.
    AssetTransferPtr RequestAsset(const AssetReference &ref);

    /// Returns the asset provider that is used to fetch assets from the given full URL.
    /** Example: GetProviderForAssetRef("local://my.mesh") will return an instance of LocalAssetProvider.
        @param assetRef The asset reference name to query a provider for.
        @param assetType An optionally specified asset type. Some providers can only handle certain asset types. This parameter can be 
                        used to more completely specify the type. */
    AssetProviderPtr GetProviderForAssetRef(QString assetRef, QString assetType = "");

    /// Creates a new empty unloaded asset of the given type and name.
    /** This function uses the Asset type factories to create an instance of the proper asset class.
        @param type The asset type of the asset to be created. A factory of this type must have been registered beforehand,
                    using the AssetAPI::RegisterAssetTypeFactory function.
        @param name Specifies the name to give to the new asset. This name must be unique in the system, or this call will fail.
                    Use GetAsset(name) to query if an asset with the given name exists, and the AssetAPI::GenerateUniqueAssetName 
                    to guarantee the creation of a unique asset name. */
    AssetPtr CreateNewAsset(QString type, QString name);

    /// Loads an asset from a local file.
    AssetPtr CreateAssetFromFile(QString assetType, QString assetFile);

    /// Generates a new asset name that is guaranteed to be unique in the system.
    /** @param assetTypePrefix The type of the asset to use as a human-readable visual prefix identifier for the name. May be empty.
        @param assetNamePrefix A name prefix that is added to the asset name for visual identification. May be empty.
        @return A string of the form "Asset_<assetTypePrefix>_<assetNamePrefix>_<number>". */
    QString GenerateUniqueAssetName(QString assetTypePrefix, QString assetNamePrefix);

    /// Generates an absolute path name to a file on the local system that is guaranteed to be writable to and nonexisting. This file can be used as temporary workspace
    /// for asset serialization/deserialization routines. This is used especially with Ogre-related data (de)serializers, since they don't have support for loading/saving data
    /// from memory and need to access a file.
    QString GenerateTemporaryNonexistingAssetFilename(QString filename);

    /// Returns the asset type factory that can create assets of the given type, or null, if no asset type provider of the given type exists.
    AssetTypeFactoryPtr GetAssetTypeFactory(QString typeName);

    /// Returns the given asset by full URL ref if it exists, or null otherwise.
    /// @note The "name" of an asset is in most cases the URL ref of the asset, so use this function to query an asset by name.
    AssetPtr GetAsset(QString assetRef);
    
    /// Returns the given asset by the specified SHA-1 content hash. If no such asset exists, returns null.
    AssetPtr GetAssetByHash(QString assetHash);

    /// Returns the asset cache object that genereates a disk source for all assets.
    AssetCache *GetAssetCache() { return assetCache; }

    /// Returns the asset storage of the given name.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    AssetStoragePtr GetAssetStorage(const QString &name) const;

    /// Removes the given asset storage from the list of all asset storages.
    /// The scene can still refer to assets in this storage, and download requests can be performed to it, but it will not show up in the Assets dialog,
    /// and asset upload operations cannot be performed to it. Also, it will not be used as a default storage.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    bool RemoveAssetStorage(const QString &name);

    /// Creates an asset storage from the given serialized string form.
    /// Returns a null pointer if the given storage could not be added.
    AssetStoragePtr DeserializeAssetStorageFromString(const QString &storage);

    /// Returns the AssetStorage that should be used by default when assets are requested by their local name only, e.g. when an assetRef only contains
    /// a string "texture.png" and nothing else.
    AssetStoragePtr GetDefaultAssetStorage() const;

    /// Sets the asset storage to be used when assets are requested by their local names.
    void SetDefaultAssetStorage(const AssetStoragePtr &storage);

    /// Tries to find the filename in an url/assetref.
    /** For example, all "my.mesh", "C:\files\my.mesh", "local://path/my.mesh", "http://www.web.com/my.mesh" will return "my.mesh".
        "local://collada.dae,subMeshName" returns "collada.dae". */
    static QString ExtractFilenameFromAssetRef(QString ref);

    /// Returns an asset type name of the given assetRef. e.g. "asset.png" -> "Texture". The Asset type name is a unique type identifier string
    /// each asset type has.
    static QString GetResourceTypeFromAssetRef(QString assetRef);

    /// Parses a (relative) assetRef in the given context, and returns an assetRef pointing to the same asset as an absolute asset ref.
    /// For example: context: "local://myasset.material", ref: "texture.png" returns "local://texture.png".
    /// context: "http://myserver.com/path/myasset.material", ref: "texture.png" returns "http://myserver.com/path/texture.png".
    /// The context string may be left empty, in which case the current default storage (GetDefaultAssetStorage()) is used as the context.
    /// If ref is an absolute asset reference, it is returned unmodified (no need for context).
    QString ResolveAssetRef(QString context, QString ref);

    /// Given an assetRef, turns it into a native OS file path to the asset. The given ref is resolved in the context of "local://", if it is
    /// a relative asset ref. If ref contains a subAssetName, it is stripped from outFilePath, and returned in subAssetName.
    /// If the assetRef doesn't represent a file on the local filesystem, FileQueryExternalFile is returned and outFilePath is set to equal best effort to parse 'ref' locally.
    FileQueryResult ResolveLocalAssetPath(QString ref, QString baseDirectoryContext, QString &outFilePath, QString *subAssetName = 0);

    /// Recursively iterates through the given path and all its subdirectories and tries to find the given file.
    /** Returns the absolute path for that file, if it exists. The path contains the filename,
        i.e. it is of form "C:\folder\file.ext" or "/home/username/file.ext". */
    static QString RecursiveFindFile(QString basePath, QString filename);

    /// Removes the given asset from the system and frees up all resources related to it. Any assets depending on this asset will break.
    /// @param assetRef A valid assetRef that is in the asset system. If this asset ref does not exist, this call will do nothing.
    /// @param removeDiskSource If true, the disk source of the asset is also deleted. In most cases, this is the locally cached version of the remote file,
    ///         but for example for local assets, this is the asset itself.
    /// @note Calling ForgetAsset on an asset will unload it from the system. Do not dereference the asset after calling this function.
    void ForgetAsset(QString assetRef, bool removeDiskSource);

    /// Removes the given asset from the system and frees up all resources related to it. Any assets depending on this asset will break.
    /// @param removeDiskSource If true, the disk source of the asset is also deleted. In most cases, this is the locally cached version of the remote file,
    ///         but for example for local assets, this is the asset itself.
    /// @note Calling ForgetAsset on an asset will unload it from the system. Do not dereference the asset after calling this function.
    void ForgetAsset(AssetPtr asset, bool removeDiskSource);

    /// Sends an asset deletion request to the remote asset storage the asset resides in.
    /// @note Calling DeleteAssetFromStorage on an asset will unload it from the system, *and* delete the disk source of this asset. Do not dereference 
    /// the asset after calling this function. After calling this function, the asset will be gone from everywhere!
    void DeleteAssetFromStorage(QString assetRef);

    /// Uploads an asset to an asset storage.
    /** @param filename The source file to load the asset from.
        @param storageName The asset storage to upload the asset to.
        @param assetName Optional name to give to the asset in the storage. If this is not given assetName = original filename.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.
        
        @note This is a script friendly override. This function will never throw Exceptions (CoreException.h) if passed data is invalid, but will return null AssetUploadTransferPtr. */
    AssetUploadTransferPtr UploadAssetFromFile(const QString &filename, const QString &storageName, const QString &assetName = "");

    /// Uploads an asset to an asset storage.
    /** @param filename The source file to load the asset from.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This function will never return 0, but insted will throw Exception (CoreException.h) if passed data is invalid. */
    AssetUploadTransferPtr UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName);

    /// Uploads an asset from the given data in memory to an asset storage.
    /** @param data A QByteArray that has the uploaded data.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This is a script friendly override. This function will never throw Exceptions (CoreException.h) if passed data is invalid, but will return null AssetUploadTransferPtr. */
    AssetUploadTransferPtr UploadAssetFromFileInMemory(const QByteArray &data, const QString &storageName, const QString &assetName);

    /// Uploads an asset from the given data pointer in memory to an asset storage.
    /** @param data A pointer to raw source data in memory.
        @param numBytes The amount of data in the data array.
        @param destination The asset storage to upload the asset to.
        @param assetName The name to give to the asset in the storage.
        @return The returned IAssetUploadTransfer pointer represents the ongoing asset upload process.

        @note This function will never return 0, but insted will throw Exception (CoreException.h) if passed data is invalid. */
    AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName);

    /// Unloads all known assets, and removes them from the list of internal assets known to the Asset API.
    /** Use this to clear the client's memory from all assets.
        @note There may be any number of strong references to assets in other parts of code, in which case the assets are not deleted
        until the refcounts drop to zero.
        @note Do not dereference any asset pointers that might have been left over after calling this function. */
    void ForgetAllAssets();

    /// Returns a pointer to an existing asset transfer if one is in-progress for the given assetRef. Returns a null pointer if no transfer exists, in which
    /// case the asset may already have been loaded to the system (or not). It can be that an asset is loaded to the system, but one or more of its dependencies
    /// have not, in which case there exists both an IAssetTransfer and IAsset to this particular assetRef (so the existence of these two objects is not
    /// mutually exclusive).
    /// \note Client code should not need to worry about whether a particular transfer is pending or not, but simply call RequestAsset whenever an asset
    /// request is needed. AssetAPI will optimize away any duplicate transfers to the same asset.
    AssetTransferPtr GetPendingTransfer(QString assetRef);

    /// Starts an asset transfer for each dependency the given asset has.
    void RequestAssetDependencies(AssetPtr transfer);

    /// An utility function that counts the number of dependencies the given asset has to other assets that have not been loaded in.
    int NumPendingDependencies(AssetPtr asset);

    /// Emit AssetDiscovered signal
    void EmitAssetDiscovered(const QString &assetRef, const QString &assetType);
    
    /// Emit AssetDeleted signal
    void EmitAssetDeleted(const QString &assetRef);
    
signals:
    /// Emitted for each new asset that was created and added to the system. When this signal is triggered, the dependencies of an asset
    /// may not yet have been loaded.
    void AssetCreated(AssetPtr asset);

    /// Emitted before an asset is going to be forgotten.
    void AssetAboutToBeRemoved(AssetPtr asset);

    /// Emitted before an assets disk source will be removed.
    void DiskSourceAboutToBeRemoved(AssetPtr asset);

    /// Emitted when an asset has been uploaded
    void AssetUploaded(const QString &assetRef);

    /// Emitted when an asset storage has been added
    void AssetStorageAdded(AssetStoragePtr storage);

    /// Emitted when a new assetref has been discovered through the AssetDiscovery network message
    void AssetDiscovered(const QString &assetRef, const QString &assetType);

    /// Emitted when an asset is successfully deleted from storage. It is the providers' responsibility to make the AssetAPI emit this signal
    void AssetDeleted(const QString &assetRef);
    
    /// Emitted when the contents of an asset disk source has changed. ///\todo Implement.
 //   void AssetDiskSourceChanged(AssetPtr asset);

    /// Emitted when the asset has changed in the remote AssetStorage it is in. ///\todo Implement.
//    void AssetStorageSourceChanged(AssetPtr asset);

private slots:
    /// The Asset API listens on each asset when they get loaded, to track the completion of the dependencies of other loaded assets.
    void OnAssetLoaded(AssetPtr asset);

    /// The Asset API reloads all assets from file when their disk source contents change.
    void OnAssetDiskSourceChanged(const QString &path);

private:
    bool isHeadless_;
    typedef std::map<QString, AssetTransferPtr, AssetAPI::QStringLessThanNoCase> AssetTransferMap;
    /// Stores all the currently ongoing asset transfers.
    AssetTransferMap currentTransfers;

    typedef std::map<QString, AssetUploadTransferPtr, AssetAPI::QStringLessThanNoCase> AssetUploadTransferMap;
    /// Stores all the currently ongoing asset uploads, maps full assetRefs to the asset upload transfer structures.
    AssetUploadTransferMap currentUploadTransfers;

    typedef std::vector<std::pair<QString, QString> > AssetDependenciesMap;
    /// Keeps track of all the dependencies each asset has to each other asset.
    /// \todo Find a more effective data structure for this. Needs something like boost::bimap but for multi-indices.
    AssetDependenciesMap assetDependencies;

    /// Removes from AssetDependenciesMap all dependencies the given asset has.
    void RemoveAssetDependencies(QString asset);

    /// Stores a list of asset requests to assets that have already been downloaded into the system. These requests don't go to the asset providers
    /// to process, but are internally filled by the Asset API. This member vector is needed to be able to delay the requests and virtual completions
    /// by one frame, so that the client gets a chance to connect his handler's Qt signals to the AssetTransferPtr slots.
    std::vector<AssetTransferPtr> readyTransfers;

    /// Contains all known asset storages in the system.
//    std::vector<AssetStoragePtr> storages;

    /// Specifies the storage to use for asset requests with local name only.
    AssetStorageWeakPtr defaultStorage;

    /// Stores all the registered asset type factories in the system.
    std::vector<AssetTypeFactoryPtr> assetTypeFactories;

    /// Stores a list of asset requests that the Asset API hasn't started at all but has put on hold, until other operations complete.
    /// This data structure is used to enforce that asset uploads are completed before any asset downloads to that asset.
    struct PendingDownloadRequest
    {
        QString assetRef;
        QString assetType;
        AssetTransferPtr transfer;
    };
    typedef std::map<QString, PendingDownloadRequest, AssetAPI::QStringLessThanNoCase> PendingDownloadRequestMap;
    PendingDownloadRequestMap pendingDownloadRequests;

    /// Stores all the already loaded assets in the system.
    AssetMap assets;

    /// Tracks all loaded assets if their DiskSources change, and issues a reload of the assets.
    QFileSystemWatcher *diskSourceChangeWatcher;

    /// Specifies all the registered asset providers in the system.
    std::vector<AssetProviderPtr> providers;

    AssetCache *assetCache;
};

#include "AssetAPI.inl"

#endif
