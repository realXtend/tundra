#include "DebugOperatorNew.h"
#include <boost/algorithm/string.hpp>
#include <QList>

#include "AssetCache.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#include <QUrl>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QScopedPointer>

#ifdef Q_WS_WIN
#include "Windows.h"
#else
/// @todo implement linux/mac support eg. http://pubs.opengroup.org/onlinepubs/009604599/functions/utime.html
#endif

#include "MemoryLeakCheck.h"

AssetCache::AssetCache(AssetAPI *owner, QString assetCacheDirectory) : 
    assetAPI(owner),
    cacheDirectory(GuaranteeTrailingSlash(QDir::fromNativeSeparators(assetCacheDirectory)))
{
    LogInfo("* Asset cache directory: " + cacheDirectory);  

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

    // Check that the needed sub folders exist
    if (!assetDir.exists("data"))
        assetDir.mkdir("data");
    assetDataDir = QDir(cacheDirectory + "data");

    // Check --clear-asset-cache start param
    if (owner->GetFramework()->HasCommandLineParameter("--clear-asset-cache"))
    {
        LogInfo("AssetCache: Removing all data and metadata files from cache, found 'clear-asset-cache' from start params!");
        ClearAssetCache();
    }
}

QString AssetCache::FindInCache(const QString &assetRef)
{
    /// @note This is deprecated since ~2.1.4. Remove this functions when scripts etc. 
    /// 3rd party code have migrated to using GetDiskSourceByRef().
    LogWarning("AssetCache::FileInCache is deprecated and is up for removal, use AssetCache::GetDiskSourceByRef instead!");
    return GetDiskSourceByRef(assetRef);
}

QString AssetCache::GetDiskSourceByRef(const QString &assetRef)
{
    QString absolutePath = assetDataDir.absolutePath() + "/" + AssetAPI::SanitateAssetRef(assetRef);
    if (QFile::exists(absolutePath))
        return absolutePath;
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
    return StoreAsset(&data[0], data.size(), asset->Name());
}

QString AssetCache::StoreAsset(const u8 *data, size_t numBytes, const QString &assetName)
{
    QString absolutePath = GetAbsoluteDataFilePath(assetName);
    bool success = SaveAssetFromMemoryToFile(data, numBytes, absolutePath.toStdString().c_str());
    if (success)
        return absolutePath;
    return "";
}

QDateTime AssetCache::LastModified(const QString &assetRef)
{
    QDateTime dateTime;
    QString absolutePath = GetDiskSourceByRef(assetRef);
    if (absolutePath.isEmpty())
        return dateTime;

#ifdef Q_WS_WIN
    HANDLE fileHandle = (HANDLE)OpenFileHandle(absolutePath);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("AssetCache: Failed to open cache file to read last modified time: " + assetRef);
        return dateTime;
    }

    bool success = false;
    FILETIME *fileTime = new FILETIME;
    SYSTEMTIME  *sysTime = new SYSTEMTIME;

    // Get last write time
    if (GetFileTime(fileHandle, 0, 0, fileTime)) 
    {
        if (FileTimeToSystemTime(fileTime, sysTime))
        {
            // Ignore msec
            dateTime.setDate(QDate((int)sysTime->wYear, (int)sysTime->wMonth, (int)sysTime->wDay));
            dateTime.setTime(QTime((int)sysTime->wHour, (int)sysTime->wMinute, (int)sysTime->wSecond, 0)); 
            success = true;
        }
    }
    if (!success)
        LogError("AssetCache: Failed to read cache file last modified time: " + assetRef);

    if (!CloseHandle(fileHandle))
        LogError("AssetCache: Failed to close cache file after reading last modified time: " + assetRef);
    SAFE_DELETE(sysTime);
    SAFE_DELETE(fileTime);
#else
    /// @todo implement linux/mac support
    LogWarning("AssetCache::LastModified() not implemented for linux/mac!");
#endif

    return dateTime;
}

bool AssetCache::SetLastModified(const QString &assetRef, const QDateTime &dateTime)
{
    bool success = false;

    if (!dateTime.isValid())
    {
        LogError("SetLastModified() DateTime is invalid: " + assetRef);
        return success;
    }

    QString absolutePath = GetDiskSourceByRef(assetRef);
    if (absolutePath.isEmpty())
        return success;

    QDate date = dateTime.date();
    QTime time = dateTime.time();

#ifdef Q_WS_WIN
    HANDLE fileHandle = (HANDLE)OpenFileHandle(absolutePath);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("AssetCache: Failed to open cache file to update last modified time: " + assetRef);
        return success;
    }

    // Notes: For SYSTEMTIME Sunday is 0 and ignore msec.
    SYSTEMTIME  *sysTime = new SYSTEMTIME;
    sysTime->wDay = (WORD)date.day();
    sysTime->wDayOfWeek = (WORD)date.dayOfWeek();
    if (sysTime->wDayOfWeek == 7)
        sysTime->wDayOfWeek = 0;
    sysTime->wMonth = (WORD)date.month();
    sysTime->wYear = (WORD)date.year();
    sysTime->wHour = (WORD)time.hour();
    sysTime->wMinute = (WORD)time.minute();
    sysTime->wSecond = (WORD)time.second();
    sysTime->wMilliseconds = 0; 

    // Set last write time
    FILETIME *fileTime = new FILETIME;
    if (SystemTimeToFileTime(sysTime, fileTime))
        success = SetFileTime(fileHandle, 0, 0, fileTime); 
    if (!success)
        LogError("AssetCache: Failed to update cache file last modified time: " + assetRef);

    if (!CloseHandle(fileHandle))
        LogError("AssetCache: Failed to close cache file after updating last modified time: " + assetRef);
    SAFE_DELETE(sysTime);
    SAFE_DELETE(fileTime);
#else
    /// @todo implement linux/mac support
    LogWarning("AssetCache::SetLastModified() not implemented for linux/mac!");
#endif

    return success;
}

#ifdef Q_WS_WIN
void *AssetCache::OpenFileHandle(const QString &absolutePath)
{
    QString nativePath = QDir::toNativeSeparators(absolutePath);
    QByteArray fileBA = nativePath.toLocal8Bit();
    WCHAR szFilePath[MAX_PATH];
    MultiByteToWideChar(CP_ACP, 0, fileBA.data(), -1, szFilePath, NUMELEMS(szFilePath));
    return CreateFile(szFilePath, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
}
#endif

void AssetCache::DeleteAsset(const QString &assetRef)
{
    DeleteAsset(QUrl(assetRef, QUrl::TolerantMode));
}

void AssetCache::DeleteAsset(const QUrl &assetUrl)
{

}

void AssetCache::ClearAssetCache()
{
    ClearDirectory(assetDataDir.absolutePath());
}

QString AssetCache::GetAbsoluteFilePath(bool isMetaData, const QUrl &url)
{
    QString subDir = isMetaData ? "metadata" : "data";
    QDir assetDir(cacheDirectory + subDir);
    QString absolutePath = assetDir.absolutePath() + "/" + AssetAPI::SanitateAssetRef(url.toString());
    if (isMetaData)
        absolutePath.append(".metadata");
    return absolutePath;
}

QString AssetCache::GetAbsoluteDataFilePath(const QString &filename)
{
    return assetDataDir.absolutePath() + "/" + AssetAPI::SanitateAssetRef(filename);
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
                LogWarning("AssetCache::ClearDirectory could not remove file " + entry.absoluteFilePath());
        }
    }
}
