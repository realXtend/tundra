// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "CoreStringUtils.h"
#include "AssetFwd.h"
#include "IAssetStorage.h"

#include <QObject>
#include <vector>
#include <utility>
#include <map>

class QFileSystemWatcher;

/// Loads the given local file into the specified vector. Clears all data previously in the vector.
/// Returns true on success.
bool TUNDRACORE_API LoadFileToVector(const QString &filename, std::vector<u8> &dst);

/// Copies the given source file to the destination file on the local filesystem. Returns true on success.
bool TUNDRACORE_API CopyAssetFile(const QString &sourceFile, const QString &destFile);

/// Saves the given raw data buffer to destFile. Returns true on success.
bool TUNDRACORE_API SaveAssetFromMemoryToFile(const u8 *data, size_t numBytes, const QString &destFile);

/// Given a string of form "someString?param1=value1&param2=value2", returns a map of key-value pairs.
/// @param body [out] If specified, the part 'someString' is returned here.
std::map<QString, QString> TUNDRACORE_API ParseAssetRefArgs(const QString &url, QString *body);

/// Adds a trailing slash to the given string representing a directory path if it doesn't have one at the end already.
/// This trailing slash will always be of the unix format, i.e. '/'.
/// If an empty string is submitted, and empty string will be output, so that an empty string won't suddenly point to the filesystem root.
QString TUNDRACORE_API GuaranteeTrailingSlash(const QString &source);

typedef std::map<QString, AssetPtr, QStringLessThanNoCase> AssetMap; ///<  Maps asset names to their AssetPtrs.
typedef std::map<QString, AssetBundlePtr, QStringLessThanNoCase> AssetBundleMap; ///<  Maps asset bundle names to their AssetBundlePtrs.
typedef std::map<QString, AssetTransferPtr, QStringLessThanNoCase> AssetTransferMap;
typedef std::map<QString, AssetBundleMonitorPtr, QStringLessThanNoCase> AssetBundleMonitorMap;

typedef std::vector<AssetStoragePtr> AssetStorageVector;

/// Implements asset download and upload functionality.
class TUNDRACORE_API AssetAPI : public QObject
{
    Q_OBJECT

public:
    AssetAPI(Framework *fw, bool headless);

    ~AssetAPI();

public:
    /// Registers a type factory for creating assets of the type governed by the factory.
    void RegisterAssetTypeFactory(AssetTypeFactoryPtr factory);

    /// Registers a type factory for creating asset bundles of the type governed by the factory.
    void RegisterAssetBundleTypeFactory(AssetBundleTypeFactoryPtr factory);

    /// Returns all registered asset type factories.
    /** You can use this list to query which asset types the system can handle. */
    std::vector<AssetTypeFactoryPtr> AssetTypeFactories() const { return assetTypeFactories; }

    /// Returns the asset provider of the given type.
    /** The registered asset providers are unique by type. You cannot register two instances of the same provider type to the system. */
    template<typename T>
    shared_ptr<T> AssetProvider() const;

    /// Registers a new asset provider to the Asset API.
    /** Use this to add a new provider type you have instantiated to the system. */
    void RegisterAssetProvider(AssetProviderPtr provider);

    /// Returns all the asset providers that are registered to the Asset API.
    std::vector<AssetProviderPtr> AssetProviders() const;

    /// Returns all the currently ongoing or waiting asset transfers.
    std::vector<AssetTransferPtr> PendingTransfers() const;

    /// Performs internal tick-based updates of the whole asset system.
    /** This function is intended to be called only by the core, do not call it yourself. */
    void Update(f64 frametime);

    /// Called by each AssetProvider to notify the Asset API that an asset transfer has completed.
    /** Do not call this function from client code. */
    void AssetTransferCompleted(IAssetTransfer *transfer);

    /// Called by each AssetProvider to notify the Asset API that the asset transfer finished in a failure.
    /** The Asset API will erase this transfer and also fail any transfers of assets which depended on this transfer. */
    void AssetTransferFailed(IAssetTransfer *transfer, QString reason);
    
    /// Called by each AssetProvider to notify of aborted transfers.
    /** The Asset API will erase this transfer and also fail any transfers of assets which depended on this transfer. */
    void AssetTransferAborted(IAssetTransfer *transfer);

