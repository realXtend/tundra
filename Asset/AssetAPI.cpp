// For conditions of distribution and use, see copyright notice in license.txt

#include "AssetAPI.h"
#include "Framework.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "IAssetStorage.h"
#include "AssetServiceInterface.h"
#include "IAssetProvider.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"
#include "EventManager.h"
#include "ResourceInterface.h"
#include "CoreException.h"
#include "IAssetTypeFactory.h"
#include "GenericAssetFactory.h"
#include "AssetCache_.h"
#include "Platform.h"
#include <QDir>

DEFINE_POCO_LOGGING_FUNCTIONS("Asset")

using namespace Foundation;

AssetAPI::AssetAPI(Foundation::Framework *owner)
:framework(owner)
{
    const char cDefaultAssetCachePath[] = "/assetcache";

    assetCache = new AssetCache((owner->GetPlatform()->GetApplicationDataDirectory() + cDefaultAssetCachePath).c_str());

    // The Asset API always understands at least this single built-in asset type "Binary".
    // You can use this type to request asset data as binary, without generating any kind of in-memory representation or loading for it.
    // Your module/component can then parse the content in a custom way.
    RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Binary")));
}

AssetAPI::~AssetAPI()
{
    delete assetCache;
}

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded(this);
}

void IAssetTransfer::EmitAssetDecoded()
{
    emit Decoded(this);
}

void IAssetTransfer::EmitAssetLoaded()
{
    emit Loaded(this);
}

std::vector<AssetProviderPtr> AssetAPI::GetAssetProviders() const
{
    ServiceManagerPtr service_manager = framework->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        throw Exception("Unagle to get AssetServiceInterface!");

    std::vector<AssetProviderPtr> providers = asset_service->Providers();
    return providers;
}

AssetStoragePtr AssetAPI::GetAssetStorage(const QString &name) const
{
    foreach(AssetProviderPtr provider, GetAssetProviders())
        foreach(AssetStoragePtr storage, provider->GetStorages())
            if (storage->Name() == name)
                return storage;

    return AssetStoragePtr();
}

std::vector<AssetStoragePtr> AssetAPI::GetAssetStorages() const
{
    std::vector<AssetStoragePtr> storages;

    std::vector<AssetProviderPtr> providers = GetAssetProviders();

    for(size_t i = 0; i < providers.size(); ++i)
    {
        std::vector<AssetStoragePtr> stores = providers[i]->GetStorages();
        storages.insert(storages.end(), stores.begin(), stores.end());
    }

    return storages;
}

AssetAPI::FileQueryResult AssetAPI::QueryFileLocation(QString sourceRef, QString baseDirectory, QString &outFilePath)
{
    sourceRef = sourceRef.trimmed();
    baseDirectory = GuaranteeTrailingSlash(baseDirectory.trimmed());
    outFilePath = "";

    // Remove both 'file://' and 'local://' specifiers in this lookup.
    if (sourceRef.startsWith("file://"))
        sourceRef = sourceRef.mid(7);
    if (sourceRef.startsWith("local://"))
        sourceRef = sourceRef.mid(8);

    if (sourceRef.contains("://")) // It's an external URL with a protocol specifier?
    {
        outFilePath = sourceRef;
        return FileQueryExternalFile;
    }

    if (QDir::isAbsolutePath(sourceRef)) // If the user specified an absolute path, don't look recursively at all, and ignore baseDirectory.
    {
        outFilePath = sourceRef; // This is where the file should be if it exists.
        if (QFile::exists(sourceRef))
            return FileQueryLocalFileFound;
        else
            return FileQueryLocalFileMissing;
    }

    // The user did not specify an URL with a protocol specifier, and he did not specify an absolute path, so it's a relative path.

    QString sourceFilename = ExtractFilenameFromAssetRef(sourceRef);

    if (!baseDirectory.isEmpty())
    {
        // The baseDirectory has the first priority for lookup.
        QString fullPath = RecursiveFindFile(baseDirectory, sourceFilename);
        if (!fullPath.isEmpty())
        {
            outFilePath = fullPath;
            return FileQueryLocalFileFound;
        }
    }

    // Do a recursive lookup through all local asset providers and the given base directory.
    ///\todo Implement this. Can't query the LocalAssetProviders here directly (wrong direction for dependency chain).

    return FileQueryLocalFileMissing;
}

