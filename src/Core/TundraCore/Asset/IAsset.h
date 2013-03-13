// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

#include <QObject>
#include <vector>

/// Base class for all assets loaded in the system.
class TUNDRACORE_API IAsset : public QObject, public enable_shared_from_this<IAsset>
{
    Q_OBJECT
    Q_ENUMS(SourceType)

public:
    IAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    /// The base class destructor does nothing.
    /** However, each asset subclass is expected to Unload itself in the destructor. Especially, any assets that create  any Ogre asset 
        types into internal Ogre pools must guarantee that at dtor (and at Unload()) these resources from Ogre pools are completely cleared. */
    virtual ~IAsset() {}

    enum SourceType
    {
        /// The disk source is the authoritative copy of the asset
        Original = 0,
        /// The disk source is a cached copy of the asset
        Cached,
        /// The asset is programmatically created. This is the default for empty new assets.
        Programmatic,
        /// The asset is a sub asset from a bundle.
        Bundle
    };
    
public slots:
    /// Returns the type of this asset. The type of an asset cannot change during the lifetime of the instance of an asset.
    QString Type() const { return type; }

    /// Returns the unique name of this asset. The name of an asset cannot change during the lifetime of the instance of an asset.
    QString Name() const { return name; }

    /// Specifies the file from which this asset can be reloaded, if it is unloaded in between. 
    void SetDiskSource(const QString &diskSource);

    /// Sets the disk source type for this asset.
    void SetDiskSourceType(SourceType type);

    /// Returns the absolute path name to the file that contains the disk-cached version of this asset.
    /** For some assets, this is the cache file containing this asset, but for assets from some providers (e.g. LocalAssetProvider),
        this is the actual source filename of the asset. */
    QString DiskSource() const { return diskSource; }

    /// Returns the disk source type of this asset.
    SourceType DiskSourceType() const { return diskSourceType; }
    
    /// Loads this asset from the given file on the local filesystem. Returns true if loading succeeds, false otherwise.
    virtual bool LoadFromFile(QString filename);

    /// Forces a reload of this asset from its disk source. Returns true if loading succeeded, false otherwise.
    bool LoadFromCache();

    /// Unloads this asset from memory.
    /** After calling this function, this asset still can be queried for its Type(), Name() and CacheFile(),
        but its dependencies cannot be determined and it cannot be used in any other way. */
    void Unload();

    /// Returns true if this asset is loaded in memory, and is ready to use.
    /// An asset can be in an unloaded state, to save memory. In this state the asset can be reloaded from its DiskSource() to enable using it.
    virtual bool IsLoaded() const = 0;

    /// Returns true if the asset is empty. An empty asset is unloaded, and has an empty disk source.
    bool IsEmpty() const;

    /// Returns true if this asset content is trusted.
    bool IsTrusted() const;

    /// Marks the asset to be have been modified in memory.
    void MarkModified();
    
    /// Clears the modified in memory -status.
    void ClearModified();
    
    /// Returns true if the asset has been modified in memory without saving to the source.
    bool IsModified() const { return modified; }
    
    /// Makes a clone of this asset.
    /** For this function to succeed, the asset must be loaded in memory. (IsLoaded() == true)
        @param newAssetName The name for the new asset. This will be the 'assetRef' of the new asset. You will use AssetAPI::GetAsset(newAssetName) to get
            a reference to the new asset.
        @note The default implementation is able to clone each asset by first serializing the old asset to a buffer, and then deserializing that buffer onto a newly
            created asset object of the same type than this asset. A subclass derived from IAsset can reimplement this method if it can implement a more efficient
            mechanism for cloning itself. */
    virtual AssetPtr Clone(QString newAssetName) const;

    /// Stores the *current in-memory copy* of this asset to disk to the given file on the local filesystem.
    /** Use this function to export an asset from the system to a file.
        The default implementation immediately returns false for the asset.
        @param serializationParameters Optional parameters for the actual asset type serializer that specifies
            custom options on how to perform the serialization.
        @return True if saving succeeded, false otherwise. */
    virtual bool SaveToFile(const QString &filename, const QString &serializationParameters = "") const;

    /// Copies the disk cache version of this asset to the specified file. 
    bool SaveCachedCopyToFile(const QString &filename);

    /// Returns the asset storage this asset was loaded from.
    AssetStoragePtr AssetStorage() const;

    /// Returns the asset provider this asset was loaded from.
    AssetProviderPtr AssetProvider() const;

    /// Returns a textual human-readable representation of this asset in the form "<name> (<type>)".
    QString ToString() const;

    // Returns a copy of the raw data in this asset.
    QByteArray RawData(const QString serializationParameters = "") const;

