// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include <boost/thread.hpp>
#include <boost/algorithm/string.hpp>
#include <QList>
#include <boost/regex.hpp>
#include "MemoryLeakCheck.h"
#include "AssetAPI.h"
#include "Framework.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "IAssetStorage.h"
#include "IAssetProvider.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"
#include "CoreException.h"
#include "IAssetTypeFactory.h"
#include "IAssetUploadTransfer.h"
#include "GenericAssetFactory.h"
#include "AssetCache.h"
#include <QDir>
#include <QFileSystemWatcher>

AssetAPI::AssetAPI(bool isHeadless)
:assetCache(0),
diskSourceChangeWatcher(0),
isHeadless_(isHeadless)
{
    // The Asset API always understands at least this single built-in asset type "Binary".
    // You can use this type to request asset data as binary, without generating any kind of in-memory representation or loading for it.
    // Your module/component can then parse the content in a custom way.
    RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Binary")));
    isHeadless_ = isHeadless;
}

AssetAPI::~AssetAPI()
{
    delete assetCache;
    delete diskSourceChangeWatcher;
}

void AssetAPI::OpenAssetCache(QString directory)
{
    SAFE_DELETE(assetCache);
    SAFE_DELETE(diskSourceChangeWatcher);
    assetCache = new AssetCache(this, directory.toStdString().c_str());
    diskSourceChangeWatcher = new QFileSystemWatcher();
    connect(diskSourceChangeWatcher, SIGNAL(fileChanged(QString)), this, SLOT(OnAssetDiskSourceChanged(QString)));
}

std::vector<AssetProviderPtr> AssetAPI::GetAssetProviders() const
{
    return providers;
}

void AssetAPI::RegisterAssetProvider(AssetProviderPtr provider)
{
    for(uint i=0; i<providers.size(); ++i)
    {
        if (providers[i]->Name() == provider->Name())
        {
            LogWarning("AssetAPI::RegisterAssetProvider: Provider with name '" + provider->Name() + "' already registered.");
            return;
        }
    }
    providers.push_back(provider);
}

AssetStoragePtr AssetAPI::GetAssetStorage(const QString &name) const
{
    foreach(AssetProviderPtr provider, GetAssetProviders())
        foreach(AssetStoragePtr storage, provider->GetStorages())
            if (storage->Name() == name)
                return storage;
    return AssetStoragePtr();
}

AssetStoragePtr AssetAPI::DeserializeAssetStorageFromString(const QString &storage)
{
    for(size_t i = 0; i < providers.size(); ++i)
    {
        AssetStoragePtr assetStorage = providers[i]->TryDeserializeStorageFromString(storage);
        if (assetStorage)
            return assetStorage;
    }
    LogError("Failed to deserialize asset storage from string \"" + storage + "\"!");
    return AssetStoragePtr();
}

AssetStoragePtr AssetAPI::GetDefaultAssetStorage() const
{
    AssetStoragePtr defStorage = defaultStorage.lock();
    if (defStorage)
        return defStorage;

    // If no default storage set, return the first one on the list.
    std::vector<AssetStoragePtr> storages = GetAssetStorages();
    if (storages.size() > 0)
        return storages[0];

    return AssetStoragePtr();
}