QString AssetAPI::ExtractFilenameFromAssetRef(QString ref)
{
    using namespace std;

    // Try to find the local filename from the given string, e.g. "c:\data\my.mesh" -> "my.mesh", or "file://my.mesh" -> "my.mesh".
    QString s = ref.trimmed();
    int end = 0;
    end = max(end, s.lastIndexOf('/')+1);
    end = max(end, s.lastIndexOf('\\')+1);
    return s.mid(end);
}

QString AssetAPI::RecursiveFindFile(QString basePath, QString filename)
{
    basePath = basePath.trimmed();
    filename = ExtractFilenameFromAssetRef(filename.trimmed());

    QDir dir(GuaranteeTrailingSlash(basePath) + filename);
    if (boost::filesystem::exists(dir.absolutePath().toStdString()))
        return dir.absolutePath();

    try
    {
        boost::filesystem::recursive_directory_iterator iter(basePath.toStdString());
        boost::filesystem::recursive_directory_iterator end_iter;
        // Check the subdir
        for(; iter != end_iter; ++iter)
        {
            QDir dir(GuaranteeTrailingSlash(iter->path().string().c_str()) + filename);
            if (!fs::is_regular_file(iter->status()) && boost::filesystem::exists(dir.absolutePath().toStdString()))
                return dir.absolutePath();
        }
    }
    catch (...)
    {
    }

    return "";    
}

AssetPtr AssetAPI::CreateAssetFromFile(QString assetType, QString assetFile)
{
    AssetPtr asset = CreateNewAsset(assetType, assetFile);
    bool success = asset->LoadFromFile(assetFile);
    if (success)
        return asset;
    else
    {
        DeleteAsset(asset);
        return AssetPtr();
    }
}

void AssetAPI::DeleteAsset(AssetPtr asset)
{
    if (!asset.get())
        return;

    // Do an explicit unload of the asset before deletion (the dtor of each asset has to do unload as well, but this handles the cases where
    // some object left a dangling strong ref to an asset).
    asset->Unload();

    AssetMap::iterator iter = assets.find(asset->Name());
    if (iter == assets.end())
    {
        LogError("AssetAPI::DeleteAsset called on asset \"" + asset->Name().toStdString() + "\", which does not exist in AssetAPI!");
        return;
    }
    assets.erase(iter);
}

IAssetUploadTransfer *AssetAPI::UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName)
{
    if (!filename || strlen(filename) == 0)
        throw Exception("AssetAPI::UploadAssetFromFile failed! No source filename given!");

    if (!assetName || strlen(assetName) == 0)
        throw Exception("AssetAPI::UploadAssetFromFile failed! No destination asset name given!");

    if (!destination.get())
        throw Exception("AssetAPI::UploadAssetFromFile failed! The passed destination asset storage was null!");

    AssetProviderPtr provider = destination->provider.lock();
    if (!provider.get())
        throw Exception("AssetAPI::UploadAssetFromFile failed! The passed destination asset storage was null!");

    return provider->UploadAssetFromFile(filename, destination, assetName);
    /// \todo The pointer returned above can leak, if the provider doesn't guarantee deletion. Move the ownership to Asset API in a centralized manner.
}

IAssetUploadTransfer *AssetAPI::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    if (!data || numBytes == 0)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! Null source data passed!");

    if (!assetName || strlen(assetName) == 0)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! No destination asset name given!");

    if (!destination.get())
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The passed destination asset storage was null!");

    AssetProviderPtr provider = destination->provider.lock();
    if (!provider.get())
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The passed destination asset storage was null!");

    return provider->UploadAssetFromFileInMemory(data, numBytes, destination, assetName);
    /// \todo The pointer returned above can leak, if the provider doesn't guarantee deletion. Move the ownership to Asset API in a centralized manner.
}

void AssetAPI::DeleteAllAssets()
{
    while(assets.size() > 0)
        DeleteAsset(assets.begin()->second); // DeleteAsset removes the asset it is given to from the assets list, so this loop terminates.

    assets.clear();
    currentTransfers.clear();
}

