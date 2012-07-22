// For conditions of distribution and use, see copyright notice in LICENSE

#include "ZipAssetBundle.h"

#include "AssetCache.h"
#include "LoggingFunctions.h"
#include "CoreTypes.h"

#include "zzip/zzip.h"

#include <QFile>
#include <QtAlgorithms>
#include <QTime>
#include <QDebug>

bool ArchiveFileSizeCompare(const ArchiveFile &f1, const ArchiveFile &f2)
{
    return f1.uncompressedSize < f2.uncompressedSize;
}

ZipAssetBundle::ZipAssetBundle(AssetAPI *owner, const QString &type, const QString &name) :
    IAssetBundle(owner, type, name),
    cacheDir_(owner->GetAssetCache()->CacheDirectory()),
    archive_(0)
{
    // Check if the ref has a sub asset in it and remove it.
    AssetAPI::ParseAssetRef(name, 0, 0, 0, 0, 0, 0, 0, 0, 0, &baseArchiveRef_);
    if (baseArchiveRef_.isEmpty())
        baseArchiveRef_ = name;

    // Create working directory if does not exist
    QString cacheDirName = "archive_" + AssetAPI::SanitateAssetRef(baseArchiveRef_).toLower();
    if (!cacheDir_.exists(cacheDirName))
        if (!cacheDir_.mkdir(cacheDirName))
            LogError("ZipAssetBundle: Failed to create cache directory for uncompression, cannot continue: " + cacheDir_.absoluteFilePath(cacheDirName));
    if (!cacheDir_.cd(cacheDirName))
        cacheDir_ = QDir();
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
    /// @todo Thread this unzip procedure and emit Loaded in the main thread after!

    if (!cacheDir_.exists())
    {
        LogError("ZipAssetBundle::DeserializeFromData: Uncompression destination does not exits in asset cache, aborting.");
        return false;
    }

    QTime t;
    t.start();

    if (!DiskSource().isEmpty())
    {
        archive_ = zzip_dir_open(DiskSource().toStdString().c_str(), 0);
        if (archive_ && !CheckError())
        {
            QStringList subDirectories;
            ZZIP_DIRENT archiveEntry;
            while(zzip_dir_read(archive_, &archiveEntry))
            {
                ArchiveFile file;
                file.relativePath = QDir::fromNativeSeparators(archiveEntry.d_name);
                if (!file.relativePath.endsWith("/"))
                {
                    file.compressedSize = archiveEntry.d_csize;
                    file.uncompressedSize = archiveEntry.st_size;
                    files_ << file;
                }
                else
                    subDirectories << file.relativePath;
            }

            // Create cache sub directories
            foreach(QString subDirectory, subDirectories)
                if (!cacheDir_.exists(subDirectory) && !cacheDir_.mkpath(subDirectory))
                    LogError("ZipAssetBundle::DeserializeFromData: Failed to create subfolder \"" + subDirectory + "\" to cache directory.");

            // Sort by size so we have to resize the buffer less times.
            qSort(files_.begin(), files_.end(), ArchiveFileSizeCompare);

            // Read file contents
            zzip_ssize_t chunkLen = 0;
            zzip_ssize_t chunkRead = 0;
            std::vector<u8> buffer;

            foreach(ArchiveFile file, files_)
            {
                if (file.isFile)
                {
                    //LogInfo("Processing: " + file.toString());

                    // Open file from zip
                    ZZIP_FILE *zzipFile = zzip_file_open(archive_, file.relativePath.toStdString().c_str(), ZZIP_ONLYZIP | ZZIP_CASELESS);
                    if (zzipFile && !CheckError())
                    {
                        /** @todo Do some modified time checking, if at all possible, so we don't unpack files that have not changed for nothing!
                            AssetAPI should only redownload if the zip file has changed in the storage but we need to check our internal things here. */

                        // Create cache file
                        QFile cacheFile(cacheDir_.absoluteFilePath(file.relativePath));
                        if (!cacheFile.open(QIODevice::WriteOnly))
                        {
                            LogError("ZipAssetBundle::DeserializeFromData: Failed to open cache file: " + cacheFile.fileName());
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
            }

            // Close the zzip directory ptr
            Close();

            // If the zip file was empty we don't want IsLoaded to fail on the files_ check.
            // The bundle loaded fine but there was no content, log a warning.
            if (files_.isEmpty())
            {
                LogWarning("ZipAssetBundle: Bundle loaded but does not contain any files " + Name());
                files_ << ArchiveFile();
            }
        }
        else
            return false;

        LogInfo("** " + Name() + " completed in " + QString::number(t.elapsed()) + " ms");
        emit Loaded(this);
        return true;
    }
    else
    {
        LogError("ZipAssetBundle::DeserializeFromData: Cannot process archive, no disk source for " + Name());
        return false;
    }
}

bool ZipAssetBundle::DeserializeFromData(const u8 *data, size_t numBytes)
{
    /** @note At this point it seems zzip needs a disk source to do processing
        so we require disk source for the archive. This might change in the future with changing the lib */
    return false;
}

std::vector<u8> ZipAssetBundle::GetSubAssetData(const QString &subAssetName)
{
    /** Makes no sense to keep the whole zip file in cache at the moment.
        We cant however make this function also open the zip file and uncompress the data,
        but that is rather pointless, not to mention slower, as we already have 
        the unpacked individual assets on disk. If the unpacking to disk changes we might
        need to rething this. */

    QString filePath = GetSubAssetDiskSource(subAssetName);
    if (filePath.isEmpty())
        return std::vector<u8>();

    std::vector<u8> data;
    return LoadFileToVector(filePath, data) ? data : std::vector<u8>();
}

QString ZipAssetBundle::GetSubAssetDiskSource(const QString &subAssetName)
{
    // AssetAPI should have take care of any in front "/" forward slashes in the sub asset name.
    QString absolutePath = cacheDir_.absoluteFilePath(subAssetName);
    return QFile::exists(absolutePath) ? absolutePath : "";
}

bool ZipAssetBundle::IsLoaded() const
{
    return (archive_ != 0 || !files_.isEmpty());
}

bool ZipAssetBundle::CheckError()
{
    if (!archive_)
        return false;

    QString errorMsg;
    switch (zzip_error(archive_))
    {
        case ZZIP_NO_ERROR:
            break;
        case ZZIP_OUTOFMEM:
            errorMsg = "Out of memory.";
            break;            
        case ZZIP_DIR_OPEN:
        case ZZIP_DIR_STAT: 
        case ZZIP_DIR_SEEK:
        case ZZIP_DIR_READ:
            errorMsg = "Unable to read zip file.";
            break;            
        case ZZIP_UNSUPP_COMPR:
            errorMsg = "Unsupported compression format.";
            break;            
        case ZZIP_CORRUPTED:
            errorMsg = "Corrupted archive.";
            break;            
        default:
            errorMsg = "Unknown error.";
            break;            
    };

    if (!errorMsg.isEmpty())
        LogError("ZipAssetBundle:" + errorMsg);
    return !errorMsg.isEmpty();
}

void ZipAssetBundle::Close()
{
    if (archive_)
    {
        zzip_dir_close(archive_);
        archive_ = 0;
    }
}

