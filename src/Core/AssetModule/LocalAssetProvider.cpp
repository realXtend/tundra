// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Win.h"
#include "LocalAssetProvider.h"
#include "LocalAssetStorage.h"
#include "AssetModule.h"
#include "IAssetUploadTransfer.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"
#include "IAsset.h"
#include "Profiler.h"

#include "Framework.h"
#include "LoggingFunctions.h"
#include "CoreStringUtils.h"
#include "FileUtils.h"
#include "Profiler.h"

#include <QDir>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMap>

#include "MemoryLeakCheck.h"

LocalAssetProvider::LocalAssetProvider(Framework* framework_) :
    framework(framework_)
{
    enableRequestsOutsideStorages = framework_->HasCommandLineParameter("--accept_unknown_local_sources");
}

LocalAssetProvider::~LocalAssetProvider()
{
}

QString LocalAssetProvider::Name()
{
    static const QString name("Local");
    return name;
}

bool LocalAssetProvider::IsValidRef(QString assetRef, QString)
{
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef.trimmed());
    if (refType == AssetAPI::AssetRefLocalPath || refType == AssetAPI::AssetRefLocalUrl)
        return true;

    if (refType == AssetAPI::AssetRefRelativePath)
    {
        QString path = GetPathForAsset(assetRef, 0);
        return !path.isEmpty();
    }
    else
        return false;
}

AssetTransferPtr LocalAssetProvider::RequestAsset(QString assetRef, QString assetType)
{
    PROFILE(LocalAssetProvider_RequestAsset);
    if (assetRef.isEmpty())
        return AssetTransferPtr();
    assetType = assetType.trimmed();
    if (assetType.isEmpty())
        assetType = framework->Asset()->GetResourceTypeFromAssetRef(assetRef);

    if (!enableRequestsOutsideStorages)
    {
        AssetStoragePtr storage = GetStorageForAssetRef(assetRef);
        if (!storage)
        {
            LogError("LocalAssetProvider::RequestAsset: Discarding asset request to path \"" + assetRef + "\" because requests to sources outside registered LocalAssetStorages have been forbidden. (See --accept_unknown_local_sources).");
            return AssetTransferPtr();
        }
    }

    AssetTransferPtr transfer = AssetTransferPtr(new IAssetTransfer);
    transfer->source.ref = assetRef.trimmed();
    transfer->assetType = assetType;
    transfer->diskSourceType = IAsset::Original; // The disk source represents the original authoritative source for the asset.
    
    pendingDownloads.push_back(transfer);

    return transfer;
}

bool LocalAssetProvider::AbortTransfer(IAssetTransfer *transfer)
{
    if (!transfer)
        return false;

    for (std::vector<AssetTransferPtr>::iterator iter = pendingDownloads.begin(); iter != pendingDownloads.end(); ++iter)
    {
        AssetTransferPtr ongoingTransfer = (*iter);
        if (ongoingTransfer.get() == transfer)
        {
            framework->Asset()->AssetTransferAborted(transfer);
            
            ongoingTransfer.reset();
            pendingDownloads.erase(iter);
            return true;
        }
    }
    return false;
}

QString LocalAssetProvider::GetPathForAsset(const QString &assetRef, LocalAssetStoragePtr *storage) const
{
    QString path;
    QString path_filename;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef.trimmed(), 0, 0, 0, 0, &path_filename, &path);
    if (refType == AssetAPI::AssetRefLocalPath)
    {
        // If the asset ref has already been converted to an absolute path, simply return the assetRef as is.
        // However, lookup also the storage if wanted
        if (storage)
        {
            for (size_t i = 0; i < storages.size(); ++i)
            {
                if (path.startsWith(storages[i]->directory, Qt::CaseInsensitive))
                {
                    *storage = storages[i];
                    return path;
                }
            }
        }
        
        return path;
    }
    // Check first all subdirs without recursion, because recursion is potentially slow
    for (size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(path_filename, false);
        if (path != "")
        {
            if (storage)
                *storage = storages[i];
            return path;
        }
    }

    for (size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(path_filename, true);
        if (path != "")
        {
            if (storage)
                *storage = storages[i];
            return path;
        }
    }
    
    if (storage)
        *storage = LocalAssetStoragePtr();
    return "";
}