void AssetAPI::SetDefaultAssetStorage(const AssetStoragePtr &storage)
{
    defaultStorage = storage;
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

AssetAPI::FileQueryResult AssetAPI::ResolveLocalAssetPath(QString ref, QString baseDirectoryContext, QString &outFilePath, QString *subAssetName)
{
    // Make sure relative paths are turned into local paths.
    QString refLocal = ResolveAssetRef("local://", ref);

    QString pathFilename;
    QString protocol;
    AssetRefType sourceRefType = ParseAssetRef(refLocal, &protocol, 0, 0, 0, &pathFilename, 0, 0, subAssetName);
    if (sourceRefType == AssetRefInvalid || sourceRefType == AssetRefExternalUrl || sourceRefType == AssetRefNamedStorage)
    {
        // Failed to resolve path, it is not local.
        outFilePath = refLocal; // Return the path that was tried.
        return FileQueryExternalFile;
    }
    else if (sourceRefType == AssetRefLocalPath)
    {
        outFilePath = pathFilename;
        return QFile::exists(outFilePath) ? FileQueryLocalFileFound : FileQueryLocalFileMissing;
    }
    else if (sourceRefType == AssetRefLocalUrl)
    {
        outFilePath = pathFilename;
        if (QFile::exists(outFilePath))
            return FileQueryLocalFileFound;

        outFilePath = RecursiveFindFile(baseDirectoryContext, pathFilename);
        if (!outFilePath.isEmpty())
            return FileQueryLocalFileFound;

        ///\todo Query all local storages for the file.
        ///\todo Can't currently query the LocalAssetProviders here directly (wrong direction for dependency chain).

        outFilePath = ref;
        return FileQueryLocalFileMissing;
    }
    else
    {
        // Unknown reference type.
        outFilePath = ref;
        return FileQueryExternalFile;
    }
}

AssetAPI::AssetRefType AssetAPI::ParseAssetRef(QString assetRef, QString *outProtocolPart, QString *outNamedStorage, QString *outProtocol_Path, 
                                               QString *outPath_Filename_SubAssetName, QString *outPath_Filename, QString *outPath, 
                                               QString *outFilename, QString *outSubAssetName, QString *outFullRef)
{
    if (outProtocolPart) *outProtocolPart = "";
    if (outNamedStorage) *outNamedStorage = "";
    if (outProtocol_Path) *outProtocol_Path = "";
    if (outPath_Filename_SubAssetName) *outPath_Filename_SubAssetName = "";
    if (outPath_Filename) *outPath_Filename = "";
    if (outPath) *outPath = "";
    if (outFilename) *outFilename = "";
    if (outSubAssetName) *outSubAssetName = "";
    if (outFullRef) *outFullRef = "";

    /* Examples of asset refs:

     a1) local://asset.sfx                                      AssetRefType = AssetRefLocalUrl.
         file:///unix/file/path/asset.sfx
         file://C:/windows/file/path/asset.sfx

     a2) http://server.com/asset.sfx                            AssetRefType = AssetRefExternalUrl.
         someotherprotocol://some/path/specifier/asset.sfx

     b0) special case: www.server.com/asset.png                 AssetRefType = AssetRefExternalUrl with 'http' hardcoded as protocol.
         As customary with web browsers, people expect to be able to write just www.server.com/asset.png when they mean a network URL.
         We detect this case as a string that starts with 'www.'.

     b1) /unix/absolute/path/asset.sfx                          AssetRefType = AssetRefLocalPath.

     b2) X:/windows/forwardslash/path/asset.sfx                 AssetRefType = AssetRefLocalPath.
         X:\windows\backslash\path\asset.sfx
         X:\windows/mixedupslash\path/asset.sfx

     b3) namedstorage:asset.sfx                                 AssetRefType = AssetRefNamedStorage.

     b4) asset.sfx                                              AssetRefType = AssetRefRelativePath.
         ./asset.sfx
         .\asset.sfx
         relative/path/asset.sfx
         ./relative/path/asset.sfx
         ../relative/path/asset.sfx
         
        Each category can have an optional extra subasset specifier, separated with a comma. Examples:

             asset.sfx,subAssetName.sfx2
             asset.sfx,subAssetNameWithoutSuffix
             asset.sfx, "subAssetName as a string with spaces"
             http://server.com/asset.sfx,subAssetName.sfx2
    */

    assetRef = assetRef.trimmed();
    assetRef.replace("\\", "/"); // Normalize all path separators to use forward slashes.

    using namespace boost;
    using namespace std;

    wstring ref = QStringToWString(assetRef);

    wregex expression1(L"(.*?)://(.*)"); // a): protocolSpecifier://pathToAsset
    wregex expression2(L"([A-Za-z]:[/\\\\].*?)"); // b2): X:\windowsPathToAsset or X:/windowsPathToAsset
    wregex expression3(L"(.*?):(.*)"); // b3): X:\windowsPathToAsset or X:/windowsPathToAsset
    wsmatch what;
    wstring fullPath; // Contains the url without the "protocolPart://" prefix.
    AssetRefType refType = AssetRefInvalid;
    if (regex_match(ref, what, expression1)) // Is ref of type 'a)' above?
    {
        QString protocol = WStringToQString(what[1].str());
        if (protocol.compare("local", Qt::CaseInsensitive) == 0 || protocol.compare("file", Qt::CaseInsensitive) == 0)
            refType = AssetRefLocalUrl;
        else
            refType = AssetRefExternalUrl;
        if (outProtocolPart)
            *outProtocolPart = protocol;

        fullPath = what[2].str();
        if (outProtocol_Path) // Partially save the beginning of the protocol & path part. This will be completed below with the full path.
            *outProtocol_Path = protocol + "://";

        if (outFullRef)
            *outFullRef = protocol.toLower() + "://";
    }
    else if (assetRef.startsWith("www.", Qt::CaseInsensitive)) // Ref is of type 'b0)'?
    {
        refType = AssetRefExternalUrl;
        if (outProtocolPart)
            *outProtocolPart = "http";
        fullPath = ref;

        if (outProtocol_Path) // Partially save the beginning of the protocol & path part. This will be completed below with the full path.
            *outProtocol_Path = "http://";
        if (outFullRef)
            *outFullRef = "http://";
    }
    else if (assetRef.startsWith("/")) // Is ref of type 'b1)'?
    {
        refType = AssetRefLocalPath;
        fullPath = ref;
    }
    else if (regex_match(ref, what, expression2)) // b2)
    {
        refType = AssetRefLocalPath;
        fullPath = ref;
    }
    else if (regex_match(ref, what, expression3)) // b3)
    {
        refType = AssetRefNamedStorage;
        QString storage = WStringToQString(what[1].str()).trimmed();
        if (outNamedStorage)
            *outNamedStorage = storage;
        fullPath = what[2].str();

        if (outProtocol_Path)
            *outProtocol_Path = storage + ":";
        if (outFullRef)
            *outFullRef = storage + ":";
    }
    else // We assume it must be of type b4).
    {
        refType = AssetRefRelativePath;
        fullPath = ref;
    }

    // After the above check, we are left with a fullPath reference that can only contain three parts: directory, local name and subAssetName.
    // The protocol specifier or named storage part has been stripped off.
    if (outPath_Filename_SubAssetName)
        *outPath_Filename_SubAssetName = WStringToQString(fullPath).trimmed();

    // Parse subAssetName if it exists.
    QString subAssetName = "";
    wregex expression4(L"(.*?)\\s*,\\s*\"?\\s*(.*?)\\s*\"?\\s*"); // assetRef, "subAssetName". Note: this regex does not parse badly matched '"' signs, but it's a minor issue. (e.g. 'assetRef, ""jeejee' is incorrectly accepted) .
    if (regex_match(fullPath, what, expression4))
    {
        wstring assetRef = what[1].str();
        subAssetName = WStringToQString(what[2].str()).trimmed();
        if (outSubAssetName)
            *outSubAssetName = subAssetName;
        fullPath = assetRef; // Remove the subAssetName from the asset ref so that the parsing can continue without the subAssetName in it.
    }

    QString fullPathRef = WStringToQString(fullPath);
    if (outPath_Filename) 
        *outPath_Filename = fullPathRef;

    // Now the only thing that is left is to split the base filename and the path for the asset.
    int lastPeriodIndex = fullPathRef.lastIndexOf(".");
    int directorySeparatorIndex = fullPathRef.lastIndexOf("/"); 
    if (lastPeriodIndex == -1 || lastPeriodIndex < directorySeparatorIndex)
        directorySeparatorIndex = fullPathRef.length()-1;

    QString path = GuaranteeTrailingSlash(fullPathRef.left(directorySeparatorIndex+1).trimmed());
    if (outPath)
        *outPath = path;
    if (outProtocol_Path)
        *outProtocol_Path += path;
    if (outFilename)
        *outFilename = fullPathRef.mid(directorySeparatorIndex+1);
    if (outFullRef)
    {
        *outFullRef += fullPathRef;
        if (!subAssetName.isEmpty())
            if (subAssetName.contains(' '))
                *outFullRef += ", \"" + subAssetName + "\"";
            else
                *outFullRef += ", " + subAssetName;
    }

    return refType;
}

QString AssetAPI::ExtractFilenameFromAssetRef(QString ref)
{
    QString filename;
    ParseAssetRef(ref, 0, 0, 0, 0, 0, 0, &filename, 0);
    return filename;
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
    catch(...)
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
        ForgetAsset(asset, false);
        return AssetPtr();
    }
}