AssetTransferPtr AssetAPI::RequestAsset(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    if (assetType.isEmpty())
        assetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());

    assetRef = assetRef.trimmed();
    if (assetRef.isEmpty())
    {
        // Removed this print - seems like a bad idea to print out this warning, since there are lots of scenes with null assetrefs.
        // Perhaps have a verbose log channel for these kinds of sanity checks.
//        LogError("AssetAPI::RequestAsset: Request by empty url \"\" of type \"" + assetType.toStdString() + " received!");
        return AssetTransferPtr();
    }

    // To optimize, we first check if there is an outstanding requests to the given asset. If so, we return that request. In effect, we never
    // have multiple transfers running to the same asset. (Important: This must occur before checking the assets map for whether we already have the asset in memory, since
    // an asset will be stored in the AssetMap when it has been downloaded, but it might not yet have all its dependencies loaded).
    AssetTransferMap::iterator iter = currentTransfers.find(assetRef);
    if (iter != currentTransfers.end())
    {
        AssetTransferPtr transfer = iter->second;

        // Check that the requested types were the same. Don't know what to do if they differ, so only print a warning if so.
        if (!assetType.isEmpty() && !transfer->assetType.isEmpty() && assetType != transfer->assetType)
            LogWarning("AssetAPI::RequestAsset: Asset \"" + assetRef.toStdString() + "\" first requested by type " + transfer->assetType.toStdString() + 
            ", but now requested by type " + assetType.toStdString() + ".");

        return transfer;
    }

    // Check if we've already downloaded this asset before and it already is loaded in the system. We never reload an asset we've downloaded before, unless the 
    // client explicitly forces so, or if we get a change notification signal from the source asset provider telling the asset was changed.
    AssetMap::iterator iter2 = assets.find(assetRef);
    if (iter2 != assets.end())
    {
        // Whenever the client requests an asset that was loaded before, we create a request for that asset nevertheless.
        // The idea is to have the code path run the same independent of whether the asset existed or had to be downloaded, i.e.
        // a request is always made, and the receiver writes only a single asynchronous code path for handling the asset.

        // The asset was already downloaded. Generate a 'virtual asset transfer' and return it to the client.
        AssetTransferPtr transfer = AssetTransferPtr(new IAssetTransfer());
        transfer->asset = iter2->second; // For 'normal' requests, the asset ptr is zero, but for these virtual requests, we can already fill the asset here.
        transfer->source.ref = assetRef;        
        transfer->assetType = assetType;
        transfer->provider = transfer->asset->GetAssetProvider();
        transfer->storage = transfer->asset->GetAssetStorage();

        readyTransfers.push_back(transfer); // There is no assetprovider that will "push" the AssetTransferCompleted call. We have to remember to do it ourselves.
        return transfer;
    }

    // Find the AssetProvider that will fulfill this request.
    AssetProviderPtr provider = GetProviderForAssetRef(assetRef, assetType);
    if (!provider.get())
    {
        LogError("AssetAPI::RequestAsset: Failed to find a provider for asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
        return AssetTransferPtr();
    }

    // Check if we can fetch the asset from the asset cache. If so, we do a immediately load the data in from the asset cache and don't go to any asset provider.
    QString assetFileInCache = assetCache->FindAsset(assetRef);
    AssetTransferPtr transfer;

    if (!assetFileInCache.isEmpty())
    {
        // The asset can be found from cache. Generate a 'virtual asset transfer' and return it to the client.
        transfer = AssetTransferPtr(new IAssetTransfer());
        bool success = LoadFileToVector(assetFileInCache.toStdString().c_str(), transfer->rawAssetData);
        if (!success)
        {
            LogError("AssetAPI::RequestAsset: Failed to load asset from cache!");
            return AssetTransferPtr();
        }
        transfer->source.ref = assetRef;        
        transfer->assetType = assetType;
        transfer->storage = AssetStorageWeakPtr(); // Note: Unfortunately when we load an asset from cache, we don't get the information about which storage it's supposed to come from.
        LogDebug("AssetAPI::RequestAsset: Loaded asset \"" + assetRef.toStdString() + "\" from disk cache instead of having to use asset provider."); 
        readyTransfers.push_back(transfer); // There is no assetprovider that will "push" the AssetTransferCompleted call. We have to remember to do it ourselves.
    }
    else // Can't find the asset in cache. Do a real request from the asset provider.
    {
        transfer = provider->RequestAsset(assetRef, assetType);
    }

    if (!transfer.get())
    {
        LogError("AssetAPI::RequestAsset: Failed to request asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
        return AssetTransferPtr();
    }
    transfer->provider = provider;

    // Store the newly allocated AssetTransfer internally, so that any duplicated requests to this asset will return the same request pointer,
    // so we'll avoid multiple downloads to the exact same asset.
    assert(currentTransfers.find(assetRef) == currentTransfers.end());
    currentTransfers[assetRef] = transfer;
    connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), this, SLOT(OnAssetLoaded(IAssetTransfer*)));
    return transfer;
}

