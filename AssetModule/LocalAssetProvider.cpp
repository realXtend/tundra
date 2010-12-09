// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "LocalAssetProvider.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"
#include "LocalAssetStorage.h"
#include "IAssetUploadTransfer.h"
#include "IAssetTransfer.h"
#include "AssetAPI.h"
#include <QByteArray>
#include <QFile>
#include <QFileInfo>
#include <QFileSystemWatcher>

namespace Asset
{

LocalAssetProvider::LocalAssetProvider(Foundation::Framework* framework) :
    framework_(framework)
{
    EventManagerPtr event_manager = framework_->GetEventManager();
    event_category_ = event_manager->QueryEventCategory("Asset");
}

LocalAssetProvider::~LocalAssetProvider()
{
}

const QString &LocalAssetProvider::Name()
{
    static const QString name("Local");
    
    return name;
}

bool LocalAssetProvider::IsValidRef(QString asset_id, QString asset_type)
{
    QString ref = asset_id.toStdString().c_str();
    ref = ref.trimmed();
    if (ref.length() == 0)
        return false;
    if (ref.startsWith("file://") || ref.startsWith("local://"))
        return true;

    if (!ref.contains("://")) // If the ref doesn't contain a protocol specifier (we do this simple check for it), try to directly find the given local file.
    {
        QString path = GetPathForAsset(ref, 0);
        return path.length() > 0;
    }
    else
        return false;
}

///\todo This function call is deprecated, and used in conjunction with the old code flow for asset requests. Use the AssetAPI and
/// AssetTransferPtr LocalAssetProvider::RequestAsset(QString assetRef, QString assetType) instead.
bool LocalAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
{
    // Complete any file uploads before processing any download requests. (a total hack, but this function will be removed in the future)
    CompletePendingFileUploads();

    if (!IsValidRef(asset_id.c_str(), asset_type.c_str()))
        return false;
    
    ServiceManagerPtr service_manager = framework_->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        return false;
    
    AssetModule::LogDebug("New local asset request for ref \"" + asset_id + "\"");
        
    // Strip file: trims asset provider id (f.ex. 'file://') and potential mesh name inside the file (everything after last slash)
    std::string filename = QString(asset_id.c_str()).trimmed().toStdString();
    if (filename.find("file://") != std::string::npos)
        filename = filename.substr(7);
    else if (filename.find("local://") != std::string::npos)
        filename = filename.substr(8);
    size_t lastSlash = filename.find_last_of('/');
    if (lastSlash != std::string::npos)
        filename = filename.substr(0, lastSlash);
    
    std::string assetpath = GetPathForAsset(filename.c_str(), 0).toStdString(); // Look up all known local file asset storages for this asset.
    if (assetpath.empty())
    {
        AssetModule::LogInfo("Failed to load local asset \"" + filename + "\"");
        return true;
    }
    
    boost::filesystem::path file_path(assetpath + "/" + filename);
    std::ifstream filestr(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
    if (filestr.good())
    {
        filestr.seekg(0, std::ios::end);
        uint length = filestr.tellg();
        filestr.seekg(0, std::ios::beg);
        
        if (length > 0)
        {
            RexAsset* new_asset = new RexAsset(asset_id, asset_type);
            Foundation::AssetInterfacePtr asset_ptr(new_asset);
            
            RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
            data.resize(length);
            filestr.read((char *)&data[0], length);
            filestr.close();
            
            // Store to memory cache only
            asset_service->StoreAsset(asset_ptr, false);
            // Send asset_ready event as delayed
            Events::AssetReady* event_data = new Events::AssetReady(asset_ptr->GetId(), asset_ptr->GetType(), asset_ptr, tag);
            framework_->GetEventManager()->SendDelayedEvent(event_category_, Events::ASSET_READY, EventDataPtr(event_data));

            AssetModule::LogDebug("OLD Asset path: Downloaded asset \"" + asset_id + "\" from file " + file_path.native_directory_string().c_str());

            return true;
        }
        else
            filestr.close();
    }
    
    AssetModule::LogInfo("Failed to load local asset \"" + filename + "\"");
    return true;
}

AssetTransferPtr LocalAssetProvider::RequestAsset(QString assetRef, QString assetType)
{
    AssetTransferPtr transfer = AssetTransferPtr(new IAssetTransfer);
    transfer->source.ref = assetRef.trimmed();
    transfer->assetType = assetType;

    pendingDownloads.push_back(transfer);

    return transfer;
}

QString LocalAssetProvider::GetPathForAsset(const QString &assetname, LocalAssetStoragePtr *storage)
{
    // Check first all subdirs without recursion, because recursion is potentially slow
    for(size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(assetname.toStdString().c_str(), false);
        if (path != "")
        {
            if (storage)
                *storage = storages[i];
            return path;
        }
    }

    for(size_t i = 0; i < storages.size(); ++i)
    {
        QString path = storages[i]->GetFullPathForAsset(assetname.toStdString().c_str(), true);
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

void LocalAssetProvider::AddStorageDirectory(const std::string &directory, const std::string &storageName, bool recursive)
{
    ///\todo Check first if the given directory exists as a storage, and don't add it as a duplicate if so.

    LocalAssetStoragePtr storage = LocalAssetStoragePtr(new LocalAssetStorage());
    storage->directory = directory.c_str();
    storage->name = storageName.c_str();
    storage->recursive = recursive;
    storage->provider = shared_from_this();
    storage->SetupWatcher(); // Start listening on file change notifications.
//    connect(storage->changeWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(FileChanged(QString)));
    connect(storage->changeWatcher, SIGNAL(fileChanged(QString)), this, SLOT(FileChanged(QString)));

    storages.push_back(storage);
}

std::vector<AssetStoragePtr> LocalAssetProvider::GetStorages() const
{
    std::vector<AssetStoragePtr> stores;
    for(size_t i = 0; i < storages.size(); ++i)
        stores.push_back(storages[i]);
    return stores;
}

IAssetUploadTransfer *LocalAssetProvider::UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName)
{
    LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(destination.get());
    if (!storage)
    {
        AssetModule::LogError("LocalAssetProvider::UploadAssetFromFile: Invalid destination asset storage type! Was not of type LocalAssetStorage!");
        return 0;
    }

    AssetUploadTransferPtr transfer = AssetUploadTransferPtr(new IAssetUploadTransfer());
    transfer->sourceFilename = filename;
    transfer->destinationName = assetName;
    transfer->destinationStorage = destination;

    pendingUploads.push_back(transfer);

    return transfer.get();
}

IAssetUploadTransfer *LocalAssetProvider::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    assert(data);
    if (!data)
    {
        AssetModule::LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Null source data pointer passed to function!");
        return 0;
    }

    LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage*>(destination.get());
    if (!storage)
    {
        AssetModule::LogError("LocalAssetProvider::UploadAssetFromFileInMemory: Invalid destination asset storage type! Was not of type LocalAssetStorage!");
        return 0;
    }

    AssetUploadTransferPtr transfer = AssetUploadTransferPtr(new IAssetUploadTransfer());
    transfer->sourceFilename = "";
    transfer->destinationName = assetName;
    transfer->destinationStorage = destination;
    transfer->assetData.insert(transfer->assetData.end(), data, data + numBytes);

    pendingUploads.push_back(transfer);

    return transfer.get();
}

void LocalAssetProvider::CompletePendingFileDownloads()
{
    while(pendingDownloads.size() > 0)
    {
        AssetTransferPtr transfer = pendingDownloads.back();
        pendingDownloads.pop_back();

        QString ref = transfer->source.ref;

        AssetModule::LogDebug("New local asset request: " + ref.toStdString());

        // Strip file: trims asset provider id (f.ex. 'file://') and potential mesh name inside the file (everything after last slash)
        if (ref.startsWith("file://"))
            ref = ref.mid(7);
        if (ref.startsWith("local://"))
            ref = ref.mid(8);

        int lastSlash = ref.lastIndexOf('/');
        if (lastSlash != -1)
            ref = ref.left(lastSlash);

        LocalAssetStoragePtr storage;
        QString path = GetPathForAsset(ref, &storage);
        if (path.isEmpty())
        {
            AssetModule::LogWarning("Failed to find local asset with filename \"" + ref.toStdString() + "\"!");
            continue;
        }
    
        QFileInfo file(GuaranteeTrailingSlash(path) + ref);
        QString absoluteFilename = file.absoluteFilePath();

        bool success = LoadFileToVector(absoluteFilename.toStdString().c_str(), transfer->rawAssetData);
        if (!success)
        {
            AssetModule::LogError("Failed to read asset data for asset \"" + ref.toStdString() + "\" from file \"" + absoluteFilename.toStdString() + "\"");
            continue;
        }
        
        // Tell the Asset API that this asset should not be cached into the asset cache, and instead the original filename should be used
        // as a disk source, rather than generating a cache file for it.
        transfer->SetCachingBehavior(false, absoluteFilename.toStdString().c_str());

        transfer->storage = storage;
        AssetModule::LogDebug("Downloaded asset \"" + ref.toStdString() + "\" from file " + absoluteFilename.toStdString());

        // Signal the Asset API that this asset is now successfully downloaded.
        framework_->Asset()->AssetTransferCompleted(transfer.get());
    }
}

void LocalAssetProvider::CompletePendingFileUploads()
{
    while(pendingUploads.size() > 0)
    {
        AssetUploadTransferPtr transfer = pendingUploads.back();
        pendingUploads.pop_back();

        LocalAssetStorage *storage = dynamic_cast<LocalAssetStorage *>(transfer->destinationStorage.lock().get());
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
        }
        else
            transfer->EmitTransferCompleted();
    }
}

void LocalAssetProvider::FileChanged(const QString &path)
{
    AssetModule::LogInfo(("File " + path + " changed.").toStdString());
}

} // ~Asset