void AssetAPI::ForgetAsset(QString assetRef, bool removeDiskSource)
{
    AssetPtr asset = GetAsset(assetRef);
    if (asset.get())
        ForgetAsset(asset, removeDiskSource);
}

void AssetAPI::ForgetAsset(AssetPtr asset, bool removeDiskSource)
{
    if (!asset)
        return;

    emit AssetAboutToBeRemoved(asset);

    // If we are supposed to remove the cached (or original for local assets) version of the asset, do so.
    if (removeDiskSource && !asset->DiskSource().isEmpty())
    {
        emit DiskSourceAboutToBeRemoved(asset);
        // Remove disk watcher before deleting the file. Otherwise we get tons of spam and not wanted reload tries.
        if (diskSourceChangeWatcher)
            diskSourceChangeWatcher->removePath(asset->DiskSource());
        assetCache->DeleteAsset(asset->Name());
        asset->SetDiskSource("");
    }

    // Do an explicit unload of the asset before deletion (the dtor of each asset has to do unload as well, but this handles the cases where
    // some object left a dangling strong ref to an asset).
    asset->Unload();

    AssetMap::iterator iter = assets.find(asset->Name());
    if (iter == assets.end())
    {
        LogError("AssetAPI::ForgetAsset called on asset \"" + asset->Name() + "\", which does not exist in AssetAPI!");
        return;
    }
    if (diskSourceChangeWatcher && !asset->DiskSource().isEmpty())
        diskSourceChangeWatcher->removePath(asset->DiskSource());
    assets.erase(iter);
}

void AssetAPI::DeleteAssetFromStorage(QString assetRef)
{
    AssetPtr asset = GetAsset(assetRef);

    AssetProviderPtr provider = (asset.get() ? asset->GetAssetProvider() : AssetProviderPtr());
    if (!provider)
        provider = GetProviderForAssetRef(assetRef); // If the actual AssetPtr didn't specify the originating provider, try to guess it from the assetRef string.

    // We're erasing the asset from the storage, so also clean it from memory and disk source to avoid any leftovers from remaining in the system.
    if (asset)
        ForgetAsset(asset, true);

    if (!provider)
    {
        LogError("AssetAPI::DeleteAssetFromStorage called on asset \"" + assetRef + "\", but the originating provider was not set!");
        // Remove this asset from memory and from the disk source, the best we can do for it.
        return;
    }

    provider->DeleteAssetFromStorage(assetRef);
}

AssetUploadTransferPtr AssetAPI::UploadAssetFromFile(const QString &filename, const QString &storageName, const QString &assetName)
{
    QFile file(filename);
    if (!file.exists())
    {
        LogError("AssetAPI::UploadAssetFromFile failed! File location not valid for " + filename);
        return AssetUploadTransferPtr();
    }
    QString newAssetName = assetName;
    if (newAssetName.isEmpty())
        newAssetName = file.fileName().split("/").last();
    AssetStoragePtr storage = GetAssetStorage(storageName);
    if (!storage.get())
    {
        LogError("AssetAPI::UploadAssetFromFile failed! No storage found with name " + storageName + "! Please add a storage with this name.");
        return AssetUploadTransferPtr();
    }

    // Protect crashes when as this function is called from scripts!
    AssetUploadTransferPtr transfer;
    try
    {
        transfer = UploadAssetFromFile(filename.toStdString().c_str(), storage, newAssetName.toStdString().c_str());
    }
    catch(Exception &e)
    {
        LogError(std::string(e.what()));
    }
    catch(...)
    {
        LogError("AssetAPI::UploadAssetFromFile failed, unknown exception!");
    }
    return transfer;
}

