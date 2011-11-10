// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QString>
#include <QUrl>
#include <QDir>
#include <QObject>
#include <QDateTime>

#include "CoreTypes.h"
#include "AssetFwd.h"

/// Implements a disk cache for asset files to avoid re-downloading assets between runs.
class AssetCache : public QObject
{
    Q_OBJECT

public:
    explicit AssetCache(AssetAPI *owner, QString assetCacheDirectory);

public slots:
    /// Gets absolute file path to disk source for asset ref.
    /// @note This is deprecated since ~2.1.4. Remove this functions when scripts etc. 3rd party code have migrated to using GetDiskSourceByRef().
    QString FindInCache(const QString &assetRef);

    /// Gets absolute file path to disk source for asset ref.
    /// @param assetRef Asset reference to return the disk source.
    QString GetDiskSourceByRef(const QString &assetRef);
    
    /// Saves the given asset to cache.
    /// @return QString the absolute path name to the asset cache entry. If not successful returns an empty string.
    QString StoreAsset(AssetPtr asset);

    /// Saves the specified data to the asset cache.
    /// @return QString the absolute path name to the asset cache entry. If not successful returns an empty string.
    QString StoreAsset(const u8 *data, size_t numBytes, const QString &assetName);

    /// Return the last modified date and time for assetRefs cache file.
    /// If cache file does not exist for assetRef return invalid QDateTime. You can check return valud with QDateTime::isValid().
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
    
private slots:
    /// Generates the absolute path to an data asset cache entry.
    QString GetAbsoluteDataFilePath(const QString &filename);

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