AssetTransferPtr AssetAPI::RequestAsset(const AssetReference &ref)
{
    return RequestAsset(ref.ref);
}

AssetProviderPtr AssetAPI::GetProviderForAssetRef(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());

    std::vector<AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
        if (providers[i]->IsValidRef(assetRef, assetType))
            return providers[i];

    return AssetProviderPtr();
}

void AssetAPI::RegisterAssetTypeFactory(AssetTypeFactoryPtr factory)
{
    AssetTypeFactoryPtr existingFactory = GetAssetTypeFactory(factory->Type());
    if (existingFactory.get())
        return; ///\todo Log out warning.

    assert(factory->Type() == factory->Type().trimmed());

    assetTypeFactories.push_back(factory);
}

QString AssetAPI::GenerateUniqueAssetName(QString assetTypePrefix, QString assetNamePrefix)
{
    static unsigned long uniqueRunningAssetCounter = 1;

    assetTypePrefix = assetTypePrefix.trimmed();
    assetNamePrefix = assetNamePrefix.trimmed();

    if (assetTypePrefix.isEmpty())
        assetTypePrefix = "Asset";

    QString assetName;
    bool ok = false;
    while(!ok) // We loop until we manage to generate a single asset name that does not exist, incrementing a running counter at each iteration.
    {
        assetName = assetTypePrefix + "_" + assetNamePrefix + (assetNamePrefix.isEmpty() ? "" : "_") + QString::number(uniqueRunningAssetCounter++);
        if (!GetAsset(assetName).get())
            ok = true;
    }
    return assetName;
}

AssetPtr AssetAPI::CreateNewAsset(QString type, QString name)
{
    type = type.trimmed();
    name = name.trimmed();
    if (name.length() == 0)
    {
        LogError("AssetAPI:CreateNewAsset: Trying to create an asset with name=\"\"!");
        return AssetPtr();
    }

    AssetTypeFactoryPtr factory = GetAssetTypeFactory(type);
    if (!factory.get())
    {
        LogError("AssetAPI:CreateNewAsset: Cannot create asset of type \"" + type.toStdString() + "\", name: \"" + name.toStdString() + "\". No type factory registered for the type!");
        return AssetPtr();
    }
    AssetPtr asset = factory->CreateEmptyAsset(this, name.toStdString().c_str());
    if (!asset.get())
    {
        LogError("AssetAPI:CreateNewAsset: IAssetTypeFactory::CreateEmptyAsset(type \"" + type.toStdString() + "\", name: \"" + name.toStdString() + "\") failed to create asset!");
        return AssetPtr();
    }
    return asset;
}

AssetTypeFactoryPtr AssetAPI::GetAssetTypeFactory(QString typeName)
{
    for(size_t i = 0; i < assetTypeFactories.size(); ++i)
        if (assetTypeFactories[i]->Type().toLower() == typeName.toLower())
            return assetTypeFactories[i];

    return AssetTypeFactoryPtr();
}

AssetPtr AssetAPI::GetAsset(QString assetRef)
{
    AssetMap::iterator iter = assets.find(assetRef);
    if (iter != assets.end())
        return iter->second;
    return AssetPtr();
}

AssetPtr AssetAPI::GetAssetByHash(QString assetHash)
{
    ///\todo Implement.
    return AssetPtr();
}