AssetUploadTransferPtr AssetAPI::UploadAssetFromFile(const char *filename, AssetStoragePtr destination, const char *assetName)
{
    if (!filename || strlen(filename) == 0)
        throw Exception("AssetAPI::UploadAssetFromFile failed! No source filename given!");

    if (!assetName || strlen(assetName) == 0)
        throw Exception("AssetAPI::UploadAssetFromFile failed! No destination asset name given!");

    if (!destination)
        throw Exception("AssetAPI::UploadAssetFromFile failed! The passed destination asset storage was null!");

    AssetProviderPtr provider = destination->provider.lock();
    if (!provider)
        throw Exception("AssetAPI::UploadAssetFromFile failed! The provider pointer of the passed destination asset storage was null!");

    std::vector<u8> data;
    bool success = LoadFileToVector(filename, data);
    if (!success)
        throw Exception("AssetAPI::UploadAssetFromFile failed! Could not load file to a data vector.");
    if (data.size() == 0)
        throw Exception("AssetAPI::UploadAssetFromFile failed! Loaded file to data vector but size is zero.");

    return UploadAssetFromFileInMemory(&data[0], data.size(), destination, assetName);
}

AssetUploadTransferPtr AssetAPI::UploadAssetFromFileInMemory(const QByteArray &data, const QString &storageName, const QString &assetName)
{
    if (data.isEmpty() || data.isNull())
    {
        LogError("AssetAPI::UploadAssetFromFileInMemory failed! QByteArray data is empty and/or null for " + assetName + " asset upload request.");
        return AssetUploadTransferPtr();
    }
    AssetStoragePtr storage = GetAssetStorage(storageName);
    if (!storage.get())
    {
        LogError("AssetAPI::UploadAssetFromFileInMemory failed! No storage found with name " + storageName + "! Please add a storage with this name.");
        return AssetUploadTransferPtr();
    }

    // Protect crashes when as this function is called from scripts!
    AssetUploadTransferPtr transfer;
    try
    {
        transfer = UploadAssetFromFileInMemory((const u8*)data.constData(), data.size(), storage, assetName.toStdString().c_str());
    }
    catch(Exception &e)
    {
        LogError(std::string(e.what()));
    }
    catch(...)
    {
        LogError("AssetAPI::UploadAssetFromFileInMemory failed, unknown exception!");
    }
    return transfer;
}

AssetUploadTransferPtr AssetAPI::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const char *assetName)
{
    if (!data || numBytes == 0)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! Null source data passed!");

    if (!assetName || strlen(assetName) == 0)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! No destination asset name given!");

    if (!destination)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The passed destination asset storage was null!");

    AssetProviderPtr provider = destination->provider.lock();
    if (!provider)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The provider pointer of the passed destination asset storage was null!");

    AssetUploadTransferPtr transfer = provider->UploadAssetFromFileInMemory(data, numBytes, destination, assetName);
    if (transfer)
        currentUploadTransfers[transfer->destinationStorage.lock()->GetFullAssetURL(assetName)] = transfer;

    return transfer;
}

void AssetAPI::ForgetAllAssets()
{
    while(assets.size() > 0)
        ForgetAsset(assets.begin()->second, false); // ForgetAsset removes the asset it is given to from the assets list, so this loop terminates.

    assets.clear();
    currentTransfers.clear();
}

std::vector<AssetTransferPtr> AssetAPI::PendingTransfers()
{
    std::vector<AssetTransferPtr> transfers;
    for(AssetTransferMap::iterator iter = currentTransfers.begin(); iter != currentTransfers.end(); ++iter)
        transfers.push_back(iter->second);

    transfers.insert(transfers.end(), readyTransfers.begin(), readyTransfers.end());
    return transfers;
}

AssetTransferPtr AssetAPI::GetPendingTransfer(QString assetRef)
{
    AssetTransferMap::iterator iter = currentTransfers.find(assetRef);
    if (iter != currentTransfers.end())
        return iter->second;
    for(size_t i = 0; i < readyTransfers.size(); ++i)
        if (readyTransfers[i]->source.ref == assetRef)
            return readyTransfers[i];

    return AssetTransferPtr();
}