    /// Called by each IAsset when it has completed loading successfully.
    /** Typically inside IAsset::DeserializeFromData or later on if it is loading asynchronously. */
    void AssetLoadCompleted(const QString assetRef);

    /// Called by each IAsset when it has failed to load.
    /** Typically inside IAsset::DeserializeFromData or later on if it is loading asynchronously. */
    void AssetLoadFailed(const QString assetRef);

    /// Called by each AssetProvider to notify the Asset API that an asset upload transfer has completed. Do not call this function from client code.
    void AssetUploadTransferCompleted(IAssetUploadTransfer *transfer);

    void AssetDependenciesCompleted(AssetTransferPtr transfer);

    void NotifyAssetDependenciesChanged(AssetPtr asset);

    bool IsHeadless() const { return isHeadless; }

    /// Returns all the currently loaded assets which depend on the asset dependeeAssetRef.
    std::vector<AssetPtr> FindDependents(QString dependeeAssetRef);

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
    /** @param assetRef The assetRef to parse.
        @param outProtocolPart [out]  Receives the protocol part of the ref, e.g. "http://server.com/asset.png" -> "http". If it doesn't exist, returns an empty string.
        @param outNamedStorage [out]  Receives the named storage specifier in the ref, e.g. "myStorage:asset.png" -> "myStorage". If it doesn't exist, returns an empty string.
        @param outProtocol_Path [out] Receives the "path name" identifying where the asset is stored in.
                                      e.g. "http://server.com/path/folder2/asset.png" -> "http://server.com/path/folder2/". 
                                           "myStorage:asset.png" -> "myStorage:". Always has a trailing slash if necessary.
        @param outPath_Filename_SubAssetName [out] Gets the combined path name, asset filename and asset subname in the ref.
                                      e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/asset.zip#subAsset".
                                           "namedStorage:path/folder/asset.zip#subAsset" -> "path/folder/asset.zip, subAsset".
        @param outPath_Filename [out] Gets the combined path name and asset filename in the ref.
                                      e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/asset.zip".
                                           "namedStorage:path/folder/asset.zip#subAsset" -> "path/folder/asset.zip".
        @param outPath [out] Returns the path part of the ref, e.g. "local://path/folder/asset.zip#subAsset" -> "path/folder/". Has a trailing slash when necessary.
        @param outFilename [out] Returns the base filename of the asset. e.g. "local://path/folder/asset.zip#subAsset" -> "asset.zip".
        @param outSubAssetName [out] Returns the sub asset name in the ref. e.g. "local://path/folder/asset.zip#subAsset" -> "subAsset".
        @param outFullRef [out] Returns a cleaned or "canonicalized" version of the asset ref in full.
        @param outFullRefNoSubAssetName [out] Returns a cleaned or "canonicalized" version of the asset ref in full without possible sub asset. */
    static AssetRefType ParseAssetRef(QString assetRef, QString *outProtocolPart = 0, QString *outNamedStorage = 0, QString *outProtocol_Path = 0, 
        QString *outPath_Filename_SubAssetName = 0, QString *outPath_Filename = 0, QString *outPath = 0, QString *outFilename = 0, QString *outSubAssetName = 0,
        QString *outFullRef = 0, QString *outFullRefNoSubAssetName = 0);

    typedef std::vector<std::pair<QString, QString> > AssetDependenciesMap;
    
    /// Sanitates an assetref so that it can be used as a filename for caching.
    /** Characters like ':'. '/', '\' and '*' will be replaced with $1, $2, $3, $4 .. respectively, in a reversible way.
        Note that sanitated assetrefs will not work when querying from the asset system, for that you need the desanitated form.
        @sa DesanitateAssetRef */
    static QString SanitateAssetRef(const QString& ref);
    static std::string SanitateAssetRef(const std::string& ref); /**< @overload */

    /// Desanitates an assetref with $1 $2 $3 $4 ... into original form.
    /** @sa SanitateAssetRef */
    static QString DesanitateAssetRef(const QString& ref);
    static std::string DesanitateAssetRef(const std::string& ref); /**< @overload */

    /// Explodes the given asset storage description string to key-value pairs.
    static QMap<QString, QString> ParseAssetStorageString(QString storageString);

    Framework *GetFramework() const { return fw; }

