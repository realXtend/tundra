// For conditions of distribution and use, see copyright notice in license.txt

// A note about the filename: This file will be renamed to AssetCache_.h at the moment the old AssetCache.h from AssetModule is deleted.

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
    /// Searches if the cache contains the asset with the given assetRef. Returns an absolute path to the asset on the local file system, if it is found.
    /// @return An absolute path to the assets disk source, or an empty string if asset is not in the cache.
    /// @note Currently this will always return an empty string for http/https assets. This will force the AssetAPI to check that it has the latest asset from the source.
    QString FindInCache(const QString &assetRef);

    /// Gets disk source for asset ref, disregarding the http protocol check in FindInCache()
    /// This is only used by AssetAPI as an emergency mechanism for querying http asset disk source after loading
    QString GetDiskSourceByRef(const QString &assetRef);
    
    /// Get the cache directory. Returned path is guaranteed to have a trailing slash /.
    /// @return QString absolute path to the caches data directory
    QString GetCacheDirectory() const;

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

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QUrl asset reference url.
    void DeleteAsset(const QUrl &assetUrl);

    /// Deletes all data and metadata files from the asset cache.
    /// Will not clear sub folders in the cache folders, or remove any folders.
    void ClearAssetCache();

    /// Returns cache directory
    const QString& CacheDirectory() const { return cacheDirectory; }
    
private slots:
    /// Generates the absolute path to an asset cache entry. Helper function for the QNetworkDiskCache overrides.
    QString GetAbsoluteFilePath(bool isMetaData, const QUrl &url);

    /// Generates the absolute path to an data asset cache entry.
    QString GetAbsoluteDataFilePath(const QString &filename);

    /// Removes all files from a directory. Will not delete the folder itself or any subfolders it has.
    void ClearDirectory(const QString &absoluteDirPath);

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