AssetTransferPtr AssetAPI::RequestAsset(QString assetRef, QString assetType)
{
    // Turn named storage (and default storage) specifiers to absolute specifiers.
    assetRef = ResolveAssetRef("", assetRef);
    if (assetRef.isEmpty())
        return AssetTransferPtr();

    assetType = assetType.trimmed();
    if (assetType.isEmpty())
    {
        QString assetFilename;
        ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, &assetFilename);
        assetType = GetResourceTypeFromAssetRef(assetFilename);
    }

    // To optimize, we first check if there is an outstanding request to the given asset. If so, we return that request. In effect, we never
    // have multiple transfers running to the same asset. (Important: This must occur before checking the assets map for whether we already have the asset in memory, since
    // an asset will be stored in the AssetMap when it has been downloaded, but it might not yet have all its dependencies loaded).
    AssetTransferMap::iterator iter = currentTransfers.find(assetRef);
    if (iter != currentTransfers.end())
    {
        AssetTransferPtr transfer = iter->second;

        // Check that the requested types were the same. Don't know what to do if they differ, so only print a warning if so.
        if (!assetType.isEmpty() && !transfer->assetType.isEmpty() && assetType != transfer->assetType)
            LogWarning("AssetAPI::RequestAsset: Asset \"" + assetRef + "\" first requested by type " + transfer->assetType + 
            ", but now requested by type " + assetType + ".");

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

    // See if there is an asset upload that should block this download. If the same asset is being uploaded and downloaded simultaneously, make the download
    // wait until the upload completes.
    if (currentUploadTransfers.find(assetRef) != currentUploadTransfers.end())
    {
        LogDebug("The download of asset \"" + assetRef + "\" needs to wait, since the same asset is being uploaded at the moment.");
        PendingDownloadRequest pendingRequest;
        pendingRequest.assetRef = assetRef;
        pendingRequest.assetType = assetType;
        pendingRequest.transfer = AssetTransferPtr(new IAssetTransfer);

        pendingDownloadRequests[assetRef] = pendingRequest;
        return pendingRequest.transfer; ///\bug Problem. When we return this structure, the client will connect to this.
    }

    // Find the AssetProvider that will fulfill this request.
    AssetProviderPtr provider = GetProviderForAssetRef(assetRef, assetType);
    if (!provider)
    {
        LogError("AssetAPI::RequestAsset: Failed to find a provider for asset \"" + assetRef + "\", type: \"" + assetType + "\"");
        return AssetTransferPtr();
    }

    // Check if we can fetch the asset from the asset cache. If so, we do a immediately load the data in from the asset cache and don't go to any asset provider.
    QString assetFileInCache = assetCache->FindInCache(assetRef);
    AssetTransferPtr transfer;

    if (!assetFileInCache.isEmpty())
    {
        // The asset can be found from cache. Generate a 'virtual asset transfer' and return it to the client.
        transfer = AssetTransferPtr(new IAssetTransfer());
        bool success = LoadFileToVector(assetFileInCache.toStdString().c_str(), transfer->rawAssetData);
        if (!success)
        {
            LogError("AssetAPI::RequestAsset: Failed to load asset \"" + assetFileInCache + "\" from cache!");
            return AssetTransferPtr();
        }
        transfer->source.ref = assetRef;
        transfer->assetType = assetType;
        transfer->storage = AssetStorageWeakPtr(); // Note: Unfortunately when we load an asset from cache, we don't get the information about which storage it's supposed to come from.
        transfer->provider = provider;
        transfer->SetCachingBehavior(false, assetFileInCache);
        LogDebug("AssetAPI::RequestAsset: Loaded asset \"" + assetRef + "\" from disk cache instead of having to use asset provider."); 
        readyTransfers.push_back(transfer); // There is no assetprovider that will "push" the AssetTransferCompleted call. We have to remember to do it ourselves.
    }
    else // Can't find the asset in cache. Do a real request from the asset provider.
    {
        transfer = provider->RequestAsset(assetRef, assetType);
    }

    if (!transfer)
    {
        LogError("AssetAPI::RequestAsset: Failed to request asset \"" + assetRef + "\", type: \"" + assetType + "\"");
        return AssetTransferPtr();
    }
    transfer->provider = provider;

    // Store the newly allocated AssetTransfer internally, so that any duplicated requests to this asset will return the same request pointer,
    // so we'll avoid multiple downloads to the exact same asset.
    assert(currentTransfers.find(assetRef) == currentTransfers.end());
    currentTransfers[assetRef] = transfer;
    connect(transfer.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAssetLoaded(AssetPtr)));
    return transfer;
}

AssetTransferPtr AssetAPI::RequestAsset(const AssetReference &ref)
{
    return RequestAsset(ref.ref, ref.type);
}

AssetProviderPtr AssetAPI::GetProviderForAssetRef(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromAssetRef(assetRef.toLower().toStdString().c_str());

    // If the assetRef is by local filename without a reference to a provider or storage, use the default asset storage in the system for this assetRef.
    QString namedStorage;
    AssetRefType assetRefType = ParseAssetRef(assetRef, 0, &namedStorage);
    if (assetRefType == AssetRefRelativePath)
    {
        AssetStoragePtr defaultStorage = GetDefaultAssetStorage();
        AssetProviderPtr defaultProvider = (defaultStorage ? defaultStorage->provider.lock() : AssetProviderPtr());
        if (defaultProvider)
            return defaultProvider;
        // If no default provider, intentionally fall through to lookup each asset provider in turn.
    }
    else if (assetRefType == AssetRefNamedStorage) // The asset ref explicitly points to a named storage. Use the provider for that storage.
    {
        AssetStoragePtr storage = GetAssetStorage(namedStorage);
        AssetProviderPtr provider = (storage ? storage->provider.lock() : AssetProviderPtr());
        return provider;
    }

    std::vector<AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
        if (providers[i]->IsValidRef(assetRef, assetType))
            return providers[i];

    return AssetProviderPtr();
}

