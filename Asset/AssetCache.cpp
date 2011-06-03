#include "DebugOperatorNew.h"
#include <boost/algorithm/string.hpp>
#include <QList>

#include "AssetCache.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "LoggingFunctions.h"

#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QCryptographicHash>
#include <QScopedPointer>

#include "MemoryLeakCheck.h"

DEFINE_POCO_LOGGING_FUNCTIONS("AssetCache")

QString SanitateAssetRefForCache(QString assetRef)
{ 
    assetRef.replace("/", "_");
    assetRef.replace("\\", "_");
    assetRef.replace(":", "_");
    assetRef.replace("*", "_");
    assetRef.replace("?", "_");
    assetRef.replace("\"", "_");
    assetRef.replace("'", "_");
    assetRef.replace("<", "_");
    assetRef.replace(">", "_");
    assetRef.replace("|", "_");
    return assetRef;
}

// AssetCache

AssetCache::AssetCache(AssetAPI *owner, QString assetCacheDirectory) : 
    QNetworkDiskCache(owner),
    assetAPI(owner),
    cacheDirectory(GuaranteeTrailingSlash(assetCacheDirectory))
{
    LogInfo("Using AssetCache in directory '" + assetCacheDirectory.toStdString() + "'");

    // Check that the main directory exists
    QDir assetDir(cacheDirectory);
    if (!assetDir.exists())
    {
        QString dirName = cacheDirectory.split("/", QString::SkipEmptyParts).last();
        QString parentPath = cacheDirectory;
        parentPath.chop(dirName.length()+1);
        QDir parentDir(parentPath);
        parentDir.mkdir(dirName);
    }

    // Check that the needed subfolders exist
    if (!assetDir.exists("data"))
        assetDir.mkdir("data");
    assetDataDir = QDir(cacheDirectory + "data");
    if (!assetDir.exists("metadata"))
        assetDir.mkdir("metadata");
    assetMetaDataDir = QDir(cacheDirectory + "metadata");

    // Set for QNetworkDiskCache
    setCacheDirectory(cacheDirectory);
}

QIODevice* AssetCache::data(const QUrl &url)
{
    QScopedPointer<QFile> dataFile;
    QString absoluteDataFile = GetAbsoluteFilePath(false, url);
    if (QFile::exists(absoluteDataFile))
    {
        dataFile.reset(new QFile(absoluteDataFile));
        if (!dataFile->open(QIODevice::ReadWrite))
        {
            dataFile.reset();
            return 0;
        }
    }
    // It is the callers responsibility to delete this ptr as said by the Qt docs.
    // This will most likely happen when QNetworkReply->deleteLater() is called, meaning next qt mainloop cycle from that call.
    return dataFile.take();
}

void AssetCache::insert(QIODevice* device)
{
    // We own this ptr from prepare()
    QHashIterator<QString, QFile*> it(preparedItems);
    while (it.hasNext())
    {
        it.next();
        if (it.value() == device)
        {
            preparedItems.remove(it.key());
            break;
        }
    }
    // Delete later, meaning next qt mainloop cycle, because the asset will 
    // use this ptr to deserialize the content to and IAsset after this call return.
    device->close();
    device->deleteLater();
}

QIODevice* AssetCache::prepare(const QNetworkCacheMetaData &metaData)
{
    if (!WriteMetadata(GetAbsoluteFilePath(true, metaData.url()), metaData))
        return 0;
    QScopedPointer<QFile> dataFile(new QFile(GetAbsoluteFilePath(false, metaData.url())));
    if (!dataFile->open(QIODevice::ReadWrite))
    {
        LogError("Failed not open data file QIODevice::ReadWrite mode for " + metaData.url().toString().toStdString());
        dataFile.reset();
        remove(metaData.url());
        return 0;
    }
    if (dataFile->bytesAvailable() > 0)
    {
        if (!dataFile->resize(0))
        {
            LogError("Failed not reset existing data from cache entry. Skipping cache store for " + metaData.url().toString().toStdString());
            dataFile->close();
            dataFile.reset();
            remove(metaData.url());
            return 0;
        }
    }
    // Take ownership of the ptr
    QFile *dataPtr = dataFile.take();
    preparedItems[metaData.url().toString()] = dataPtr;
    return dataPtr;
}

bool AssetCache::remove(const QUrl &url)
{
    // remove() is also used for canceling insertion after prepare()
    // we need to delete the QFile* ptr also in these cases
    // note: this is not a common operation
    QHashIterator<QString, QFile*> it(preparedItems);
    while (it.hasNext())
    {
        it.next();
        if (it.key() == url.toString())
        {
            delete it.value();
            preparedItems.remove(it.key());
            break;
        }
    }

    // Remove the actual files related to this url
    bool success = true;
    QString absoluteMetaDataFile = GetAbsoluteFilePath(true,url);
    if (QFile::exists(absoluteMetaDataFile))
        success = QFile::remove(absoluteMetaDataFile);
    if (!success)
        return false;
    QString absoluteDataFile = GetAbsoluteFilePath(false, url);
    if (QFile::exists(absoluteDataFile))
        success = QFile::remove(absoluteDataFile);
    return success;
}