void LocalAssetProvider::Update(f64 /*frametime*/)
{
    PROFILE(LocalAssetProvider_Update);

    ///@note It is *very* important that below we first complete all uploads, and then the downloads.
    /// This is because it is a rather common code flow to upload an asset for an entity, and immediately after that
    /// generate a entity in the scene that refers to that asset, which means we do both an upload and a download of the
    /// asset into the same asset storage. If the download request was processed before the upload request, the download
    /// request would fail on missing file, and the entity would erroneously get an "asset not found" result.
    CompletePendingFileUploads();
    CompletePendingFileDownloads();
    CheckForPendingFileSystemChanges();
}

void LocalAssetProvider::DeleteAssetFromStorage(QString assetRef)
{
    if (!assetRef.isEmpty())
    {
        LocalAssetStoragePtr storage;
        QString path = GetPathForAsset(assetRef, &storage);
        if (!storage)
        {
            LogError("LocalAssetProvider::DeleteAssetFromStorage: Could not verify the asset storage pointed by \"" + assetRef + "\"!");
            return;
        }

        QString fullFilename = path + QDir::separator() + AssetAPI::ExtractFilenameFromAssetRef(assetRef);
        bool success = QFile::remove(fullFilename);
        if (success)
        {
            LogInfo("LocalAssetProvider::DeleteAssetFromStorage: Deleted asset \"" + assetRef + "\", file " + fullFilename + " from disk.");
            framework->Asset()->EmitAssetDeletedFromStorage(assetRef);
        }
        else
        {
            LogError("Could not delete asset " + assetRef);
        }
    }
}

bool LocalAssetProvider::RemoveAssetStorage(QString storageName)
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->name.compare(storageName, Qt::CaseInsensitive) == 0)
        {
            storages.erase(storages.begin() + i);
            return true;
        }

    return false;
}

LocalAssetStoragePtr LocalAssetProvider::AddStorageDirectory(QString directory, QString storageName, bool recursive, bool writable, bool liveUpdate, bool autoDiscoverable)
{
    directory = directory.trimmed();
    if (directory.isEmpty())
    {
        LogError("LocalAssetProvider: Cannot add storage \"" + storageName + "\" to an empty directory!");
        return LocalAssetStoragePtr();
    }
    directory = QDir::toNativeSeparators(GuaranteeTrailingSlash(directory));

    storageName = storageName.trimmed();
    if (storageName.isEmpty())
    {
        LogInfo("LocalAssetProvider: Cannot add storage with an empty name to directory \"" + directory + "\"!");
        return LocalAssetStoragePtr();
    }

    // Test if we already have a storage registered with this name.
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->name.compare(storageName, Qt::CaseInsensitive) == 0)
        {
            if (storages[i]->directory != directory)
            {
                LogWarning("LocalAssetProvider: Storage '" + storageName + "' already exist in '" + storages[i]->directory + "', not adding with '" + directory + "'.");
                return LocalAssetStoragePtr();
            }
            else // We already have a storage with that name and target directory registered, just return that.
                return storages[i];
        }

    //LogInfo("LocalAssetProvider::AddStorageDirectory " + directory);
    LocalAssetStoragePtr storage = LocalAssetStoragePtr(new LocalAssetStorage(writable, liveUpdate, autoDiscoverable));
    storage->directory = QDir::toNativeSeparators(GuaranteeTrailingSlash(directory));
    storage->name = storageName;
    storage->recursive = recursive;
    storage->provider = shared_from_this();
// On Android, we get spurious file change notifications. Disable watcher for now.
#ifndef ANDROID
    storage->SetupWatcher(); // Start listening on file change notifications. Note: it's important that recursive is set before calling this!
    connect(storage->changeWatcher, SIGNAL(directoryChanged(const QString&)), SLOT(OnDirectoryChanged(const QString &)), Qt::UniqueConnection);
    connect(storage->changeWatcher, SIGNAL(fileChanged(const QString &)), SLOT(OnFileChanged(const QString &)), Qt::UniqueConnection);
