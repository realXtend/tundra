// For conditions of distribution and use, see copyright notice in license.txt

// A note about the filename: This file will be renamed to AssetCache_.h at the moment the old AssetCache.h from AssetModule is deleted.

#pragma once

#include <QString>
#include <QNetworkDiskCache>
#include <QNetworkCacheMetaData>
#include <QHash>
#include <QUrl>
#include <QDir>
#include <QObject>

#include "CoreTypes.h"
#include "AssetFwd.h"

class QNetworkDiskCache;

/// An utility function that takes an assetRef and makes a string out of it that can safely be used as a part of a filename.
/// Replaces characters / \ : * ? " ' < > | with _
QString SanitateAssetRefForCache(QString assetRef);

/// Subclassing QNetworkDiskCache has the main goal of separating metadata from the raw asset data. The basic implementation of QNetworkDiskCache
/// will store both in the same file. That did not work very well with our asset system as we need absolute paths to loaded assets for various purpouses.
class AssetCache : public QNetworkDiskCache
{

Q_OBJECT

public:
    explicit AssetCache(AssetAPI *owner, QString assetCacheDirectory);

    /// Allocates new QFile*, it is the callers responsibility to free the memory once done with it.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QIODevice* data(const QUrl &url);
    
    /// Frees allocated QFile* that was prepared in prepare().
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void insert(QIODevice* device);

    /// Allocates new QFile*, the data is freed in either insert() or remove(), 
    /// remove() cancels the preparation and insert() finishes it.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QIODevice* prepare(const QNetworkCacheMetaData &metaData);
    
    /// Frees allocated QFile* if one was prepared in prepare().
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual bool remove(const QUrl &url);

    /// Reads metadata file to hard drive.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QNetworkCacheMetaData metaData(const QUrl &url);

    /// Writes metadata file to hard drive, if metadata is different from known cache metadata.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void updateMetaData(const QNetworkCacheMetaData &metaData);

    /// Deletes all data and metadata files from the asset cache.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void clear();

    /// Checks if asset cache is currently over the maximum limit.
    /// This call is ignored untill we decide to limit the disk cache size.
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual qint64 expire();

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
    /// @return QString the absolute path name to the asset cache entry. If not successfull returns an empty string.
    QString StoreAsset(AssetPtr asset);

    /// Saves the specified data to the asset cache.
    /// @return QString the absolute path name to the asset cache entry. If not successfull returns an empty string.
    QString StoreAsset(const u8 *data, size_t numBytes, const QString &assetName);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QString asset reference.
    void DeleteAsset(const QString &assetRef);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QUrl asset reference url.
    void DeleteAsset(const QUrl &assetUrl);

    /// Deletes all data and metadata files from the asset cache.
    /// Will not clear subfolders in the cache folders, or remove any folders.
    void ClearAssetCache();

private slots:
    /// Writes metadata into a file. Helper function for the QNetworkDiskCache overrides.
    bool WriteMetadata(const QString &filePath, const QNetworkCacheMetaData &metaData);

    /// Genrates the absolute path to an asset cache entry. Helper function for the QNetworkDiskCache overrides.
    QString GetAbsoluteFilePath(bool isMetaData, const QUrl &url);

    /// Genrates the absolute path to an data asset cache entry.
    QString GetAbsoluteDataFilePath(const QString &filename);

    /// Removes all files from a directory. Will not delete the folder itself or any subfolders it has.
    void ClearDirectory(const QString &absoluteDirPath);

private:
    /// Cache directory, passed here from AssetAPI in the ctor.
    QString cacheDirectory;

    /// AssetAPI ptr.
    AssetAPI *assetAPI;

    /// Asset data dir.
    QDir assetDataDir;

    /// Asset metadata dir.
    QDir assetMetaDataDir;

    /// Internal tracking of prepared QUrl to QIODevice pairs.
    QHash<QString, QFile*> preparedItems;
};

