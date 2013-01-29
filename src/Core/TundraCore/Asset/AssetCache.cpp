// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetCache.h"
#include "AssetAPI.h"
#include "IAsset.h"

#include "CoreDefines.h"
#include "Framework.h"
#include "LoggingFunctions.h"

#include <QDateTime>
#include <QUrl>
#include <QFile>
#include <QDataStream>
#include <QFileInfo>
#include <QScopedPointer>

#ifdef Q_WS_WIN
#include "Win.h"
#else
#include <sys/stat.h>
#include <utime.h>
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

    // Check that the needed subfolders exist
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
    QString absolutePath = GetDiskSourceByRef(assetRef);
    if (QFile::exists(absolutePath))
        return absolutePath;
    else // The file is not in cache, return an empty string to denote that.
        return "";
}

QString AssetCache::GetDiskSourceByRef(const QString &assetRef)
{
    // Return the path where the given asset ref would be stored, if it was saved in the cache
    // (regardless of whether it now exists in the cache).
    return assetDataDir.absolutePath() + "/" + AssetAPI::SanitateAssetRef(assetRef);
}

QString AssetCache::CacheDirectory() const
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
    QString absolutePath = GetDiskSourceByRef(assetName);
    bool success = SaveAssetFromMemoryToFile(data, numBytes, absolutePath);
    if (success)
        return absolutePath;
    return "";
}

QDateTime AssetCache::LastModified(const QString &assetRef)
{
    QString absolutePath = FindInCache(assetRef);
    if (absolutePath.isEmpty())
        return QDateTime();

#ifdef Q_WS_WIN
    HANDLE fileHandle = (HANDLE)OpenFileHandle(absolutePath);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("AssetCache: Failed to open cache file to read last modified time: " + assetRef);
        return QDateTime();
    }

    // Get last write time.
    FILETIME fileTime;
    BOOL success = GetFileTime(fileHandle, 0, 0, &fileTime); // http://msdn.microsoft.com/en-us/library/windows/desktop/ms724320(v=VS.85).aspx
    CloseHandle(fileHandle);
    if (!success)
    {
        LogError("AssetCache: Failed to read cache file last modified time: " + assetRef);
        return QDateTime();
    }

    // Convert to UTC.
    SYSTEMTIME sysTime;
    if (!FileTimeToSystemTime(&fileTime, &sysTime)) // http://msdn.microsoft.com/en-us/library/windows/desktop/ms724280(v=VS.85).aspx
    {
        LogError("Win32 FileTimeToSystemTime failed for asset ref " + assetRef);
        return QDateTime();
    }

    // Ignore msec
    QDateTime dateTime;
    dateTime.setTimeSpec(Qt::UTC);
    dateTime.setDate(QDate((int)sysTime.wYear, (int)sysTime.wMonth, (int)sysTime.wDay));
    dateTime.setTime(QTime((int)sysTime.wHour, (int)sysTime.wMinute, (int)sysTime.wSecond, 0));
    return dateTime;
#else
    QDateTime dateTime;
    QString nativePath = QDir::toNativeSeparators(absolutePath);
    struct stat fileStats;
    if (stat(nativePath.toStdString().c_str(), &fileStats) == 0)
    {
        qint64 msecFromEpoch = (qint64)fileStats.st_mtime * 1000;
        dateTime.setMSecsSinceEpoch(msecFromEpoch);
    }
    else
        LogError("AssetCache: Failed to read cache file last modified time: " + assetRef);
    return dateTime;
#endif
}

bool AssetCache::SetLastModified(const QString &assetRef, const QDateTime &dateTime)
{
    if (!dateTime.isValid())
    {
        LogError("SetLastModified() DateTime is invalid: " + assetRef);
        return false;
    }

    QString absolutePath = FindInCache(assetRef);
    if (absolutePath.isEmpty())
        return false;

    QDate date = dateTime.date();
    QTime time = dateTime.time();

#ifdef Q_WS_WIN
    HANDLE fileHandle = (HANDLE)OpenFileHandle(absolutePath);
    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        LogError("AssetCache: Failed to open cache file to update last modified time: " + assetRef);
        return false;
    }

    // Notes: For SYSTEMTIME Sunday is 0 and ignore msec.
    SYSTEMTIME sysTime;
    sysTime.wDay = (WORD)date.day();
    sysTime.wDayOfWeek = (WORD)date.dayOfWeek();
    if (sysTime.wDayOfWeek == 7)
        sysTime.wDayOfWeek = 0;
    sysTime.wMonth = (WORD)date.month();
    sysTime.wYear = (WORD)date.year();
    sysTime.wHour = (WORD)time.hour();
    sysTime.wMinute = (WORD)time.minute();
    sysTime.wSecond = (WORD)time.second();
    sysTime.wMilliseconds = 0; 

    // Set last write time
    FILETIME fileTime;
    BOOL success = SystemTimeToFileTime(&sysTime, &fileTime);
    if (success)
        success = SetFileTime(fileHandle, 0, 0, &fileTime);
    CloseHandle(fileHandle);
    if (!success)
    {
        LogError("AssetCache: Failed to update cache file last modified time: " + assetRef);
        return false;
    }
    return true;
#else
    QString nativePath = QDir::toNativeSeparators(absolutePath);
    utimbuf modTime;
    modTime.actime = (time_t)(dateTime.toMSecsSinceEpoch() / 1000);
    modTime.modtime = (time_t)(dateTime.toMSecsSinceEpoch() / 1000);
    if (utime(nativePath.toStdString().c_str(), &modTime) == -1)
    {
        LogError("AssetCache: Failed to read cache file last modified time: " + assetRef);
        return false;
    }
    else
        return true;
#endif
}

#ifdef Q_WS_WIN
void *AssetCache::OpenFileHandle(const QString &absolutePath)
{
    QString nativePath = QDir::toNativeSeparators(absolutePath);
    QByteArray fileBA = nativePath.toLocal8Bit();
    if (fileBA.size() < MAX_PATH)
    {
        WCHAR szFilePath[MAX_PATH];
        MultiByteToWideChar(CP_ACP, 0, fileBA.data(), -1, szFilePath, NUMELEMS(szFilePath));
        return CreateFile(szFilePath, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
    else
    {
        // Prefixing file path with \\?\ is windows magic, see more from
        // http://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#maxpath
        // This allows use to open >MAX_PATH length file paths which do happen in asset cache if the 
        // source url is long! Assume 260*3 will be enough as it still very rarely goes over 260.
        fileBA = "\\\\?\\" + fileBA;
        WCHAR szFilePath[MAX_PATH*3];
        MultiByteToWideChar(CP_ACP, 0, fileBA.data(), -1, szFilePath, NUMELEMS(szFilePath));
        return CreateFileW(szFilePath, GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    }
}
#endif

void AssetCache::DeleteAsset(const QString &assetRef)
{
    QString absolutePath = GetDiskSourceByRef(assetRef);
    if (QFile::exists(absolutePath))
        QFile::remove(absolutePath);
}

void AssetCache::ClearAssetCache()
{
    if (!assetDataDir.exists())
        return;
    QFileInfoList entries = assetDataDir.entryInfoList(QDir::Files|QDir::NoSymLinks|QDir::NoDotAndDotDot);
    foreach(QFileInfo entry, entries)
    {
        if (entry.isFile())
        {
            if (!assetDataDir.remove(entry.fileName()))
                LogWarning("AssetCache::ClearAssetCache could not remove file " + entry.absoluteFilePath());
        }
    }
}
