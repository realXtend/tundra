// For conditions of distribution and use, see copyright notice in LICENSE

#include "ZipWorker.h"
#include "ZipHelpers.h"
#include "CoreTypes.h"
#include "LoggingFunctions.h"

#include "zzip/zzip.h"

#include <QtAlgorithms>
#include <QDir>
#include <QFile>

bool ArchiveFileSizeCompare(const ZipArchiveFile &f1, const ZipArchiveFile &f2)
{
    return f1.uncompressedSize < f2.uncompressedSize;
}

ZipWorker::ZipWorker(const QString &diskSource, ZipFileList files) :
    diskSource_(diskSource),
    files_(files),
    archive_(0)
{
    // Make sure this worker object is deleted by QThreadPool once run() completes.
    setAutoDelete(true);
}

ZipWorker::~ZipWorker()
{
    Close();
}

void ZipWorker::run()
{
    zzip_error_t error = ZZIP_NO_ERROR;
    archive_ = zzip_dir_open(QDir::toNativeSeparators(diskSource_).toStdString().c_str(), &error);
    if (CheckAndLogZzipError(error) || CheckAndLogArchiveError(archive_) || !archive_)
    {
        archive_ = 0;
        emit AsynchLoadCompleted(false);
        return;
    }
    
    // Sort by size so we can resize the buffer less often.
    qSort(files_.begin(), files_.end(), ArchiveFileSizeCompare);

    // Read file contents
    zzip_ssize_t chunkLen = 0;
    zzip_ssize_t chunkRead = 0;
    std::vector<u8> buffer;

    foreach(ZipArchiveFile file, files_)
    {
        if (!file.doExtract)
            continue;
        
        // Open file from zip
        ZZIP_FILE *zzipFile = zzip_file_open(archive_, file.relativePath.toStdString().c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS);
        if (zzipFile && !CheckAndLogArchiveError(archive_))
        {            
            // Create cache file
            QFile cacheFile(file.cachePath);
            if (!cacheFile.open(QIODevice::WriteOnly))
            {
                LogError("ZipWorker: Failed to open cache file: " + cacheFile.fileName() + ". Cannot unzip " + file.relativePath);
                continue;
            }

            // Detect file size and adjust buffer (quite naive atm but is a slight speed improvement)
            if (file.uncompressedSize > 1000*1024)
                chunkLen = 500*1024;
            else if (file.uncompressedSize > 500*1024)
                chunkLen = 250*1024;
            else if (file.uncompressedSize > 100*1024)
                chunkLen = 50*1024;
            else if (file.uncompressedSize > 20*1024)
                chunkLen = 10*1024;
            else
                chunkLen = 5*1024;

            if (buffer.size() != (uint)chunkLen)
                buffer.resize(chunkLen);

            // Read zip file content to cache file
            while (0 < (chunkRead = zzip_read(zzipFile, &buffer[0], chunkLen)))
                cacheFile.write((char*)&buffer[0], chunkRead);

            // Close zip and cache file.
            zzip_file_close(zzipFile);
            cacheFile.close();
        }
    }

    // Close the zzip directory ptr
    Close();
        
    emit AsynchLoadCompleted(true);
}

void ZipWorker::Close()
{
    if (archive_)
    {
        zzip_dir_close(archive_);
        archive_ = 0;
    }
}