#endif
    storages.push_back(storage);

    // Tell the Asset API that we have created a new storage.
    framework->Asset()->EmitAssetStorageAdded(storage);

    // If autodiscovery is on, make the storage refresh itself immediately.
    if (storage->AutoDiscoverable())
        storage->RefreshAssetRefs();

    return storage;
}

std::vector<AssetStoragePtr> LocalAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> stores;
    for(size_t i = 0; i < storages.size(); ++i)
        stores.push_back(storages[i]);
    return stores;
}

AssetUploadTransferPtr LocalAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName)
{
    assert(data);
    if (!data)
    {
        LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Null source data pointer passed to function!");
        return AssetUploadTransferPtr();
    }

    LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(destination.get());
    if (!storage)
    {
        LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Invalid destination asset storage type! Was not of type LocalAssetStorage!");
        return AssetUploadTransferPtr();
    }

    AssetUploadTransferPtr transfer = AssetUploadTransferPtr(new IAssetUploadTransfer());
    transfer->sourceFilename = "";
    transfer->destinationName = assetName;
    transfer->destinationStorage = destination;
    transfer->assetData.insert(transfer->assetData.end(), data, data + numBytes);

    pendingUploads.push_back(transfer);

    return transfer;
}

void LocalAssetProvider::CompletePendingFileDownloads()
{
    // If we have any uploads running, first wait for each of them to complete, until we download any more.
    // This is because we might want to download the same asset that we uploaded, so they must be done in
    // the proper order.
    if (pendingUploads.size() > 0)
        return;

    const int maxLoadMSecs = 16;
    tick_t startTime = GetCurrentClockTime();

    while(pendingDownloads.size() > 0)
    {
        PROFILE(LocalAssetProvider_ProcessPendingDownload);

        AssetTransferPtr transfer = pendingDownloads.back();
        pendingDownloads.pop_back();
            
        QString ref = transfer->source.ref;

        QString path_filename;
        AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(ref.trimmed(), 0, 0, 0, 0, &path_filename);

        LocalAssetStoragePtr storage;

        QFileInfo file;

        if (refType == AssetAPI::AssetRefLocalPath)
        {
            file = QFileInfo(path_filename);
        }
        else // Using a local relative path, like "local://asset.ref" or "asset.ref".
        {
            AssetAPI::AssetRefType urlRefType = AssetAPI::ParseAssetRef(path_filename);
            if (urlRefType == AssetAPI::AssetRefLocalPath)
                file = QFileInfo(path_filename); // 'file://C:/path/to/asset/asset.png'.
            else // The ref is of form 'file://relativePath/asset.png'.
            {
                QString path = GetPathForAsset(path_filename, &storage);
                if (path.isEmpty())
                {
                    QString reason = "Failed to find local asset with filename \"" + ref + "\"!";
                    framework->Asset()->AssetTransferFailed(transfer.get(), reason);
                    // Also throttle asset loading here. This is needed in the case we have a lot of failed refs.
                    //if (GetCurrentClockTime() - startTime >= GetCurrentClockFreq() * maxLoadMSecs / 1000)
                    //    break;
                    continue;
                }
            
                file = QFileInfo(GuaranteeTrailingSlash(path) + path_filename);
            }
        }
        QString absoluteFilename = file.absoluteFilePath();

        bool success = LoadFileToVector(absoluteFilename, transfer->rawAssetData);
        if (!success)
        {
            QString reason = "Failed to read asset data for asset \"" + ref + "\" from file \"" + absoluteFilename + "\"";
            framework->Asset()->AssetTransferFailed(transfer.get(), reason);
            // Also throttle asset loading here. This is needed in the case we have a lot of failed refs.
            if (GetCurrentClockTime() - startTime >= GetCurrentClockFreq() * maxLoadMSecs / 1000)
                break;
            continue;
        }
        
        // Tell the Asset API that this asset should not be cached into the asset cache, and instead the original filename should be used
        // as a disk source, rather than generating a cache file for it.
        transfer->SetCachingBehavior(false, absoluteFilename);
        transfer->storage = storage;

        // Signal the Asset API that this asset is now successfully downloaded.
        framework->Asset()->AssetTransferCompleted(transfer.get());

        // Throttle asset loading to at most 16 msecs/frame.
        if (GetCurrentClockTime() - startTime >= GetCurrentClockFreq() * maxLoadMSecs / 1000)
            break;
    }
}

