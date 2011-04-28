// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "AssetModule.h"
#include "CoreTypes.h"
#include "LocalAssetProvider.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"
#include "LocalAssetStorage.h"
#include "IAssetUploadTransfer.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"
#include "CoreStringUtils.h"
#include <QDir>
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>

namespace Asset
{

LocalAssetProvider::LocalAssetProvider(Foundation::Framework* framework_)
:framework(framework_)
{
    /// \Todo: react to AssetDiscovered & AssetDeleted like HttpAssetProvider does
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
    if (assetRef.isEmpty())
        return AssetTransferPtr();
    assetType = assetType.trimmed();
    if (assetType.isEmpty())
        assetType = AssetAPI::GetResourceTypeFromAssetRef(assetRef.toStdString().c_str());

    AssetTransferPtr transfer = AssetTransferPtr(new IAssetTransfer);
    transfer->source.ref = assetRef.trimmed();
    transfer->assetType = assetType;

    pendingDownloads.push_back(transfer);

    return transfer;
}

QString LocalAssetProvider::GetPathForAsset(const QString &assetRef, LocalAssetStoragePtr *storage)
{
    QString path;
    QString path_filename;
    AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(assetRef.trimmed(), 0, 0, 0, 0, &path_filename, &path);
    if (refType == AssetAPI::AssetRefLocalPath)
        return path; // If the asset ref has already been converted to an absolute path, simply return the assetRef as is.

    // Check first all subdirs without recursion, because recursion is potentially slow
    for(size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(path_filename.toStdString().c_str(), false);
        if (path != "")
        {
            if (storage)
                *storage = storages[i];
            return path;
        }
    }

    for(size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(path_filename.toStdString().c_str(), true);
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

void LocalAssetProvider::Update(f64 frametime)
{
    ///\note It is *very* important that below we first complete all uploads, and then the downloads.
    /// This is because it is a rather common code flow to upload an asset for an entity, and immediately after that
    /// generate a entity in the scene that refers to that asset, which means we do both an upload and a download of the
    /// asset into the same asset storage. If the download request was processed before the upload request, the download
    /// request would fail on missing file, and the entity would erroneously get an "asset not found" result.
    CompletePendingFileUploads();
    CompletePendingFileDownloads();
}

void LocalAssetProvider::DeleteAssetFromStorage(QString assetRef)
{
    if (!assetRef.isEmpty())
        QFile::remove(assetRef); ///\todo Check here that the assetRef points to one of the accepted storage directories, and don't allow deleting anything else.
    
    AssetModule::LogInfo("LocalAssetProvider::DeleteAssetFromStorage: Deleted asset file \"" + assetRef.toStdString() + "\" from disk.");
    framework->Asset()->EmitAssetDeleted(assetRef);
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

LocalAssetStoragePtr LocalAssetProvider::AddStorageDirectory(const QString &directory, QString storageName, bool recursive)
{
    storageName = storageName.trimmed();
    if (storageName.isEmpty())
        return LocalAssetStoragePtr();

    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->name.compare(storageName, Qt::CaseInsensitive) == 0)
        {
            if (storages[i]->directory != directory)
                AssetModule::LogError("LocalAssetProvider::AddStorageAddress failed: A storage by name \"" + storageName.toStdString() + "\" already exists, but points to directory \"" + storages[i]->directory.toStdString() + "\" instead of \"" + directory.toStdString() + "\"!");
            return LocalAssetStoragePtr();
        }

    LocalAssetStoragePtr storage = LocalAssetStoragePtr(new LocalAssetStorage());
    storage->directory = directory;
    storage->name = storageName;
    storage->recursive = recursive;
    storage->provider = shared_from_this();
    storage->SetupWatcher(); // Start listening on file change notifications.
//    connect(storage->changeWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(FileChanged(QString)));
//    connect(storage->changeWatcher, SIGNAL(fileChanged(QString)), this, SLOT(FileChanged(QString)));

    storages.push_back(storage);

    return storage;
}

std::vector<AssetStoragePtr> LocalAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> stores;
    for(size_t i = 0; i < storages.size(); ++i)
        stores.push_back(storages[i]);
    return stores;
}

AssetUploadTransferPtr LocalAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    assert(data);
    if (!data)
    {
        AssetModule::LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Null source data pointer passed to function!");
        return AssetUploadTransferPtr();
    }

    LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(destination.get());
    if (!storage)
    {
        AssetModule::LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Invalid destination asset storage type! Was not of type LocalAssetStorage!");
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
    while(pendingDownloads.size() > 0)
    {
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
        //            AssetModule::LogWarning(reason.toStdString());
                    framework->Asset()->AssetTransferFailed(transfer.get(), reason);
                    continue;
                }
            
                file = QFileInfo(GuaranteeTrailingSlash(path) + path_filename);
            }
        }
        QString absoluteFilename = file.absoluteFilePath();

