// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_AssetCache_h
#define incl_Asset_AssetCache_h

#include <QString>

#include "CoreTypes.h"
#include "AssetFwd.h"

class AssetCache
{
public:
    explicit AssetCache(QString assetCacheDirectory);

    /// Checks whether the asset cache contains an asset with the given ref, and returns the absolute path name to it, if so.
    /// Otherwise returns an empty string.
    QString FindAsset(QString assetRef);

    /// Checks whether the asset cache contains an asset with the given content hash, and returns the absolute path name to it, if so.
    /// Otherwise returns an empty string.
    QString FindAssetByContentHash(QString contentHash);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    void DeleteAsset(QString assetRef);

    /// Deletes all assets in the cache directory.
    void ClearAssetCache();

    /// Saves the specified data to the asset cache.
    /// Returns the absolute path name to the 
    QString StoreAsset(const u8 *data, size_t numBytes, QString assetName, QString assetContentHash);

    /// Saves the given asset to cache.
    void StoreAsset(AssetPtr asset);

    QString GetCacheDirectory() const;

private:
    QString cacheDirectory;
};

#endif