AssetStoragePtr LocalAssetProvider::TryDeserializeStorageFromString(const QString &storage, bool /*fromNetwork*/)
{
    QMap<QString, QString> s = AssetAPI::ParseAssetStorageString(storage);
    if (s.contains("type") && s["type"].compare("LocalAssetStorage", Qt::CaseInsensitive) != 0)
        return AssetStoragePtr();
    if (!s.contains("src"))
        return AssetStoragePtr();

    QString path;
    QString protocolPath;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(s["src"], 0, 0, &protocolPath, 0, 0, &path);

    if (refType == AssetAPI::AssetRefRelativePath)
    {
        path = GuaranteeTrailingSlash(QDir::currentPath()) + path;
        refType = AssetAPI::AssetRefLocalPath;
    }
    if (refType != AssetAPI::AssetRefLocalPath)
        return AssetStoragePtr();

    QString name = (s.contains("name") ? s["name"] : GenerateUniqueStorageName());

    bool recursive = true;
    bool writable = true;
    bool liveUpdate = true;
    bool autoDiscoverable = true;
    if (s.contains("recursive"))
        recursive = ParseBool(s["recursive"]);

    if (s.contains("readonly"))
        writable = !ParseBool(s["readonly"]);

    if (s.contains("liveupdate"))
        liveUpdate = ParseBool(s["liveupdate"]);
    
    if (s.contains("autodiscoverable"))
        autoDiscoverable = ParseBool(s["autodiscoverable"]);

    LocalAssetStoragePtr storagePtr = AddStorageDirectory(path, name, recursive, writable, liveUpdate, autoDiscoverable);

    ///\bug Refactor these sets to occur inside AddStorageDirectory so that when the NewStorageAdded signal is emitted, these values are up to date.
    if (storagePtr)
    {
        if (s.contains("replicated"))
            storagePtr->SetReplicated(ParseBool(s["replicated"]));
        if (s.contains("trusted"))
            storagePtr->trustState = IAssetStorage::TrustStateFromString(s["trusted"]);
    }

    return storagePtr;
}

QString LocalAssetProvider::GenerateUniqueStorageName() const
{
    QString name = "Scene";
    int counter = 2;
    while(GetStorageByName(name) != 0)
        name = "Scene" + QString::number(counter++);
    return name;
}

LocalAssetStoragePtr LocalAssetProvider::FindStorageForPath(const QString &path) const
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (GuaranteeTrailingSlash(path).contains(GuaranteeTrailingSlash(QDir::fromNativeSeparators(storages[i]->directory))))
            return storages[i];
    return LocalAssetStoragePtr();
}

AssetStoragePtr LocalAssetProvider::GetStorageByName(const QString &name) const
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->name.compare(name, Qt::CaseInsensitive) == 0)
            return storages[i];

    return AssetStoragePtr();
}

AssetStoragePtr LocalAssetProvider::GetStorageForAssetRef(const QString &assetRef) const
{
    PROFILE(LocalAssetProvider_GetStorageForAssetRef);

    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef.trimmed());
    if (refType != AssetAPI::AssetRefLocalPath && refType != AssetAPI::AssetRefLocalUrl)
        return AssetStoragePtr();

    LocalAssetStoragePtr storage;
    GetPathForAsset(assetRef, &storage);
    return static_pointer_cast<IAssetStorage>(storage);
}

