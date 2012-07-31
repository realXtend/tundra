// For conditions of distribution and use, see copyright notice in LICENSE

#include "ZipAssetBundle.h"
#include "ZipHelpers.h"

#include "CoreDefines.h"
#include "AssetCache.h"
#include "LoggingFunctions.h"

#include "zzip/zzip.h"
#include <QDir>

ZipAssetBundle::ZipAssetBundle(AssetAPI *owner, const QString &type, const QString &name) :
    IAssetBundle(owner, type, name),
    archive_(0),
    worker_(0)
{
}

ZipAssetBundle::~ZipAssetBundle()
{
    Unload();
}

void ZipAssetBundle::DoUnload()
{
    Close();
    CloseWorker();
}

bool ZipAssetBundle::DeserializeFromDiskSource()
{
    if (DiskSource().isEmpty())
    {
        LogError("ZipAssetBundle::DeserializeFromDiskSource: Cannot process archive, no disk source for " + Name());
        return false;
    }
    
    zzip_error_t error = ZZIP_NO_ERROR;
    archive_ = zzip_dir_open(QDir::fromNativeSeparators(DiskSource()).toStdString().c_str(), &error);
    if (CheckAndLogZzipError(error) || CheckAndLogArchiveError(archive_) || !archive_)
    {
        archive_ = 0;
        return false;
    }
    
    ZZIP_DIRENT archiveEntry;
    while(zzip_dir_read(archive_, &archiveEntry))
    {
        ZipArchiveFile file;
        file.relativePath = QDir::fromNativeSeparators(archiveEntry.d_name);
        if (!file.relativePath.endsWith("/"))
        {
            file.cachePath = assetAPI_->GetAssetCache()->GetDiskSourceByRef(GetFullAssetReference(file.relativePath));
            file.compressedSize = archiveEntry.d_csize;
            file.uncompressedSize = archiveEntry.st_size;
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
        
        // In this case there is no need to spin up the worker.
        emit Loaded(this);
        return true;
    }

    // Now that the file info has been read, continue in a worker thread.
    LogInfo("ZipAssetBundle: File information read for " + Name() + ". File count: " + QString::number(files_.size()) + ". Starting worker thread.");
    
    worker_ = new ZipWorker(DiskSource(), files_);
    worker_->moveToThread(worker_);
    
    connect(worker_, SIGNAL(AsynchLoadCompleted(bool)), this, SLOT(OnAsynchLoadCompleted(bool)), Qt::QueuedConnection);
    worker_->start(QThread::HighPriority);
    
    return true;
}

bool ZipAssetBundle::DeserializeFromData(const u8 *data, size_t numBytes)
{
    /** @note At this point it seems zzip needs a disk source to do processing
        so we require disk source for the archive. This might change in the future by changing the lib. */
    return false;
}

std::vector<u8> ZipAssetBundle::GetSubAssetData(const QString &subAssetName)
{
    /** Makes no sense to keep the whole zip file in cache at the moment.
        We cant however make this function also open the zip file and uncompress the data,
        but that is rather pointless, not to mention slower, as we already have 
        the unpacked individual assets on disk. If the unpacking to disk changes we might
        need to rethink this. */

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
    LogInfo("ZipAssetBundle: Zip file extracted " + Name());
    
    CloseWorker();
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

void ZipAssetBundle::CloseWorker()
{
    if (worker_ && worker_->isRunning())
    {
        worker_->exit();
        worker_->wait();
    }
    SAFE_DELETE(worker_);
}