    // DEPRECATED
    bool IsAssetTypeFactoryRegistered(const QString &typeName) const { return AssetTypeFactory(typeName) != 0; } /**< @deprecated Use AssetTypeFactory. @todo Remove. */
    std::vector<AssetProviderPtr> GetAssetProviders() const { return AssetProviders(); }  /**< @deprecated Use AssetProviders instead @todo Add warning print in some distant future */
    std::vector<AssetTypeFactoryPtr> GetAssetTypeFactories() const { return AssetTypeFactories(); } /**< @deprecated Use AssetTypeFactories instead @todo Add warning print in some distant future */
    template<typename T> shared_ptr<T> GetAssetProvider() const { return AssetProvider<T>(); } /**< @deprecated Use AssetProvider instead @todo Add warning print in some distant future */

public slots:
    /// Returns all assets known to the asset system.
    AssetMap Assets() const { return assets; }

    /// Returns all asset bundles known to the asset system.
    AssetBundleMap AssetBundles() const { return assetBundles; }

    /// Returns all assets of a specific type.
    AssetMap AssetsOfType(const QString& type) const;

    /// Returns the known asset storage instances in the system.
    AssetStorageVector AssetStorages() const;

    /// Opens the internal Asset API asset cache to the given directory.
    /** When the Asset API starts up, the asset cache is not created. This allows the Asset API to be operated in a mode that does not 
        perform writes to the disk when assets are fetched. This will cause assets fetched from remote hosts to have a null disk source.
        @note Once the asset cache has been created with a call to this function, there is no way to close the asset cache (except to close and restart). */
    void OpenAssetCache(QString directory);

    /// Requests the given asset to be downloaded.
    /** The transfer will go to a pending transfers queue and will be processed when possible.
        @param assetRef The asset reference (a filename or a full URL) to request. The name of the resulting asset is the same as the asset reference
              that is used to load it.
        @param assetType The type of the asset to request. This can be null if the assetRef itself identifies the asset type.
        @param forceTransfer Force transfer even if the asset is in the loaded state
        @return A pointer to the created asset transfer, or null if the transfer could not be initiated. */
    AssetTransferPtr RequestAsset(QString assetRef, QString assetType = "", bool forceTransfer = false);
    AssetTransferPtr RequestAsset(const AssetReference &ref, bool forceTransfer = false); /**< @overload */

    /// Returns the asset provider that is used to fetch assets from the given full URL.
    /** Example: GetProviderForAssetRef("local://my.mesh") will return an instance of LocalAssetProvider.
        @param assetRef The asset reference name to query a provider for.
        @param assetType An optionally specified asset type. Some providers can only handle certain asset types. This parameter can be 
                        used to more completely specify the type. */
    AssetProviderPtr ProviderForAssetRef(QString assetRef, QString assetType = "") const;

    /// Creates a new empty unloaded asset of the given type and name.
    /** This function uses the Asset type factories to create an instance of the proper asset class.
        @param type The asset type of the asset to be created. A factory of this type must have been registered beforehand,
                    using the AssetAPI::RegisterAssetTypeFactory function.
        @param name Specifies the name to give to the new asset. This name must be unique in the system, or this call will fail.
                    Use GetAsset(name) to query if an asset with the given name exists, and the AssetAPI::GenerateUniqueAssetName 
                    to guarantee the creation of a unique asset name. */
    AssetPtr CreateNewAsset(QString type, QString name);

    /// Creates a new empty unloaded asset bundle of the given type and name.
    AssetBundlePtr CreateNewAssetBundle(QString type, QString name);

    /// Loads an asset from a local file.
    AssetPtr CreateAssetFromFile(QString assetType, QString assetFile);

    /// Generates a new asset name that is guaranteed to be unique in the system.
    /** @param assetTypePrefix The type of the asset to use as a human-readable visual prefix identifier for the name. May be empty.
        @param assetNamePrefix A name prefix that is added to the asset name for visual identification. May be empty.
        @return A string of the form "Asset_<assetTypePrefix>_<assetNamePrefix>_<number>". */
    QString GenerateUniqueAssetName(QString assetTypePrefix, QString assetNamePrefix) const;

    /// Generates an absolute path name to a file on the local system that is guaranteed to be writable to and nonexisting.
    /** This file can be used as temporary workspace for asset serialization/deserialization routines. This is used especially with
        Ogre-related data (de)serializers, since they don't have support for loading/saving data from memory and need to access a file. */
    QString GenerateTemporaryNonexistingAssetFilename(QString filename) const;