void LocalAssetProvider::CompletePendingFileUploads()
{
    while(pendingUploads.size() > 0)
    {
        PROFILE(LocalAssetProvider_ProcessPendingUpload);
        AssetUploadTransferPtr transfer = pendingUploads.back();
        pendingUploads.pop_back();

        LocalAssetStoragePtr storage = dynamic_pointer_cast<LocalAssetStorage>(transfer->destinationStorage.lock());
        if (!storage)
        {
            LogError("Invalid IAssetStorage specified for file upload in LocalAssetProvider!");
            transfer->EmitTransferFailed();
            continue;
        }

        if (transfer->sourceFilename.length() == 0 && transfer->assetData.size() == 0)
        {
            LogError("No source data present when trying to upload asset to LocalAssetProvider!");
            continue;
        }

        QString fromFile = transfer->sourceFilename;
        QString toFile = GuaranteeTrailingSlash(storage->directory) + transfer->destinationName;

        bool success;
        if (fromFile.length() == 0)
            success = SaveAssetFromMemoryToFile(&transfer->assetData[0], transfer->assetData.size(), toFile);
        else
            success = CopyAssetFile(fromFile, toFile);

        if (!success)
        {
            LogError("Asset upload failed in LocalAssetProvider: CopyAsset from \"" + fromFile + "\" to \"" + toFile + "\" failed!");
            transfer->EmitTransferFailed();
        }
        else
        {
            framework->Asset()->AssetUploadTransferCompleted(transfer.get());
        }
    }
}

