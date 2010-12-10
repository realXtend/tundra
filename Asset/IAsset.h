// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAsset_h
#define incl_Asset_IAsset_h

#include <boost/enable_shared_from_this.hpp>
#include <QObject>
#include <vector>

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

class IAsset : public QObject, public boost::enable_shared_from_this<IAsset>
{
    Q_OBJECT

public:
    IAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    /// The base class destructor does nothing. However, each asset subclass is expected to Unload itself in the destructor. Especially, any assets
    /// that create any Ogre asset types into internal Ogre pools must guarantee that at dtor (and at Unload()) these resources from Ogre pools are completely cleared.
    virtual ~IAsset() {}

public slots:
    /// Returns the type of this asset. The type of an asset cannot change during the lifetime of the instance of an asset.
    QString Type() const { return type; }

    /// Returns the unique name of this asset. The name of an asset cannot change during the lifetime of the instance of an asset.
    QString Name() const { return name; }

    /// Returns a SHA-1 hash of the contents of this asset in readable ASCII string form containing hex bytes. Can return an empty string if the asset is not loaded
    /// or if the hash was not computed.
    QString ContentHash() const { return contentHash; }

    /// Specifies the file from which this asset can be reloaded, if it is unloaded in between. 
    void SetDiskSource(QString diskSource);

    /// Returns the absolute path name to the file that contains the disk-cached version of this asset. For some assets, this is the cache file containing this asset,
    /// but for assets from some providers (e.g. LocalAssetProvider), this is the actual source filename of the asset.
    QString DiskSource() const { return diskSource; }

    /// Loads this asset from the given file on the local filesystem. Returns true if loading succeeds, false otherwise.
    bool LoadFromFile(QString filename);

    /// Forces a reload of this asset from the local disk cache. Returns true if loading succeeded, false otherwise.
    bool LoadFromCache();

    /// Unloads this asset from memory. After calling this function, this asset still can be queried for its Type(), Name() and CacheFile(),
    /// but its dependencies cannot be determined and it cannot be used in any other way.
    virtual void Unload() = 0;

    /// Stores the *current in-memory copy* of this asset to disk to the given file on the local filesystem. Use this function to export an asset from the system to a file.
    /// Returns true if saving succeeded, false otherwise.
    /// The default implementation immediately returns false for the asset.
    /// @param serializationParameters Optional parameters for the actual asset type serializer that specifies custom options on how to perform the serialization.
    virtual bool SaveToFile(const QString &filename, const QString &serializationParameters = "");

    /// Copies the disk cache version of this asset to the specified file. 
    bool SaveCachedCopyToFile(const QString &filename);

    /// Returns the asset storage this asset was loaded from.
    AssetStoragePtr GetAssetStorage();

    /// Returns the asset provider this asset was loaded from.
    AssetProviderPtr GetAssetProvider();

    /// Returns a textual human-readable representation of this asset in the form "<name> (<type>)".
    QString ToString() const;

public:
    /// Loads this asset from the specified file data in memory. Loading an asset from memory cannot change its name or type.
    /// Returns true if loading succeeded, false otherwise.
    bool LoadFromFileInMemory(const u8 *data, size_t numBytes);

    /// Called whenever another asset this asset depends on is loaded.
    virtual void DependencyLoaded(AssetPtr dependee) { }

    /// Returns all the assets this asset refers to (but not the references those assets refer to).
    /// The default implementation of this function returns an empty list of dependencies.
    virtual std::vector<AssetReference> FindReferences() const { return std::vector<AssetReference>(); }

    /// Returns true if the replace succeeds.
//    bool ReplaceReference(const QString &oldRef, const QString &newRef);

    /// Returns all the assets this asset refers to, and the assets those assets refer to, and so on.
    std::vector<AssetReference> FindReferencesRecursive() const;

    /// Saves the provider this asset was downloaded from. Intended to be only called internally by Asset API at asset load time.
    void SetAssetProvider(AssetProviderPtr provider);

    /// Saves the storage this asset was downloaded from. Intended to be only called internally by Asset API at asset load time.
    void SetAssetStorage(AssetStoragePtr storage); 

    /// Saves this asset to the given data buffer. Returns true on success. If this asset is unloaded, will return false.
    /// @param serializationParameters Optional parameters for the actual asset type serializer that specifies custom options on how to perform the serialization.
    virtual bool SerializeTo(std::vector<u8> &data, const QString &serializationParameters = "");
private:
    /// Loads this asset by deserializing it from the given data. The data pointer that is passed in is never null, and numBytes is always greater than zero.
    virtual bool DeserializeFromData(const u8 *data, size_t numBytes) = 0;

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

    /// Stores the SHA-1 hash of this content, saved as a string for convenience for script access. 
    QString contentHash;
};

#endif