    /// Returns the asset type factory that can create assets of the given type, or null, if no asset type provider of the given type exists.
    AssetTypeFactoryPtr AssetTypeFactory(const QString &typeName) const;

    /// Return the asset bundle factory that can create asset bundles of the given type, or null, if no asset bundle type provider of the given type exists.
    AssetBundleTypeFactoryPtr AssetBundleTypeFactory(const QString &typeName) const;

    /// Returns the given asset by full URL ref if it exists, or null otherwise.
    /// @note The "name" of an asset is in most cases the URL ref of the asset, so use this function to query an asset by name.
    AssetPtr GetAsset(QString assetRef) const;

    /// Returns the asset cache object that generates a disk source for all assets.
    AssetCache *Cache() const { return assetCache; }

    /// Returns the asset storage of the given name.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    AssetStoragePtr AssetStorageByName(const QString &name) const;

    /// Returns the asset storage for a given asset ref.
    /// @param ref The ref to search for.
    AssetStoragePtr StorageForAssetRef(const QString& ref) const;

    /// Removes the given asset storage from the list of all asset storages.
    /// The scene can still refer to assets in this storage, and download requests can be performed to it, but it will not show up in the Assets dialog,
    /// and asset upload operations cannot be performed to it. Also, it will not be used as a default storage.
    /// @param name The name of the storage to get. Remember that Asset Storage names are case-insensitive.
    bool RemoveAssetStorage(const QString &name);

    /// Creates an asset storage from the given serialized string form.
    /// Returns a null pointer if the given storage could not be added.
    /// @param fromNetwork If true, treats the storage specifier as if the storage had been received from the network, and not from the local computer.
    AssetStoragePtr DeserializeAssetStorageFromString(const QString &storage, bool fromNetwork);

    /// Returns the AssetStorage that should be used by default when assets are requested by their local name only, e.g. when an assetRef only contains
    /// a string "texture.png" and nothing else.
    AssetStoragePtr DefaultAssetStorage() const;

    /// Sets the asset storage to be used when assets are requested by their local names.
    void SetDefaultAssetStorage(const AssetStoragePtr &storage);

    /// Tries to find the filename in an url/assetref.
    /** For example, all "my.mesh", "C:\files\my.mesh", "local://path/my.mesh", "http://www.web.com/my.mesh" will return "my.mesh".
        "local://collada.dae,subMeshName" returns "collada.dae". */
    static QString ExtractFilenameFromAssetRef(QString ref);

    /// Returns an asset type name of the given assetRef. e.g. "asset.png" -> "Texture".
    /** The Asset type name is a unique type identifier string each asset type has. */
    QString ResourceTypeForAssetRef(QString assetRef) const;
    QString ResourceTypeForAssetRef(const AssetReference &ref) const; /**< @overload */

    /// Parses a (relative) assetRef in the given context, and returns an assetRef pointing to the same asset as an absolute asset ref.
    /** For example: context: "local://myasset.material", ref: "texture.png" returns "local://texture.png".
        context: "http://myserver.com/path/myasset.material", ref: "texture.png" returns "http://myserver.com/path/texture.png".
        The context string may be left empty, in which case the current default storage (DefaultAssetStorage) is used as the context.
        If ref is an absolute asset reference, it is returned unmodified (no need for context). */
    QString ResolveAssetRef(QString context, QString ref) const;

    /// Given an assetRef, turns it into a native OS file path to the asset.
    /** The given ref is resolved in the context of "local://", if it is a relative asset ref.
        If ref contains a subAssetName, it is stripped from outFilePath, and returned in subAssetName.
        If the assetRef doesn't represent a file on the local filesystem, FileQueryExternalFile is returned and outFilePath is set to equal best effort to parse 'ref' locally. */
    FileQueryResult ResolveLocalAssetPath(QString ref, QString baseDirectoryContext, QString &outFilePath, QString *subAssetName = 0) const;

    /// Recursively iterates through the given path and all its subdirectories and tries to find the given file.
    /** Returns the absolute path for that file, if it exists. The path contains the filename,
        i.e. it is of form "C:\folder\file.ext" or "/home/username/file.ext". */
    static QString RecursiveFindFile(QString basePath, QString filename);