QString AssetAPI::ResolveAssetRef(QString context, QString assetRef)
{
    context = context.trimmed();

    // If the assetRef is by local filename without a reference to a provider or storage, use the default asset storage in the system for this assetRef.
    QString assetPath;
    QString namedStorage;
    AssetRefType assetRefType = ParseAssetRef(assetRef, 0, &namedStorage, 0, &assetPath);
    switch(assetRefType)
    {
    case AssetRefLocalPath: // Absolute path like "C:\myassets\texture.png".
    case AssetRefLocalUrl: // Local path using "local://", like "local://file.mesh".
    case AssetRefExternalUrl: // External path using an URL specifier, like "http://server.com/file.mesh" or "someProtocol://server.com/asset.dat".
        return assetRef;
    case AssetRefRelativePath:
        {
            if (context.isEmpty())
            {
                AssetStoragePtr defaultStorage = GetDefaultAssetStorage();
                if (!defaultStorage)
                    return assetRef; // Failed to find the provider, just use the ref as it was, and hope. (It might still be satisfied by the local storage).
                return defaultStorage->GetFullAssetURL(assetRef);
            }
            else
            {
                // Join the context to form a full url, e.g. context: "http://myserver.com/path/myasset.material", ref: "texture.png" returns "http://myserver.com/path/texture.png".
                QString contextPath;
                QString contextNamedStorage;
                QString contextProtocolSpecifier;
                AssetRefType contextRefType = ParseAssetRef(context, &contextProtocolSpecifier, &contextNamedStorage, 0, 0, 0, &contextPath);
                if (contextRefType == AssetRefRelativePath || contextRefType == AssetRefInvalid)
                {
                    LogError("Asset ref context \"" + contextPath + "\" is a relative path and cannot be used as a context for lookup for ref \"" + assetRef + "\"!");
                    return assetRef; // Return at least something.
                }
                QString newAssetRef;
                if (!contextNamedStorage.isEmpty())
                    newAssetRef += contextNamedStorage + ":";
                else if (!contextProtocolSpecifier.isEmpty())
                    newAssetRef += contextProtocolSpecifier + "://";
                newAssetRef += QDir::cleanPath(contextPath + assetPath);
                return newAssetRef;
            }
        }
        break;
    case AssetRefNamedStorage: // The asset ref explicitly points to a named storage. Use the provider for that storage.
        {
            AssetStoragePtr storage = GetAssetStorage(namedStorage);
            if (!storage)
                return assetRef; // Failed to find the provider, just use the ref as it was, and hope.
            return storage->GetFullAssetURL(assetPath);
        }
        break;
    }
    assert(false);
    return assetRef;
}

void AssetAPI::RegisterAssetTypeFactory(AssetTypeFactoryPtr factory)
{
    AssetTypeFactoryPtr existingFactory = GetAssetTypeFactory(factory->Type());
    if (existingFactory)
    {
        LogWarning("AssetAPI::RegisterAssetTypeFactory: Factory with type '" + factory->Type() + "' already registered.");
        return;
    }

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
    // We loop until we manage to generate a single asset name that does not exist, incrementing a running counter at each iteration.
    for(int i = 0; i < 10000; ++i) // The intent is to loop 'infinitely' until a name is found, but do an artificial limit to avoid voodoo bugs.
    {
        assetName = assetTypePrefix + "_" + assetNamePrefix + (assetNamePrefix.isEmpty() ? "" : "_") + QString::number(uniqueRunningAssetCounter++);
        if (!GetAsset(assetName))
            return assetName;
    }
    assert(false);
    throw Exception("GenerateUniqueAssetName failed!");
}

