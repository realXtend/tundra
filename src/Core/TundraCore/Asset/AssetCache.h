// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"
#include "AssetFwd.h"

#include <QString>
#include <QDir>
#include <QObject>
#include <QDateTime>

/// Implements a disk cache for asset files to avoid re-downloading assets between runs.
class TUNDRACORE_API AssetCache : public QObject
{
    Q_OBJECT

public:
    explicit AssetCache(AssetAPI *owner, QString assetCacheDirectory);

public slots:
    /// Returns the absolute path on the local file system that contains a cached copy of the given asset ref.
    /// If the given asset file does not exist in the cache, an empty string is returned.
    /// @param assetRef The asset reference URL, which must be of type AssetRefExternalUrl.
    QString FindInCache(const QString &assetRef);

    /// Returns the absolute path on the local file system for the cached version of the given asset ref.
    /// This function is otherwise identical to FindInCache, except this version does not check whether the asset exists 
    /// in the cache, but simply returns the absolute path where the asset would be stored in the cache.
    /// @param assetRef The asset reference URL, which must be of type AssetRefExternalUrl.
    QString GetDiskSourceByRef(const QString &assetRef);
    
    /// Saves the given asset to cache.
    /// @return QString the absolute path name to the asset cache entry. If not successful returns an empty string.
    QString StoreAsset(AssetPtr asset);

    /// Saves the specified data to the asset cache.
    /// @return QString the absolute path name to the asset cache entry. If not successful returns an empty string.
    QString StoreAsset(const u8 *data, size_t numBytes, const QString &assetName);

    /// Return the last modified date and time for assetRefs cache file.
    /// If cache file does not exist for assetRef return invalid QDateTime. You can check return value with QDateTime::isValid().
    /// @param QString assetRef Asset reference thats cache file last modified date and time will be returned.
    /// @return QDateTime Last modified date and time of the cache file.
    QDateTime LastModified(const QString &assetRef);

    /// Sets the last modified date and time for the assetRefs cache file.
    /// @param QString assetRef Asset reference thats cache file last modified date and time will be set.
    /// @param QDateTime The date and time to set.
    /// @return bool Returns true if successful, false otherwise.
    bool SetLastModified(const QString &assetRef, const QDateTime &dateTime);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QString asset reference.
    void DeleteAsset(const QString &assetRef);

    /// Deletes all data and metadata files from the asset cache.
    /// Will not clear sub folders in the cache folders, or remove any folders.
    void ClearAssetCache();

    /// Get the cache directory. Returned path is guaranteed to have a trailing slash /.
    /// @return QString absolute path to the caches data directory
    QString CacheDirectory() const;
    
private:
#ifdef Q_WS_WIN
    /// Windows specific helper to open a file handle to absolutePath
    void *OpenFileHandle(const QString &absolutePath);
#endif
    /// Cache directory, passed here from AssetAPI in the ctor.
    QString cacheDirectory;

    /// AssetAPI ptr.
    AssetAPI *assetAPI;

    /// Asset data dir.
    QDir assetDataDir;
};