    /// Removes the given asset from the system and frees up all resources related to it.
    /** Any assets depending on this asset will break.
        @param assetRef A valid assetRef that is in the asset system. If this asset ref does not exist, this call will do nothing.
        @param removeDiskSource If true, the disk source of the asset is also deleted. In most cases, this is the locally cached version of the remote file,
                but for example for local assets, this is the asset itself.
        @note Calling ForgetAsset on an asset will unload it from the system. Do not dereference the asset after calling this function. */
    void ForgetAsset(AssetPtr asset, bool removeDiskSource);
    void ForgetAsset(QString assetRef, bool removeDiskSource); /**< @overload */

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
    AssetUploadTransferPtr UploadAssetFromFile(const QString &filename, AssetStoragePtr destination, const QString &assetName);

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
    AssetUploadTransferPtr UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName);

    /// Unloads all known assets, and removes them from the list of internal assets known to the Asset API.
    /** Use this to clear the client's memory from all assets.
        @note There may be any number of strong references to assets in other parts of code, in which case the assets are not deleted
        until the refcounts drop to zero.
        @note Do not dereference any asset pointers that might have been left over after calling this function. */
    void ForgetAllAssets();

    /// Cleans up everything in the Asset API.
    /** Forgets all assets, kills all asset transfers, frees all storages, providers, and type factories.
        Deletes the asset cache and the disk watcher. */
    void Reset();

    /// Returns a pointer to an existing asset transfer if one is in-progress for the given assetRef.
    /** Returns a null pointer if no transfer exists, in which case the asset may already have been loaded to the system (or not).
        It can be that an asset is loaded to the system, but one or more of its dependencies have not, in which case there exists
        both an IAssetTransfer and IAsset to this particular assetRef (so the existence of these two objects is not mutually exclusive).
        @note Client code should not need to worry about whether a particular transfer is pending or not, but simply call RequestAsset whenever an asset
        request is needed. AssetAPI will optimize away any duplicate transfers to the same asset. */
    AssetTransferPtr GetPendingTransfer(QString assetRef) const;

    /// Starts an asset transfer for each dependency the given asset has.
    void RequestAssetDependencies(AssetPtr transfer);

    /// A utility function that counts the number of dependencies the given asset has to other assets that have not been loaded in.
    int NumPendingDependencies(AssetPtr asset) const;

    /// A utility function that returns true if the given asset still has some unloaded dependencies left to process.
    /// @note For performance reasons, calling this function is highly advisable instead of calling NumPendingDependencies, if it is only
    ///       desirable to known whether the asset has any pending dependencies or not.
    bool HasPendingDependencies(AssetPtr asset) const;

    /// Handle discovery of a new asset through the AssetDiscovery network message
    void HandleAssetDiscovery(const QString &assetRef, const QString &assetType);

    /// Handle deletion of an asset through the AssetDeleted network message
    void HandleAssetDeleted(const QString &assetRef);
    
    /// Cause AssetAPI to emit AssetDeletedFromStorage. Called by asset providers
    void EmitAssetDeletedFromStorage(const QString &assetRef);
    
    /// Called by all providers to inform Asset API whenever they have added a new asset storage to the provider.
    void EmitAssetStorageAdded(AssetStoragePtr newStorage);

    /// Return current asset transfers
    AssetTransferMap CurrentTransfers() const { return currentTransfers; }

    /// A utility function that counts the number of current asset transfers.
    size_t NumCurrentTransfers() const { return currentTransfers.size(); }
    
    /// Return the current asset dependency map (debugging)
    const AssetDependenciesMap& DebugGetAssetDependencies() const { return assetDependencies; }
    
    /// Return ready asset transfers (debugging)
    const std::vector<AssetTransferPtr>& DebugGetReadyTransfers() const { return readyTransfers; }

    // DEPRECATED
    AssetMap GetAllAssets() const { return Assets(); } /**< @deprecated Use Assets instead @todo Add warning print in some distant future */
    AssetMap GetAllAssetsOfType(const QString& type) const { return AssetsOfType(type); } /**< @deprecated Use AssetsOfType instead @todo Add warning print in some distant future */
    AssetStorageVector GetAssetStorages() const { return AssetStorages(); } /**< @deprecated Use AssetStorages instead @todo Add warning print in some distant future */
    AssetProviderPtr GetProviderForAssetRef(QString assetRef, QString assetType = "") const { return ProviderForAssetRef(assetRef, assetType); }  /**< @deprecated Use ProviderForAssetRef instead @todo Add warning print in some distant future */
    AssetTypeFactoryPtr GetAssetTypeFactory(QString typeName) const { return AssetTypeFactory(typeName); } /**< @deprecated Use AssetTypeFactory instead @todo Add warning print in some distant future */
    AssetCache *GetAssetCache() const { return Cache(); } /**< @deprecated Use Cache instead @todo Add warning print in some distant future */
    AssetStoragePtr GetAssetStorageByName(const QString &name) const { return AssetStorageByName(name); } /**< @deprecated Use AssetStorageByName instead @todo Add warning print in some distant future */
    AssetStoragePtr GetStorageForAssetRef(const QString& ref) const { return StorageForAssetRef(ref); } /**< @deprecated Use Cache instead @todo Add warning print in some distant future */
    AssetStoragePtr GetDefaultAssetStorage() const { return DefaultAssetStorage(); } /**< @deprecated Use DefaultAssetStorage instead @todo Add warning print in some distant future */
    AssetTransferMap GetCurrentTransfers() const { return CurrentTransfers(); } /**< @deprecated Use CurrentTransfers instead @todo Add warning print in some distant future */
    AssetBundleMap GetAllAssetBundles() const { return AssetBundles(); } /**< @deprecated Use AssetBundles instead @todo Add warning print in some distant future */
    AssetBundleTypeFactoryPtr GetAssetBundleTypeFactory(const QString &typeName) const { return AssetBundleTypeFactory(typeName); } /**< @deprecated Use AssetBundleTypeFactory instead @todo Add warning print in some distant future */
    QString GetResourceTypeFromAssetRef(QString assetRef) const { return ResourceTypeForAssetRef(assetRef); } /**< @deprecated Use ResourceTypeForAssetRef instead @todo Add warning print in some distant future */
    QString GetResourceTypeFromAssetRef(const AssetReference &ref) const { return ResourceTypeForAssetRef(ref); } /**< @deprecated Use ResourceTypeForAssetRef instead @todo Add warning print in some distant future */