void LocalAssetProvider::CheckForPendingFileSystemChanges()
{
    PROFILE(LocalAssetProvider_CheckForPendingFileSystemChanges);
    QStringList files = changedFiles.toList();
    while(!files.isEmpty())
    {
        QString file = files.front();
        files.pop_front();

//        // If some watched file is changed, so is watched directory. Remove the directory from 
//        // changedDirectories so we can ignore it later on when checking directory changes.
//        
//        QString dir = QFileInfo(file).dir().path();
//        if (qFind(changedDirectories, dir) != changedDirectories.end())
//        {
//            LogInfo("REMOVING " + dir + "FROM changedDirectories");
//            changedDirectories.removeOne(dir);
//        }
//        
        LocalAssetStoragePtr storage = FindStorageForPath(file);
        if (storage)
        {
            if (!storage->AutoDiscoverable())
            {
                LogWarning("Received file change notification for storage of which auto-discovery is false.");
                continue;
            }

            assert(storage->changeWatcher);
            QString assetRef = file;
            int lastSlash = assetRef.lastIndexOf('/');
            if (lastSlash != -1)
                assetRef = assetRef.right(assetRef.length() - lastSlash - 1);
            assetRef.prepend("local://");

            // Note: if file was removed, it's removed automatically from tracked files of QFileSystemWatcher.
            const QStringList watchedFiles = storage->changeWatcher->files();
            if (qFind(watchedFiles, file) == watchedFiles.end() && !QFile::exists(file))
            {
                // Tracked file was not found from the list of tracked files and it doesn't exist so 
                // it must be deleted (info about new files is retrieved by directoryChanged signal).
                LogInfo("File " + file + " not found from watch list. So it must be deleted.");
                storage->EmitAssetChanged(file, IAssetStorage::AssetDelete);
            }
            else
            {
                // File was tracked and found from watched files: must've been modified.
                LogInfo("File " + file + " found from watch list so it must be modified. Asset ref: " + assetRef);
                storage->EmitAssetChanged(file, IAssetStorage::AssetModify);
            }
        }
        else
        {
            LogError("LocalAssetProvider::CheckForPendingFileSystemChanges: Could not find storage for file " + file);
        }
    }

    QStringList dirs = changedDirectories.toList();
    while(!dirs.isEmpty())
    {
        QString path = dirs.front();
        dirs.pop_front();
        LocalAssetStoragePtr storage = FindStorageForPath(path);
        if (storage)
        {
            if (!storage->AutoDiscoverable())
            {
                LogWarning("Received directory change notification for storage of which auto-discovery is false.");
                continue;
            }

            assert(storage->changeWatcher);
            const QStringList watchedDirs = storage->changeWatcher->directories();
            if (qFind(watchedDirs, path) != watchedDirs.end())
            {
                //LogError("CheckForPendingFileSystemChanges: " + path + " was on the watch list.");
                // Remove path from watch list in case the directory was removed.
                // We'll add it back anyways in case of this change was addition of new directory.
                // Remove the path only if it's not the root directory of the storage
                //if (!QString(path + "/").compare(QDir::fromNativeSeparators(storage->directory), Qt::CaseInsensitive) != 0)
                //{
                    //LogInfo("LocalAssetProvider: Directory " + path + " was on the watch list, removing it by default always.");
                   // storage->changeWatcher->removePath(path);
                //}
/*
                if (!QDir(path).exists()) // 1: Was directory deleted?
                {
                    LogDebug("Directory " + path + " removed.");
                    QStringList subdirs = DirectorySearch(path, true, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                    if (!subdirs.isEmpty())
                    {
                        LogInfo("Directory " + path + " was removed. Removing all of its subdirs too:\n" + subdirs.join("\n"));
                        storage->changeWatcher->removePaths(subdirs);
                    }
                    // Add parent dir back to watch list.
                    if (!watchedDirs.contains(path), Qt::CaseInsensitive)
                        storage->changeWatcher->addPath(path);
                }
                else // 2: Was new directory added, an existing one renamed, or something else?
*/
                {
                    // Was new file added?
                    QStringList oldFiles, currentFiles;
                    currentFiles = DirectorySearch(path, false, QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                    const QStringList watchedFiles = storage->changeWatcher->files();
                    foreach(const QString &file, watchedFiles)
                        if (GuaranteeTrailingSlash(path) == GuaranteeTrailingSlash(QFileInfo(file).dir().path()))
                            oldFiles.append(file);

                    if (currentFiles.size() > oldFiles.size())
                    {
                        LogDebug("NumOldFiles for " + path + " " + QString::number(oldFiles.size()) + " NumCurFiles: " + QString::number(currentFiles.size()));
                        LogDebug("Conclusion: new file added.");
                        foreach(const QString &file, currentFiles)
                        {
                            QStringList::const_iterator it = qFind(oldFiles, file);
                            if (it == oldFiles.end())
                            {
                                LogInfo("New file " + file + " added to storage " + storage->ToString());
                                storage->changeWatcher->addPath(file);
                                storage->EmitAssetChanged(file, IAssetStorage::AssetCreate);
                                break;
                            }
                    }
                    //else if(if (currentFiles.size() > oldFiles.size())
                    //{
                    //}
/*
                    // Check out the path's subdir count. Find all old watched dirs and replace them with the new dirs.
                    QStringList curSubdirs = DirectorySearch(path, true, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                    QStringList oldSubdirs;
                    foreach(const QString &dir, watchedDirs)
                        if (dir.contains(path, Qt::CaseInsensitive) && dir.compare(path, Qt::CaseInsensitive) != 0)
                            oldSubdirs << dir;

                    LogWarning("CURRENT DIRS " + curSubdirs.join("\n"));
                    LogWarning("OLD DIRS " + oldSubdirs.join("\n"));
                    LogInfo("NumOldSubdirs for " + path + ": " + QString::number(oldSubdirs.size()) + ", NumCurDirs: " + QString::number(curSubdirs.size()));
                    if (!oldSubdirs.isEmpty() && !curSubdirs.isEmpty() && oldSubdirs.size() == curSubdirs.size())
                    {
                        // Folder rename occurred. Remove all subdirs and add them back.
                        LogError("SOME FOLDER WAS RENAMED");
                        LogInfo("Removing\n" + oldSubdirs.join("\n"));

                        storage->changeWatcher->removePaths(oldSubdirs);

                        LogInfo("Adding\n" + curSubdirs.join("\n"));
                        foreach(const QString &d, curSubdirs) // for some odd reason addPaths() fails, so have to call addPath() for individual paths instead.
                            if (!watchedDirs.contains(d), Qt::CaseInsensitive)
                                storage->changeWatcher->addPath(d);

                        // Add parent dir back to watch list.
                        if (!watchedDirs.contains(path), Qt::CaseInsensitive)
                            storage->changeWatcher->addPath(path);
                    }
                    else if (oldSubdirs.size() != curSubdirs.size())
                    {
                        LogError("NEW DIRECTORY");
                        LogInfo("New directory added to " + path);
                        //LogInfo("Directory structure changed: number of old subdirs for " + path + " " + QString::number(oldSubdirs.size()) +
                        //    ", number of current dirs " + QString::number(curSubdirs.size()));
                        LogInfo("This means we got a new directory to watch.");
                        if (!oldSubdirs.isEmpty())
                        {
                            LogInfo("Removing oldSubdirs:\n" + oldSubdirs.join("\n"));
                            storage->changeWatcher->removePaths(oldSubdirs);
                        }
                        if (!curSubdirs.isEmpty())
                        {
                            LogInfo("Adding curSubdirs:\n" + curSubdirs.join("\n"));
                            foreach(const QString &d, curSubdirs) // for some odd reason addPaths() fails, so have to call addPath() for individual paths instead.
                                if (!watchedDirs.contains(d), Qt::CaseInsensitive)
                                    storage->changeWatcher->addPath(d);
                        }

                        QString newDir = curSubdirs.size() == 1 ? curSubdirs.first() : "";
                        foreach(const QString &curSubdir, curSubdirs)
                            if (qFind(oldSubdirs, curSubdir) == oldSubdirs.end())
                            {
                                newDir = curSubdir;
                                LogInfo("New dir's name is : " + newDir);
                                if (storage->recursive)
                                {
                                    QStringList pathsToAdd = DirectorySearch(newDir, true, QDir::Files |QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);
                                    if (!pathsToAdd.isEmpty())
                                    {
                                        LogInfo("Adding paths to watch list: " + pathsToAdd.join("\n"));
                                        LogInfo("******before " + QString::number(storage->changeWatcher->files().count()));
                                        storage->changeWatcher->addPaths(pathsToAdd);
                                        LogInfo("******after " + QString::number(storage->changeWatcher->files().count()));
                                    }
                                }
                            }

                        // Add parent dir back to watch list.
                        if (!watchedDirs.contains(path), Qt::CaseInsensitive)
                            storage->changeWatcher->addPath(path);
                    }
                    else
                    {
                        // We end up here after:
                        // -removing subdir of a watched dir we get change notification for the watcher dir,
                        // -Addition of new file to watched dir
                        LogError("DIDN'T KNOW WHAT TO DO WITH CHANGED DIR");
                        if (!watchedDirs.contains(path), Qt::CaseInsensitive)
                            storage->changeWatcher->addPath(path);

                        QStringList pathsToAdd = DirectorySearch(path, false, QDir::Files);
                        if (!pathsToAdd.isEmpty())
                        {
                            LogInfo("Adding paths to watch list: " + pathsToAdd.join("\n"));
                            foreach(const QString &p, pathsToAdd) // for some odd reason addPaths() fails, so have to call addPath() for individual paths instead.
                                storage->changeWatcher->addPath(p);
                        }
*/
                    }
                }

                //LogInfo("Refreshing storage \"" + storage->Name() + "\".");
                //storage->RefreshAssetRefs();
            }
        }
        else
        {
            LogError("LocalAssetProvider::CheckForPendingFileSystemChanges: Could not find storage for directory " + path);
        }
    }

    changedFiles.clear();
    changedDirectories.clear();
}

void LocalAssetProvider::OnFileChanged(const QString &path)
{
    LogDebug("LocalAssetProvider: File " + path + " changed.");
    changedFiles << path;
}

void LocalAssetProvider::OnDirectoryChanged(const QString &path)
{
    LogDebug("LocalAssetProvider: Directory " + path + " changed.");
    changedDirectories << path;
}