void AssetAPI::Update()
{
    // Normally it is the AssetProvider's responsibility to call AssetTransferCompleted when a download finishes.
    // The 'readyTransfers' list contains all the asset transfers that don't have any AssetProvider serving them. These occur in two cases:
    // 1) A client requested an asset that was already loaded. In that case the request is not given to any assetprovider, but delayed in readyTransfers
    //    for one frame after which we just signal the asset to have been loaded.
    // 2) We found the asset from disk cache. No need to ask an assetprovider

    // Call AssetTransferCompleted manually for any asset that doesn't have an AssetProvider serving it. ("virtual transfers").
    for(size_t i = 0; i < readyTransfers.size(); ++i)
        AssetTransferCompleted(readyTransfers[i].get());
    readyTransfers.clear();
}

QString GuaranteeTrailingSlash(const QString &source)
{
    QString s = source.trimmed();
    if (s.isEmpty())
        return ""; // If user inputted "", output "" (can't output "/", since that would mean the root of the whole filesystem on linux)

    if (s[s.length()-1] != '/' && s[s.length()-1] != '\\')
        s = s + "/";

    return s;
}

void AssetAPI::AssetTransferCompleted(IAssetTransfer *transfer_)
{
    // At this point, the transfer can originate from several different things:
    // 1) It could be a real AssetTransfer from a real AssetProvider.
    // 2) It could be an AssetTransfer to an Asset that was already downloaded before, in which case transfer_->asset is already filled and loaded at this point.
    // 3) It could be an AssetTransfer that was fulfilled from the disk cache, in which case no AssetProvider was invoked to get here. (we used the readyTransfers queue for this).

    assert(transfer_);
    AssetTransferPtr transfer = transfer_->shared_from_this(); // Elevate to a SharedPtr immediately to keep at least one ref alive of this transfer for the duration of this function call.

    if (transfer->asset.get()) // This is a duplicated transfer to an asset that has already been previously loaded. Only signal that the asset's been loaded and finish.
    {
        transfer->EmitAssetDownloaded();
        transfer->EmitAssetDecoded();
        transfer->EmitAssetLoaded();
        AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
        if (iter != currentTransfers.end())
            currentTransfers.erase(iter);
        return;
    }

    // We should be tracking this transfer in an internal data structure.
    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    if (iter == currentTransfers.end())
        LogError("AssetAPI: Asset \"" + transfer->assetType.toStdString() + "\", name \"" + transfer->source.ref.toStdString() + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    // We've finished an asset data download, now create an actual instance of an asset of that type.
    transfer->asset = CreateNewAsset(transfer->assetType, transfer->source.ref);
    if (!transfer->asset.get())
    {
        LogError("AssetAPI: Failed to create new asset of type \"" + transfer->assetType.toStdString() + "\" and name \"" + transfer->source.ref.toStdString() + "\"");
        return;
    }

    // Save this asset to cache, and find out which file will represent a cached version of this asset.
    QString assetDiskSource = transfer->DiskSource(); // The asset provider may have specified an explicit filename to use as a disk source.
    if (transfer->CachingAllowed())
        assetDiskSource = assetCache->StoreAsset(&transfer->rawAssetData[0], transfer->rawAssetData.size(), transfer->source.ref, ""); ///\todo Specify the content hash.

    // Save for the asset the storage and provider it came from.
    transfer->asset->SetDiskSource(assetDiskSource.trimmed());
    transfer->asset->SetAssetStorage(transfer->storage.lock());
    transfer->asset->SetAssetProvider(transfer->provider.lock());

    bool success = transfer->asset->LoadFromFileInMemory(&transfer->rawAssetData[0], transfer->rawAssetData.size());
    if (!success)
    {
        LogError("AssetAPI: Failed to load asset of type \"" + transfer->assetType.toStdString() + "\" and name \"" + transfer->source.ref.toStdString() + "\" from asset data.");
        return;
    }

    // Remember the newly created asset in AssetAPI's internal data structure to allow clients to later fetch it without re-requesting it.
    AssetMap::iterator iter2 = assets.find(transfer->source.ref);
    if (iter2 != assets.end())
    {
        AssetPtr existing = iter2->second;
        LogWarning("AssetAPI: Overwriting a previously downloaded asset \"" + existing->Name().toStdString() + "\", type \"" + existing->Type().toStdString() + "\" with asset of same name!");
    }
    assets[transfer->source.ref] = transfer->asset;

    // If this asset depends on any other assets, we have to make asset requests for those assets as well (and all assets that they refer to, and so on).
    RequestAssetDependencies(transfer->asset);

    // Tell everyone this transfer has now been downloaded. Note that when this signal is fired, the asset dependencies may not yet be loaded.
    transfer->EmitAssetDownloaded();

    // If we don't have any outstanding dependencies, fire the Loaded signal for the asset as well.
    if (NumPendingDependencies(transfer->asset) == 0)
        AssetDependenciesCompleted(transfer);
}

void AssetAPI::AssetDependenciesCompleted(AssetTransferPtr transfer)
{
    // This asset transfer has finished - remove it from the internal list of ongoing transfers.
    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    if (iter != currentTransfers.end())
        currentTransfers.erase(iter);
    else // Even if we didn't know about this transfer, just print a warning and continue execution here nevertheless.
        LogError("AssetAPI: Asset \"" + transfer->assetType.toStdString() + "\", name \"" + transfer->source.ref.toStdString() + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    if (transfer->rawAssetData.size() == 0)
    {
        LogError("AssetAPI: Asset \"" + transfer->assetType.toStdString() + "\", name \"" + transfer->source.ref.toStdString() + "\" transfer finished: but data size was 0 bytes!");
        return;
    }

    ///\todo Specify the following flow better.
    transfer->EmitAssetDecoded();

    // This asset is now completely finished, and all its dependencies have been loaded.
    transfer->EmitAssetLoaded();
}

void AssetAPI::NotifyAssetDependenciesChanged(AssetPtr asset)
{
    /// Delete all old stored asset dependencies for this asset.
    RemoveAssetDependencies(asset->Name());

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString ref = refs[i].ref;
        if (ref.isEmpty())
            continue;

        // Remember this assetref for future lookup.
        assetDependencies.push_back(std::make_pair(asset->Name(), ref));
    }
}

void AssetAPI::RequestAssetDependencies(AssetPtr asset)
{
    // Make sure we have most up-to-date internal view of the asset dependencies.
    NotifyAssetDependenciesChanged(asset);

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString ref = refs[i].ref;
        if (ref.isEmpty())
            continue;

        AssetPtr existing = GetAsset(refs[i].ref);
        if (existing.get())
            asset->DependencyLoaded(existing);
        else // We don't have the given asset yet, request it.
        {
            LogInfo("Asset " + asset->ToString().toStdString() + " depends on asset " + ref.toStdString() + " which has not been loaded yet. Requesting..");
            RequestAsset(refs[i].ref);
        }
    }
}