QNetworkCacheMetaData AssetCache::metaData(const QUrl &url)
{
    QNetworkCacheMetaData resultMetaData;
    QString absoluteMetaDataFile = GetAbsoluteFilePath(true, url);
    if (QFile::exists(absoluteMetaDataFile))
    {
        QFile metaDataFile(absoluteMetaDataFile);
        if (metaDataFile.open(QIODevice::ReadOnly))
        {
            QDataStream metaDataStream(&metaDataFile);
            metaDataStream >> resultMetaData;
            metaDataFile.close();
        }
    }

    // If we have a cache data file also, verify data integrity.
    // If the file is corrupted, return empty metadata to trigger a new full fetch for data.
    if (resultMetaData.isValid())
    {
        QString absoluteDataFile = GetAbsoluteFilePath(false, url);
        if (QFile::exists(absoluteDataFile))
        {
            if (!VerifyCacheContentDigest(absoluteDataFile, resultMetaData))
            {
                LogError("Detected a corrupted cache file, triggering a full fetch for " + url.toString());
                return QNetworkCacheMetaData();
            }
        }
    }

    return resultMetaData;
}

void AssetCache::updateMetaData(const QNetworkCacheMetaData &metaData)
{
    const QNetworkCacheMetaData oldMetaData = this->metaData(metaData.url());
    if (oldMetaData.isValid())
        if (oldMetaData != metaData)
            WriteMetadata(GetAbsoluteFilePath(true, metaData.url()), metaData);
}

void AssetCache::clear()
{
    ClearAssetCache();
}

qint64 AssetCache::expire()
{
    // Skip keeping cache at some static size, unlimited for now.
    return maximumCacheSize() / 2;
}

QString AssetCache::GetDiskSource(const QString &assetRef)
{
    // Deny http:// and https:// asset references to be gotten from cache
    // as the QAccessManager will request it from the overrides above later!
    // You can get the path if you ask directly as a url.
    if (assetRef.startsWith("http://") || assetRef.startsWith("https://"))
        return "";
    return GetDiskSource(QUrl(assetRef, QUrl::TolerantMode));
}

QString AssetCache::GetDiskSource(const QUrl &assetUrl)
{
    QString absolutePath = assetDataDir.absolutePath() + "/" + SanitateAssetRefForCache(assetUrl.toString());
    if (QFile::exists(absolutePath))
        return absolutePath;
    return "";
}

QString AssetCache::GetDiskSourceByContentHash(const QString &contentHash)
{
    /// \todo Implement.
    LogWarning("AssetCache::GetDiskSourceByContentHash not implemented yet.");
    return "";
}

QString AssetCache::GetCacheDirectory() const
{
    return GuaranteeTrailingSlash(assetDataDir.absolutePath());
}

QString AssetCache::StoreAsset(AssetPtr asset)
{
    std::vector<u8> data;
    asset->SerializeTo(data);
    return StoreAsset(&data[0], data.size(), asset->Name(), asset->ContentHash()); /// \todo Content hash can mismatch here.
}

QString AssetCache::StoreAsset(const u8 *data, size_t numBytes, const QString &assetName, const QString &assetContentHash)
{
    QString absolutePath = GetAbsoluteDataFilePath(assetName);
    bool success = SaveAssetFromMemoryToFile(data, numBytes, absolutePath.toStdString().c_str());
    if (success)
        return absolutePath;
    return "";
}

void AssetCache::DeleteAsset(const QString &assetRef)
{
    DeleteAsset(QUrl(assetRef, QUrl::TolerantMode));
}

void AssetCache::DeleteAsset(const QUrl &assetUrl)
{
    if (!remove(assetUrl))
        LogWarning("AssetCache::DeleteAsset Failed to delete asset " + assetUrl.toString().toStdString());
}

void AssetCache::ClearAssetCache()
{
    ClearDirectory(assetDataDir.absolutePath());
    ClearDirectory(assetMetaDataDir.absolutePath());
}

bool AssetCache::WriteMetadata(const QString &filePath, const QNetworkCacheMetaData &metaData)
{
    QFile metaDataFile(filePath);
    if (!metaDataFile.open(QIODevice::WriteOnly))
    {
        LogError("AssetCache::WriteMetadata Could not open metadata file: " + filePath.toStdString());
        return false;
    }
    if (!metaDataFile.resize(0))
    {
        LogError("AssetCache::WriteMetadata Could not reset metadata file: " + filePath.toStdString());
        return false;
    }

    QDataStream metaDataStream(&metaDataFile);
    metaDataStream << metaData;
    metaDataFile.close();
    return true;
}