signals:
    /// Emitted for each new asset that was created and added to the system.
    /** When this signal is triggered, the dependencies of an asset may not yet have been loaded. */
    void AssetCreated(AssetPtr asset);

    /// Emitted before an asset is going to be forgotten.
    void AssetAboutToBeRemoved(AssetPtr asset);

    /// Emitted before an assets disk source will be removed.
    void DiskSourceAboutToBeRemoved(AssetPtr asset);
    
    /// An asset's disk source has been modified. Practically only emitted for files in the asset cache.
    void AssetDiskSourceChanged(AssetPtr asset);
    
    /// Emitted when an asset has been uploaded
    void AssetUploaded(const QString &assetRef);

    /// Emitted when asset was confirmedly deleted from storage
    void AssetDeletedFromStorage(const QString &assetRef);

    /// Emitted when an asset storage has been added
    void AssetStorageAdded(AssetStoragePtr storage);
    
    /// Emitted when the contents of an asset disk source has changed. ///\todo Implement.
    //void AssetDiskSourceChanged(AssetPtr asset);

    /// Emitted when the asset has changed in the remote AssetStorage it is in. ///\todo Implement.
    //void AssetStorageSourceChanged(AssetPtr asset);

private slots:
    /// The Asset API listens on each asset when they get loaded, to track the completion of the dependencies of other loaded assets.
    void OnAssetLoaded(AssetPtr asset);

    /// The Asset API reloads all assets from file when their disk source contents change.
    void OnAssetDiskSourceChanged(const QString &path);

    /// An asset storage refreshed its references. Create empty assets from the new refs as necessary
    ///\todo Delete this whole function and logic when OnAssetChanged is implemented
    //void OnAssetStorageRefsChanged(AssetStoragePtr storage);

    /// Contents of asset storage has been changed.
    void OnAssetChanged(QString localName, QString diskSource, IAssetStorage::ChangeType change);

    /// Listens to the IAssetBundle Loaded signal.
    void AssetBundleLoadCompleted(IAssetBundle *bundle);

    /// Listens to the IAssetBundle Failed signal.
    void AssetBundleLoadFailed(IAssetBundle *bundle);