    // DEPRECATED
    AssetStoragePtr GetAssetStorage() const { return AssetStorage(); } /**< @deprecated Use AssetStorage @todo Add warning print in some distant future */
    AssetProviderPtr GetAssetProvider() const { return AssetProvider(); } /**< @deprecated Use AssetProvider @todo Add warning print in some distant future */
    QByteArray GetRawData(const QString serializationParameters = "") const { return RawData(serializationParameters); } /**< @deprecated Use RawData @todo Add warning print in some distant future */

signals:
    /// This signal is emitted when the contents of this asset is unloaded.
    /** It might be due to an explicit call by client code to IAsset::Unload, or it could be just prior to removing this asset from the system (perhaps in dtor at shutdown).
        @note After unloading an asset, the instance of IAsset can still remain in memory, and can be Reload()ed from its disk source or source storage later. */
    void Unloaded(IAsset *asset);

    /// This signal is emitted when the contents of this asset have been loaded from new data.
    /** It might be the first time this asset was loaded, or it might also be that this asset was reloaded from its disk source.
        When this signal is emitted after a call to RequestAsset, all the dependencies of this asset will have been loaded in.
        @todo The dependencies may not have been loaded in if the reload changes them! fix!
        @param asset A pointer to this will be passed in. The signature of this signal deliberately contains this member to be unified with AssetAPI. */
    void Loaded(AssetPtr asset);

    /// Asset properties have changed. Emitted whenever the modified flag, disksource, or disksourcetype changes.
    void PropertyStatusChanged(IAsset *asset);
    
public:
    /// Loads this asset from the specified file data in memory.
    /** Loading an asset from memory cannot change its name or type.
        @param data A pointer to the data to be loaded in. This pointer may be null if numBytes == 0, in which case this function is used to signal loading into "null data".
        @param allowAsynchronous Informs the underlying load code if it can do asynchronous load.
            Typically large sized asset types want to ignore the parameter data and load from a cached disk file if possible and notify AssetAPI when its done.
            This should be set to false if you are expecting the asset to be loaded when this function returns like in LoadFromFile and LoadFromCache.
        @return true if loading succeeded, false otherwise. */
    bool LoadFromFileInMemory(const u8 *data, size_t numBytes, bool allowAsynchronous = true);

    /// Called when this asset is loaded by AssetAPI::AssetLoadCompleted and DependencyLoaded functions.
    /// Emits Loaded() signal if all the dependencies have been loaded, otherwise does nothing.
    void LoadCompleted();

    /// Called whenever another asset this asset depends on is loaded.
    /** The default implementation will check if the asset itself is loaded, and the
        number of dependencies: if it was the last dependency, Loaded() will be emitted. */
    virtual void DependencyLoaded(AssetPtr dependee);

    /// Returns all the assets this asset refers to (but not the references those assets refer to).
    /// The default implementation of this function returns an empty list of dependencies.
    virtual std::vector<AssetReference> FindReferences() const { return std::vector<AssetReference>(); }

    /// Returns true if the replace succeeds.
    //bool ReplaceReference(const QString &oldRef, const QString &newRef);

    /// Returns all the assets this asset refers to, and the assets those assets refer to, and so on.
    std::vector<AssetReference> FindReferencesRecursive() const;

    /// Saves the provider this asset was downloaded from. Intended to be only called internally by Asset API at asset load time.
    void SetAssetProvider(AssetProviderPtr provider);

    /// Saves the storage this asset was downloaded from. Intended to be only called internally by Asset API at asset load time.
    void SetAssetStorage(AssetStoragePtr storage); 

    /// Saves this asset to the given data buffer. Returns true on success. If this asset is unloaded, will return false.
    /// @param serializationParameters Optional parameters for the actual asset type serializer that specifies custom options on how to perform the serialization.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "") const;

protected:
    /// Loads this asset by deserializing it from the given data.
    /** The data pointer that is passed in is never null, and numBytes is always greater than zero.
        The allowAsynchronous boolean must be respected, if it is false you should not do asynchronous even if you have a code path for it.
        The parameter is set to false when the requesting code is expecting the asset to be loaded when this function returns.
        @note Implementation has to call AssetAPI::AssetLoadCompleted after loaded succesfully (both synchronous and asynchronous).
        AssetAPI::AssetLoadCompleted can be called inside this function, how ever just returning true is not enough.
        AssetAPI::AssetLoadFailed will be called automatically if false is returned. */
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous) = 0;

    /// Private-implementation of the unloading of an asset.
    virtual void DoUnload() = 0;

    AssetAPI *assetAPI;

    /// Specifies the provider this asset was downloaded from. May be null.
    AssetProviderWeakPtr provider;

    /// Specifies the storage this asset was downloaded from. May be null.
    AssetStorageWeakPtr storage;

    /// Specifies the type of this asset, e.g. "Texture" or "OgreMaterial".
    QString type;

    /// Specifies the name of this asset, which for most assets is the source URL ref of where the asset was loaded.
    QString name;

    /// This path specifies a local filename from which this asset can be reloaded if necessary.
    QString diskSource;
    
    /// Disk source type of the asset.
    SourceType diskSourceType;
    
    /// Modified in memory -status of the asset.
    bool modified;
};