        bool success = LoadFileToVector(absoluteFilename.toStdString().c_str(), transfer->rawAssetData);
        if (!success)
        {
            QString reason = "Failed to read asset data for asset \"" + ref + "\" from file \"" + absoluteFilename + "\"";
//            AssetModule::LogError(reason.toStdString());
            framework->Asset()->AssetTransferFailed(transfer.get(), reason);
            continue;
        }
        
        // Tell the Asset API that this asset should not be cached into the asset cache, and instead the original filename should be used
        // as a disk source, rather than generating a cache file for it.
        transfer->SetCachingBehavior(false, absoluteFilename.toStdString().c_str());

        transfer->storage = storage;
//        AssetModule::LogDebug("Downloaded asset \"" + ref.toStdString() + "\" from file " + absoluteFilename.toStdString());

        // Signal the Asset API that this asset is now successfully downloaded.
        framework->Asset()->AssetTransferCompleted(transfer.get());
    }
}

AssetStoragePtr LocalAssetProvider::TryDeserializeStorageFromString(const QString &storage)
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
    if (s.contains("recursive"))
        recursive = ParseBool(s["recursive"]);

    return AddStorageDirectory(path, name, recursive);
}

QString LocalAssetProvider::GenerateUniqueStorageName() const
{
    QString name = "Scene";
    int counter = 2;
    while(GetStorageByName(name) != 0)
        name = "Scene" + counter++;
    return name;
}

AssetStoragePtr LocalAssetProvider::GetStorageByName(const QString &name) const
{
    for(size_t i = 0; i < storages.size(); ++i)
        if (storages[i]->name.compare(name, Qt::CaseInsensitive) == 0)
            return storages[i];

    return AssetStoragePtr();
}

void LocalAssetProvider::CompletePendingFileUploads()
{
    while(pendingUploads.size() > 0)
    {
        AssetUploadTransferPtr transfer = pendingUploads.back();
        pendingUploads.pop_back();

        LocalAssetStoragePtr storage = boost::dynamic_pointer_cast<LocalAssetStorage>(transfer->destinationStorage.lock());
        if (!storage)
        {
            AssetModule::LogError("Invalid IAssetStorage specified for file upload in LocalAssetProvider!");
            transfer->EmitTransferFailed();
            continue;
        }

        if (transfer->sourceFilename.length() == 0 && transfer->assetData.size() == 0)
        {
            AssetModule::LogError("No source data present when trying to upload asset to LocalAssetProvider!");
            continue;
        }

        QString fromFile = transfer->sourceFilename;
        QString toFile = GuaranteeTrailingSlash(storage->directory) + transfer->destinationName;

        bool success;
        if (fromFile.length() == 0)
            success = SaveAssetFromMemoryToFile(&transfer->assetData[0], transfer->assetData.size(), toFile.toStdString().c_str());
        else
            success = CopyAssetFile(fromFile.toStdString().c_str(), toFile.toStdString().c_str());

        if (!success)
        {
            AssetModule::LogError(("Asset upload failed in LocalAssetProvider: CopyAsset from \"" + fromFile + "\" to \"" + toFile + "\" failed!").toStdString());
            transfer->EmitTransferFailed();
            /// \todo Jukka lisää failure-notifikaatio.
        }
        else
        {
            framework->Asset()->AssetUploadTransferCompleted(transfer.get());
        }
    }
}

void LocalAssetProvider::FileChanged(const QString &path)
{
    AssetModule::LogInfo(("File " + path + " changed.").toStdString());
}

} // ~Asset