private:
    AssetTransferMap::iterator FindTransferIterator(QString assetRef);
    AssetTransferMap::const_iterator FindTransferIterator(QString assetRef) const;

    AssetTransferMap::iterator FindTransferIterator(IAssetTransfer *transfer);
    AssetTransferMap::const_iterator FindTransferIterator(IAssetTransfer *transfer) const;

    /// Removes from AssetDependenciesMap all dependencies the given asset has.
    void RemoveAssetDependencies(QString asset);

    /// Handle discovery of a new asset, when the storage is already known. This is used internally for optimization, so that providers don't need to be queried
    void HandleAssetDiscovery(const QString &assetRef, const QString &assetType, AssetStoragePtr storage);
    
    /// Create new asset, when the storage is already known. This is used internally for optimization
    AssetPtr CreateNewAsset(QString type, QString name, AssetStoragePtr storage);

    /// Load sub asset to transfer. Used internally for loading sub asset from bundle to virtual transfers.
    bool LoadSubAssetToTransfer(AssetTransferPtr transfer, const QString &bundleRef, const QString &fullSubAssetRef, QString subAssetType = QString());

    /// Overload that takes in AssetBundlePtr instead of refs.
    bool LoadSubAssetToTransfer(AssetTransferPtr transfer, IAssetBundle *bundle, const QString &fullSubAssetRef, QString subAssetType = QString());

    bool isHeadless;

    /// Stores all the currently ongoing asset transfers.
    AssetTransferMap currentTransfers;

    /// Stores all the currently ongoing asset bundle monitors.
    AssetBundleMonitorMap bundleMonitors;

    typedef std::map<QString, AssetUploadTransferPtr, QStringLessThanNoCase> AssetUploadTransferMap;
    /// Stores all the currently ongoing asset uploads, maps full assetRefs to the asset upload transfer structures.
    AssetUploadTransferMap currentUploadTransfers;

    /// Keeps track of all the dependencies each asset has to each other asset.
    /// \todo Find a more effective data structure for this. Needs something like boost::bimap but for multi-indices.
    AssetDependenciesMap assetDependencies;

    /// Stores a list of asset requests to assets that have already been downloaded into the system. These requests don't go to the asset providers
    /// to process, but are internally filled by the Asset API. This member vector is needed to be able to delay the requests and virtual completions
    /// by one frame, so that the client gets a chance to connect his handler's Qt signals to the AssetTransferPtr slots.
    std::vector<AssetTransferPtr> readyTransfers;
    
    // Stores a list of sub asset requests that are pending a load from a loaded asset bundle.
    std::vector<SubAssetLoader> readySubTransfers;

    /// Contains all known asset storages in the system.
    //std::vector<AssetStoragePtr> storages;

    /// Specifies the storage to use for asset requests with local name only.
    AssetStorageWeakPtr defaultStorage;

    /// Stores all the registered asset type factories in the system.
    std::vector<AssetTypeFactoryPtr> assetTypeFactories;

    /// Stores all the registered asset bundle type factories in the system.
    std::vector<AssetBundleTypeFactoryPtr> assetBundleTypeFactories;

    /// Stores a list of asset requests that the Asset API hasn't started at all but has put on hold, until other operations complete.
    /// This data structure is used to enforce that asset uploads are completed before any asset downloads to that asset.
    struct PendingDownloadRequest
    {
        QString assetRef;
        QString assetType;
        AssetTransferPtr transfer;
    };
    typedef std::map<QString, PendingDownloadRequest, QStringLessThanNoCase> PendingDownloadRequestMap;
    PendingDownloadRequestMap pendingDownloadRequests;

    /// Stores all the already loaded assets in the system.
    AssetMap assets;

    /// Stores all the already loaded asset bundles in the system.
    AssetBundleMap assetBundles;

    /// Tracks all loaded assets if their DiskSources change, and issues a reload of the assets.
    QFileSystemWatcher *diskSourceChangeWatcher;

    /// Specifies all the registered asset providers in the system.
    std::vector<AssetProviderPtr> providers;

    Framework *fw;
    AssetCache *assetCache;
};

#include "AssetAPI.inl"