bool AssetCache::VerifyCacheContentDigest(const QString &absoluteDataFilePath, const QNetworkCacheMetaData &metaData)
{
    // If we are not processing a web asset, the content digest wont be there
    // if we haven't inserted it there ourselves (we should when providers are implemented).
    // so return true and allow the cache item to be passed onward.
    QString dataUrlScheme = metaData.url().scheme().toLower();
    if (dataUrlScheme != "http" && dataUrlScheme != "https" && dataUrlScheme != "ftp")
        return true;

    QNetworkCacheMetaData::RawHeaderList headers = metaData.rawHeaders();
    
    QByteArray cachedContentDigest;
    QByteArray contentDigestHeader("content-md5");
    foreach(QNetworkCacheMetaData::RawHeader header, headers)
    {
        if (header.first.toLower() == contentDigestHeader)
        {
            cachedContentDigest = header.second;
            break;
        }
    }
    
    // All server do not serve the "Content-MD5" header,
    // if not we skip this check and allow the cache item to be passed onward.
    if (cachedContentDigest.isEmpty())
        return true;

    // We have the header present, now its time to compare the data
    // If we fail to open the file, remove it and return false to trigger a full fetch.
    QFile dataFile(absoluteDataFilePath);
    if (!dataFile.open(QIODevice::ReadOnly))
    {
        if (!remove(metaData.url()))
            LogError("Failed to remove a corrupted data or metadata file.");
        return false;
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    hash.addData(dataFile.readAll());
    dataFile.close();

    QByteArray dataContentDigest = hash.result().toBase64();
    if (dataContentDigest != cachedContentDigest)
    {
        if (!remove(metaData.url()))
            LogError("Failed to remove a corrupted data or metadata file.");
        return false;
    }
    return true;
}

QString AssetCache::GetAbsoluteFilePath(bool isMetaData, const QUrl &url)
{
    QString subDir = isMetaData ? "metadata" : "data";
    QDir assetDir(cacheDirectory + subDir);
    QString absolutePath = assetDir.absolutePath() + "/" + SanitateAssetRefForCache(url.toString());
    if (isMetaData)
        absolutePath.append(".metadata");
    return absolutePath;
}

QString AssetCache::GetAbsoluteDataFilePath(const QString &filename)
{
    return assetDataDir.absolutePath() + "/" + SanitateAssetRefForCache(filename);
}

void AssetCache::ClearDirectory(const QString &absoluteDirPath)
{
    QDir targetDir(absoluteDirPath);
    if (!targetDir.exists())
    {
        LogWarning("AssetCache::ClearDirectory called with non existing directory path.");
        return;
    }
    QFileInfoList entries = targetDir.entryInfoList(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot);
    foreach(QFileInfo entry, entries)
    {
        if (entry.isFile())
        {
            if (!targetDir.remove(entry.fileName()))
                LogWarning("AssetCache::ClearDirectory could not remove file " + entry.absoluteFilePath().toStdString());
        }
    }
}

CookieJar *AssetCache::NewCookieJar(const QString &cookieDiskFile)
{
    return new CookieJar(this, cookieDiskFile);
}

// CookieJar

CookieJar::CookieJar(QObject *parent, const QString &cookieDiskFile) :
    QNetworkCookieJar(parent),
    cookieDiskFile_(cookieDiskFile)
{
    ReadCookies();
}

CookieJar::~CookieJar()
{
    StoreCookies();
}

void CookieJar::SetDataFile(const QString &cookieDiskFile)
{
    cookieDiskFile_ = cookieDiskFile;
}

void CookieJar::ClearCookies()
{
    if (!cookieDiskFile_.isEmpty())
    {
        QFile cookiesFile(cookieDiskFile_);
        if (cookiesFile.exists())
            cookiesFile.remove();
    }
    setAllCookies(QList<QNetworkCookie>());
}

void CookieJar::ReadCookies()
{
    if (cookieDiskFile_.isEmpty())
        return;

    QFile cookiesFile(cookieDiskFile_);
    if (!cookiesFile.open(QIODevice::ReadOnly))
        return;
    
    QList<QNetworkCookie> cookies;
    QDataStream cookieData(&cookiesFile);
    while (!cookieData.atEnd()) 
    {
        QByteArray rawCookie;
        cookieData >> rawCookie;
        cookies.append(QNetworkCookie::parseCookies(rawCookie));
    }
    cookiesFile.close();
    setAllCookies(cookies);
}

void CookieJar::StoreCookies()
{
    if (cookieDiskFile_.isEmpty())
        return;
    if (allCookies().empty())
        return;

    QFile cookiesFile(cookieDiskFile_);
    if (!cookiesFile.open(QIODevice::WriteOnly))
        return;

    QDataStream cookieData(&cookiesFile);
    foreach (QNetworkCookie cookie, allCookies())
        cookieData << cookie.toRawForm();
    cookiesFile.close();
}