void AssetAPI::RemoveAssetDependencies(QString asset)
{
    for(size_t i = 0; i < assetDependencies.size(); ++i)
        if (assetDependencies[i].first == asset)
        {
            assetDependencies.erase(assetDependencies.begin() + i);
            --i;
        }
}

std::vector<AssetPtr> AssetAPI::FindDependents(QString dependee)
{
    std::vector<AssetPtr> dependents;
    for(size_t i = 0; i < assetDependencies.size(); ++i)
        if (assetDependencies[i].second == dependee)
        {
            AssetMap::iterator iter = assets.find(assetDependencies[i].first);
            if (iter != assets.end())
                dependents.push_back(iter->second);
        }
    return dependents;
}

int AssetAPI::NumPendingDependencies(AssetPtr asset)
{
    int numDependencies = 0;

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString ref = refs[i].ref;
        if (ref.isEmpty())
            continue;

        AssetPtr existing = GetAsset(refs[i].ref);
        if (!existing.get())
            ++numDependencies;
    }

    return numDependencies;
}

void AssetAPI::OnAssetLoaded(IAssetTransfer *transfer)
{
    std::vector<AssetPtr> dependents = FindDependents(transfer->source.ref);
    for(size_t i = 0; i < dependents.size(); ++i)
    {
        AssetPtr asset = dependents[i];

        // Notify the asset that one of its dependencies has now been loaded in.
        asset->DependencyLoaded(transfer->asset);

        // Check if this dependency was the last one of the given asset's dependencies.
        AssetTransferMap::iterator iter = currentTransfers.find(asset->Name());
        if (iter != currentTransfers.end())
        {
            AssetTransferPtr transfer = iter->second;

            if (NumPendingDependencies(asset) == 0)
                AssetDependenciesCompleted(transfer);
        }
    }
}

