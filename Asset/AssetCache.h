// For conditions of distribution and use, see copyright notice in license.txt

// A note about the filename: This file will be renamed to AssetCache_.h at the moment the old AssetCache.h from AssetModule is deleted.

#ifndef incl_Asset_AssetCache_h
#define incl_Asset_AssetCache_h

#include <QString>
#include <QNetworkDiskCache>
#include <QNetworkCacheMetaData>
#include <QHash>
#include <QUrl>
#include <QDir>

#include "CoreTypes.h"
#include "AssetFwd.h"

class QNetworkDiskCache;

/// An utility function that takes an assetRef and makes a string out of it that can safely be used as a part of a filename.
/// Cleans characters / \ :
QString SanitateAssetRefForCache(QString assetRef);

class AssetCache : public QNetworkDiskCache
{
public:
    explicit AssetCache(AssetAPI *owner, QString assetCacheDirectory);

    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QIODevice* data(const QUrl &url);
    
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void insert(QIODevice* device);
    
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QNetworkCacheMetaData metaData(const QUrl &url);
    
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual QIODevice* prepare(const QNetworkCacheMetaData &metaData);
    
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual bool remove(const QUrl &url);
    
    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void updateMetaData(const QNetworkCacheMetaData &metaData);

    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual void clear();

    /// QNetworkDiskCache override. Don't call directly, used by QNetworkAccessManager.
    virtual qint64 expire();

public slots:
    /// Returns an absolute path to a disk source of the url.
    /// @param QString asset ref
    /// @return QString absolute path to the assets disk source. Return empty string if asset is not in the cache.
    /// @note this will always return an empty string for http/https assets. This will force the AssetAPI to check that it has the latest asset from the source.
    QString GetDiskSource(const QString &assetRef);

    /// Returns an absolute path to a disk source of the url.
    /// @param QUrl url of the asset ref
    /// @return QString absolute path to the assets disk source. Return empty string if asset is not in the cache.
    /// @note this will return you the disk source for http/http assets unlike the QString overload.
    QString GetDiskSource(const QUrl &assetUrl);

    /// Checks whether the asset cache contains an asset with the given content hash, and returns the absolute path name to it, if so.
    /// Otherwise returns an empty string.
    /// @todo Implement.
    QString GetDiskSourceByContentHash(const QString &contentHash);

    /// Get the cache directory. Returned path is guaranteed to have a trailing slash /.
    /// @return QString absolute path to the caches data directory
    QString GetCacheDirectory() const;

    /// Saves the given asset to cache.
    /// @return QString the absolute path name to the asset cache entry. If not successfull returns an empty string.
    QString StoreAsset(AssetPtr asset);

    /// Saves the specified data to the asset cache.
    /// @return QString the absolute path name to the asset cache entry. If not successfull returns an empty string.
    QString StoreAsset(const u8 *data, size_t numBytes, const QString &assetName, const QString &assetContentHash);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QString asset reference.
    void DeleteAsset(const QString &assetRef);

    /// Deletes the asset with the given assetRef from the cache, if it exists.
    /// @param QUrl asset reference url.
    void DeleteAsset(const QUrl &assetUrl);

    /// Deletes all assets/metada in the data and metadata cache directories.
    void ClearAssetCache();

private slots:
    /// Writes metadata into a file. Helper function for the QNetworkDiskCache overrides.
    bool WriteMetadata(const QString &filePath, const QNetworkCacheMetaData &metaData);

    /// Genrates the absolute path to an asset cache entry. Helper function for the QNetworkDiskCache overrides.
    QString GetAbsoluteFilePath(bool isMetaData, const QUrl &url);

    /// Genrates the absolute path to an data asset cache entry.
    QString GetAbsoluteDataFilePath(const QString &filename);

    /// Removes all files from a directory.
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

    QHash<QUrl, QFile*> preparedItems;
};

#endif