QString AssetAPI::GenerateTemporaryNonexistingAssetFilename(QString filenameSuffix)
{
    static unsigned long uniqueRunningFilenameCounter = 1;

    // Create this file path into the cache dir to avoid
    // windows non-admin users having no write permission to the run folder
    QDir cacheDir(assetCache->GetCacheDirectory());
    if (cacheDir.exists())
    {
        QString filename;
        // We loop until we manage to generate a single filename that does not exist, incrementing a running counter at each iteration.
        for(int i = 0; i < 10000; ++i) // The intent is to loop 'infinitely' until a name is found, but do an artificial limit to avoid voodoo bugs.
        {
            filename = cacheDir.absoluteFilePath("temporary_" + QString::number(uniqueRunningFilenameCounter++) + "_" + SanitateAssetRefForCache(filenameSuffix.trimmed()));
            if (!QFile::exists(filename))
                return filename;
        }
    }
    assert(false);
    throw Exception("GenerateTemporaryNonexistingAssetFilename failed!");
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
    if (!factory)
    {
        LogError("AssetAPI:CreateNewAsset: Cannot create asset of type \"" + type + "\", name: \"" + name + "\". No type factory registered for the type!");
        return AssetPtr();
    }
    AssetPtr asset = factory->CreateEmptyAsset(this, name.toStdString().c_str());
    if (!asset)
    {
        LogError("AssetAPI:CreateNewAsset: IAssetTypeFactory::CreateEmptyAsset(type \"" + type + "\", name: \"" + name + "\") failed to create asset!");
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
    // Normalize and resolve the lookup of the given asset.
    assetRef = ResolveAssetRef("", assetRef);

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

void AssetAPI::Update(f64 frametime)
{
    for(size_t i = 0; i < providers.size(); ++i)
        providers[i]->Update(frametime);

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
//    LogDebug("Transfer of asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" succeeded.");

    if (transfer->asset) // This is a duplicated transfer to an asset that has already been previously loaded. Only signal that the asset's been loaded and finish.
    {
        transfer->EmitAssetDownloaded();
//        transfer->asset->EmitDecoded
        transfer->EmitAssetDecoded();
        transfer->asset->EmitLoaded();
        transfer->EmitAssetLoaded();
        pendingDownloadRequests.erase(transfer->source.ref);
        currentTransfers.erase(transfer->source.ref);

        return;
    }

    // We should be tracking this transfer in an internal data structure.
    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    if (iter == currentTransfers.end())
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    // We've finished an asset data download, now create an actual instance of an asset of that type.
    transfer->asset = CreateNewAsset(transfer->assetType, transfer->source.ref);
    if (!transfer->asset)
    {
        QString error("AssetAPI: Failed to create new asset of type \"" + transfer->assetType + "\" and name \"" + transfer->source.ref + "\"");
        LogError(error);
        transfer->EmitAssetFailed(error);
        return;
    }

    // Save this asset to cache, and find out which file will represent a cached version of this asset.
    QString assetDiskSource = transfer->DiskSource(); // The asset provider may have specified an explicit filename to use as a disk source.
    if (transfer->CachingAllowed() && transfer->rawAssetData.size() > 0)
        assetDiskSource = assetCache->StoreAsset(&transfer->rawAssetData[0], transfer->rawAssetData.size(), transfer->source.ref, ""); ///\todo Specify the content hash.

    // If disksource is still empty, forcibly look up from cache
    if (!assetDiskSource.length())
        assetDiskSource = assetCache->GetDiskSourceByRef(transfer->source.ref);
    
    // Save for the asset the storage and provider it came from.
    transfer->asset->SetDiskSource(assetDiskSource.trimmed());
    transfer->asset->SetAssetStorage(transfer->storage.lock());
    transfer->asset->SetAssetProvider(transfer->provider.lock());
    transfer->asset->SetAssetTransfer(transfer);

    bool success = transfer->asset->LoadFromFileInMemory(&transfer->rawAssetData[0], transfer->rawAssetData.size());
    if (!success)
    {
        QString error("AssetAPI: Failed to load " + transfer->assetType + " '" + transfer->source.ref + "' from asset data.");
        transfer->asset->HandleLoadError(error);
        transfer->EmitAssetFailed(error);
        return;
    }

    // Remember the newly created asset in AssetAPI's internal data structure to allow clients to later fetch it without re-requesting it.
    AssetMap::iterator iter2 = assets.find(transfer->source.ref);
    if (iter2 != assets.end())
    {
        AssetPtr existing = iter2->second;
        LogWarning("AssetAPI: Overwriting a previously downloaded asset \"" + existing->Name() + "\", type \"" + existing->Type() + "\" with asset of same name!");
    }
    assets[transfer->source.ref] = transfer->asset;
    if (diskSourceChangeWatcher && !transfer->asset->DiskSource().isEmpty())
        diskSourceChangeWatcher->addPath(transfer->asset->DiskSource());
    emit AssetCreated(transfer->asset);

    // If this asset depends on any other assets, we have to make asset requests for those assets as well (and all assets that they refer to, and so on).
    RequestAssetDependencies(transfer->asset);

    // Tell everyone this transfer has now been downloaded. Note that when this signal is fired, the asset dependencies may not yet be loaded.
    transfer->EmitAssetDownloaded();

    // If we don't have any outstanding dependencies, fire the Loaded signal for the asset as well.
    if (NumPendingDependencies(transfer->asset) == 0)
        AssetDependenciesCompleted(transfer);
}

void AssetAPI::AssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
    assert(transfer);
    if (!transfer)
        return;

    LogError("Transfer of asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" failed! Reason: \"" + reason + "\"");

    ///\todo In this function, there is a danger of reaching an infinite recursion. Remember recursion parents and avoid infinite loops. (A -> B -> C -> A)

    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    if (iter == currentTransfers.end())
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer failed, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    // Signal any listeners that this asset transfer failed.
    transfer->EmitAssetFailed(reason);

    // Propagate the failure of this asset transfer to all assets which depend on this asset.
    std::vector<AssetPtr> dependents = FindDependents(transfer->source.ref);
    for(size_t i = 0; i < dependents.size(); ++i)
    {
        AssetTransferPtr dependentTransfer = GetPendingTransfer(dependents[i]->Name());
        if (dependentTransfer)
        {
            QString failReason = "Transfer of dependency " + transfer->source.ref + " failed due to reason: \"" + reason + "\"";
            AssetTransferFailed(dependentTransfer.get(), failReason);
        }
    }

    pendingDownloadRequests.erase(transfer->source.ref);
    if (iter != currentTransfers.end())
        currentTransfers.erase(iter);
}

void AssetAPI::AssetUploadTransferCompleted(IAssetUploadTransfer *uploadTransfer)
{
    uploadTransfer->EmitTransferCompleted();

    QString assetRef = uploadTransfer->AssetRef();
    // We've completed an asset upload transfer. See if there is an asset download transfer that is waiting
    // for this upload to complete. 
    
    // Before issuing a request, clear our cache of this data.
    /// \note We could actually update our cache with the same version of the asset that we just uploaded,
    /// to avoid downloading what we just uploaded. That can be implemented later.
    assetCache->DeleteAsset(assetRef);

    currentUploadTransfers.erase(assetRef); // Note: this might kill the 'transfer' ptr if we were the last one to hold on to it. Don't dereference transfer below this.
    PendingDownloadRequestMap::iterator iter = pendingDownloadRequests.find(assetRef);
    if (iter != pendingDownloadRequests.end())
    {
        PendingDownloadRequest req = iter->second;

        AssetTransferPtr transfer = RequestAsset(req.assetRef, req.assetType);
        if (!transfer)
            return; ///\todo Evaluate the path to take here.
        connect(transfer.get(), SIGNAL(Downloaded(IAssetTransfer*)), req.transfer.get(), SIGNAL(Downloaded(IAssetTransfer*)));
        connect(transfer.get(), SIGNAL(Decoded(AssetPtr)), req.transfer.get(), SIGNAL(Decoded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Loaded(AssetPtr)), req.transfer.get(), SIGNAL(Loaded(AssetPtr)));
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), req.transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)));
    }
}

