// For conditions of distribution and use, see copyright notice in LICENSE

#include "ZipAssetBundle.h"
#include "ZipHelpers.h"

#include "CoreDefines.h"
#include "AssetCache.h"
#include "LoggingFunctions.h"

#include "zzip/zzip.h"
#include <QDir>
#include <QDateTime>
#include <QThreadPool>

ZipAssetBundle::ZipAssetBundle(AssetAPI *owner, const QString &type, const QString &name) :
    IAssetBundle(owner, type, name),
    archive_(0)
{
}

ZipAssetBundle::~ZipAssetBundle()
{
    Unload();
}

void ZipAssetBundle::DoUnload()
{
    Close();
}

bool ZipAssetBundle::DeserializeFromDiskSource()
{
    if (DiskSource().isEmpty())
    {
        LogError("ZipAssetBundle::DeserializeFromDiskSource: Cannot process archive, no disk source for " + Name());
        return false;
    }

    /* We want to detect if the extracted files are already up to date to save time.
       If the last modified date for the sub asset is the same as the parent zip file, 
       we don't extract it. If the zip is re-downloaded from source everything will get unpacked even
       if only one file would have changed inside it. We could do uncompressed size comparisons
       but that is not a absolute guarantee that the file has not changed. We'll be on the safe side
       to unpack the whole zip file. Zip files are meant for deploying the scene and should be touched
       rather rarely. Note that local:// refs are unpacked to cache but the zips disk source is not in the
       cache. Meaning that local:// zip files will always be extracted fully even if the disk source
       was not changed, we don't have a mechanism to get the last modified date properly except from
       the asset cache. For local scenes this should be fine as there is no real need to
       zip the scene up as you already have the disk sources right there in the storage.
       The last modified query will fail if the file is open with zziplib, do it first. */
    QDateTime zipLastModified = assetAPI_->GetAssetCache()->LastModified(Name());
        
    zzip_error_t error = ZZIP_NO_ERROR;
    archive_ = zzip_dir_open(QDir::toNativeSeparators(DiskSource()).toStdString().c_str(), &error);
    if (CheckAndLogZzipError(error) || CheckAndLogArchiveError(archive_) || !archive_)
    {
        archive_ = 0;
        return false;
    }
    
    int uncompressing = 0;
    
    ZZIP_DIRENT archiveEntry;
    while(zzip_dir_read(archive_, &archiveEntry))
    {
        QString relativePath = QDir::fromNativeSeparators(archiveEntry.d_name);
        if (!relativePath.endsWith("/"))
        {
            QString subAssetRef = GetFullAssetReference(relativePath);
            
            ZipArchiveFile file;
            file.relativePath = relativePath;
            file.cachePath = assetAPI_->GetAssetCache()->GetDiskSourceByRef(subAssetRef);
            file.lastModified = assetAPI_->GetAssetCache()->LastModified(subAssetRef);
            file.compressedSize = archiveEntry.d_csize;
            file.uncompressedSize = archiveEntry.st_size;
            
            /* Mark this file for extraction. If both cache files have valid dates
               and they differ extract. If they have the same date stamp skip extraction.
               Note that file.lastModified will be non-valid for non cached files so we 
               will cover also missing files. */
            file.doExtract = (zipLastModified.isValid() && file.lastModified.isValid()) ? (zipLastModified != file.lastModified) : true;
            if (file.doExtract)
                uncompressing++;

            files_ << file;
        }
    }
    
    // Close the zzip directory ptr
    Close();
    
    // If the zip file was empty we don't want IsLoaded to fail on the files_ check.
    // The bundle loaded fine but there was no content, log a warning.
    if (files_.isEmpty())
    {
        LogWarning("ZipAssetBundle: Bundle loaded but does not contain any files " + Name());
        files_ << ZipArchiveFile();
        emit Loaded(this);
        return true;
    }
    
    // Don't spin the worker if all sub assets are up to date in cache.
    if (uncompressing > 0)
    {   
        // Now that the file info has been read, continue in a worker thread.
        LogDebug("ZipAssetBundle: File information read for " + Name() + ". File count: " + QString::number(files_.size()) + ". Starting worker thread to uncompress " + QString::number(uncompressing) + " files.");
        
        // ZipWorker is a QRunnable we can pass to QThreadPool, it will handle scheduling it and deletes it when done.
        ZipWorker *worker = new ZipWorker(DiskSource(), files_);   
        connect(worker, SIGNAL(AsynchLoadCompleted(bool)), this, SLOT(OnAsynchLoadCompleted(bool)), Qt::QueuedConnection);
        QThreadPool::globalInstance()->start(worker);
    }
    else
        emit Loaded(this);
        
    return true;
}

bool ZipAssetBundle::DeserializeFromData(const u8 * /*data*/, size_t /*numBytes*/)
{
    /** @note At this point it seems zzip needs a disk source to do processing
        so we require disk source for the archive. This might change in the future by changing the lib. */
    return false;
}

std::vector<u8> ZipAssetBundle::GetSubAssetData(const QString &subAssetName)
{
    /* Makes no sense to keep the whole zip file contents in memory as only
       few files could be wanted from a 100mb bundle. Additionally all asset would take 2x the memory.
       We could make this function also open the zip file and uncompress the data for every sub asset request. 
       But that would be rather pointless, not to mention slower, as we already have the unpacked individual 
       assets on disk. If the unpacking to disk changes we might need to rethink this. */

    QString filePath = GetSubAssetDiskSource(subAssetName);
    if (filePath.isEmpty())
        return std::vector<u8>();

    std::vector<u8> data;
    return LoadFileToVector(filePath, data) ? data : std::vector<u8>();
}

QString ZipAssetBundle::GetSubAssetDiskSource(const QString &subAssetName)
{
    return assetAPI_->GetAssetCache()->FindInCache(GetFullAssetReference(subAssetName));
}

QString ZipAssetBundle::GetFullAssetReference(const QString &subAssetName)
{
    return Name() + "#" + subAssetName;
}

bool ZipAssetBundle::IsLoaded() const
{
    return (archive_ != 0 || !files_.isEmpty());
}

void ZipAssetBundle::OnAsynchLoadCompleted(bool successful)
{   
    // Write new timestamps for extracted files. Cannot be done (?!) in the worker
    // thread as it would need to access Framework, AssetAPI and AssetCache ptrs 
    // and they might not be safe to access from outside the main thread.
    QDateTime zipLastModified = assetAPI_->GetAssetCache()->LastModified(Name());
    if (zipLastModified.isValid())
    {
        foreach(ZipArchiveFile file, files_)
            if (file.doExtract)
                assetAPI_->GetAssetCache()->SetLastModified(GetFullAssetReference(file.relativePath), zipLastModified);
    }
    
    LogDebug("ZipAssetBundle: Zip file extracted " + Name());
    
    if (successful)
        emit Loaded(this);
    else
        emit Failed(this);
}

void ZipAssetBundle::Close()
{
    if (archive_)
    {
        zzip_dir_close(archive_);
        archive_ = 0;
    }
}