bool LoadFileToVector(const char *filename, std::vector<u8> &dst)
{
    FILE *handle = fopen(filename, "rb");
    if (!handle)
    {
        LogError("Could not open file " + std::string(filename) + ".");
        return false;
    }

    fseek(handle, 0, SEEK_END);
    long numBytes = ftell(handle);
    if (numBytes == 0)
    {
        fclose(handle);
        return false;
    }

    fseek(handle, 0, SEEK_SET);
    dst.resize(numBytes);
    size_t numRead = fread(&dst[0], sizeof(u8), numBytes, handle);
    fclose(handle);

    return (long)numRead == numBytes;
}

namespace
{
    bool IsFileOfType(const QString &filename, const char **suffixes, int numSuffixes)
    {
        for(int i = 0;i < numSuffixes; ++i)
            if (filename.endsWith(suffixes[i]))
                return true;

        return false;
    }
}

QString GetResourceTypeFromResourceFileName(const char *name)
{
    QString file(name);
    file = file.trimmed();
    if (file.endsWith(".mesh"))
        return "OgreMesh";
    if (file.endsWith(".skeleton"))
        return "OgreSkeleton";
    if (file.endsWith(".material"))
        return "OgreMaterial";
    if (file.endsWith(".particle"))
        return "OgreParticle";

    const char *textureFileTypes[] = { ".jpg", ".png", ".tga", ".bmp", ".dds" };
    if (IsFileOfType(file, textureFileTypes, NUMELEMS(textureFileTypes)))
        return "Texture";

    const char *openAssImpFileTypes[] = { ".3d", ".b3d", ".dae", ".bvh", ".3ds", ".ase", ".obj", ".ply", ".dxf", 
        ".nff", ".smd", ".vta", ".mdl", ".md2", ".md3", ".mdc", ".md5mesh", ".x", ".q3o", ".q3s", ".raw", ".ac",
        ".stl", ".irrmesh", ".irr", ".off", ".ter", ".mdl", ".hmp", ".ms3d", ".lwo", ".lws", ".lxo", ".csm",
        ".ply", ".cob", ".scn" };

    if (IsFileOfType(file, openAssImpFileTypes, NUMELEMS(openAssImpFileTypes)))
        return "OgreMesh"; // We use the OgreMeshResource type for mesh files opened using the Open Asset Import Library.

    if (file.endsWith(".js") || file.endsWith(".py"))
        return "Script";

    if (file.endsWith(".ntf"))
        return "Terrain";

    // Unknown type.
    return "";

    // Note: There's a separate OgreImageTextureResource which isn't handled above.
}

bool CopyAssetFile(const char *sourceFile, const char *destFile)
{
    assert(sourceFile);
    assert(destFile);

    QFile asset_in(sourceFile);
    if (!asset_in.open(QFile::ReadOnly))
    {
        LogError("Could not open input asset file \"" + std::string(sourceFile) + "\"");
        return false;
    }

    QByteArray bytes = asset_in.readAll();
    asset_in.close();
    
    QFile asset_out(destFile);
    if (!asset_out.open(QFile::WriteOnly))
    {
        LogError("Could not open output asset file \"" + std::string(destFile) + "\"");
        return false;
    }

    asset_out.write(bytes);
    asset_out.close();
    
    return true;
}

bool SaveAssetFromMemoryToFile(const u8 *data, size_t numBytes, const char *destFile)
{
    assert(data);
    assert(destFile);

    QFile asset_out(destFile);
    if (!asset_out.open(QFile::WriteOnly))
    {
        LogError("Could not open output asset file \"" + std::string(destFile) + "\"");
        return false;
    }

    asset_out.write((const char *)data, numBytes);
    asset_out.close();
    
    return true;
}