void AssetAPI::AssetDependenciesCompleted(AssetTransferPtr transfer)
{
    // This asset transfer has finished - remove it from the internal list of ongoing transfers.
    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    if (iter != currentTransfers.end())
        currentTransfers.erase(iter);
    else // Even if we didn't know about this transfer, just print a warning and continue execution here nevertheless.
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    if (transfer->rawAssetData.size() == 0)
    {
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer finished: but data size was 0 bytes!");
        return;
    }

    ///\todo Specify the following flow better.
    transfer->EmitAssetDecoded();

    // This asset is now completely finished, and all its dependencies have been loaded.
    if (transfer->asset)
        transfer->asset->EmitLoaded();
    
    pendingDownloadRequests.erase(transfer->source.ref);
}

void AssetAPI::NotifyAssetDependenciesChanged(AssetPtr asset)
{
    /// Delete all old stored asset dependencies for this asset.
    RemoveAssetDependencies(asset->Name());

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        // Turn named storage (and default storage) specifiers to absolute specifiers.
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
        AssetReference ref = refs[i];
        if (ref.ref.isEmpty())
            continue;

        AssetPtr existing = GetAsset(ref.ref);
        if (existing)
            asset->DependencyLoaded(existing);
        else // We don't have the given asset yet, request it.
        {
            LogDebug("Asset " + asset->ToString() + " depends on asset " + ref.ref + " (type=\"" + ref.type + "\") which has not been loaded yet. Requesting..");
            RequestAsset(ref);
        }
    }
}

void AssetAPI::RemoveAssetDependencies(QString asset)
{
    for(size_t i = 0; i < assetDependencies.size(); ++i)
        if (QString::compare(assetDependencies[i].first, asset, Qt::CaseInsensitive) == 0)
        {
            assetDependencies.erase(assetDependencies.begin() + i);
            --i;
        }
}

std::vector<AssetPtr> AssetAPI::FindDependents(QString dependee)
{
    std::vector<AssetPtr> dependents;
    for(size_t i = 0; i < assetDependencies.size(); ++i)
    {
        if (QString::compare(assetDependencies[i].second, dependee, Qt::CaseInsensitive) == 0)
        {
            AssetMap::iterator iter = assets.find(assetDependencies[i].first);
            if (iter != assets.end())
                dependents.push_back(iter->second);
        }
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
        if (!existing)
        {
            // Not loaded, just mark the single one
            ++numDependencies;
        }
        else
        {
            // Ask the dependencies of the dependency, we want all of the asset
            // down the chain to be loaded before we load the base asset
            numDependencies += NumPendingDependencies(existing);
        }
    }

    return numDependencies;
}

void AssetAPI::OnAssetLoaded(AssetPtr asset)
{
    std::vector<AssetPtr> dependents = FindDependents(asset->Name());
    for(size_t i = 0; i < dependents.size(); ++i)
    {
        AssetPtr dependent = dependents[i];

        // Notify the asset that one of its dependencies has now been loaded in.
        dependent->DependencyLoaded(asset);

        // Check if this dependency was the last one of the given asset's dependencies.
        AssetTransferMap::iterator iter = currentTransfers.find(dependent->Name());
        if (iter != currentTransfers.end())
        {
            AssetTransferPtr transfer = iter->second;
            if (NumPendingDependencies(dependent) == 0)
                AssetDependenciesCompleted(transfer);
        }
    }
}

void AssetAPI::OnAssetDiskSourceChanged(const QString &path_)
{
    QDir path(path_);
    for(AssetMap::iterator iter = assets.begin(); iter != assets.end(); ++iter)
    {
        if (!iter->second->DiskSource().isEmpty() && QDir(iter->second->DiskSource()) == path)
        {
            // Return if file does not exists at the moment, no need to try asset->LoadFromCache()
            if (!QFile::exists(iter->second->DiskSource()))
                return;

            AssetPtr asset = iter->second;
            bool success = asset->LoadFromCache();
            if (!success)
                LogError("Failed to reload changed asset \"" + asset->ToString() + "\" from file \"" + path_ + "\"!");
            else
                LogDebug("Reloaded changed asset \"" + asset->ToString() + "\" from file \"" + path_ + "\".");
        }
    }
}

bool LoadFileToVector(const char *filename, std::vector<u8> &dst)
{
    FILE *handle = fopen(filename, "rb");
    if (!handle)
    {
        LogError("AssetAPI::LoadFileToVector: Failed to open file '" + std::string(filename) + "' for reading.");
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

QString AssetAPI::GetResourceTypeFromAssetRef(QString assetRef)
{
    QString filename;
    ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, &filename);

    ///\todo This whole function is to be removed, and moved into the asset type providers for separate access. -jj.

    QString file = filename.trimmed().toLower();
    if (file.endsWith(".qml") || file.endsWith(".qmlzip"))
        return "QML";
    if (file.endsWith(".mesh"))
        return "OgreMesh";
    if (file.endsWith(".skeleton"))
        return "OgreSkeleton";
    if (file.endsWith(".material"))
        return "OgreMaterial";
    if (file.endsWith(".particle"))
        return "OgreParticle";

    const char *textureFileTypes[] = { ".jpg", ".jpeg", ".png", ".tga", ".bmp", ".dds", ".gif" };
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

    if (file.endsWith(".wav") || file.endsWith(".ogg") || file.endsWith(".mp3"))
        return "Audio";

    if (file.endsWith(".ui"))
        return "QtUiFile";

    // \todo Dont hadcode these if the extension some day change!
    // cTundraBinFileExtension and cTundraXmlFileExtension are defined 
    // in SceneStructureModules .h files, move to core?
    if (file.endsWith(".xml") || file.endsWith(".txml") || file.endsWith(".tbin")) 
        return "Binary";

    // Unknown type, return Binray type.
    return "Binary";

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
