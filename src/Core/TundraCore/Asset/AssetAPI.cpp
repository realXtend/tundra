// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AssetAPI.h"
#include "CoreDefines.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "IAssetBundle.h"
#include "IAssetStorage.h"
#include "IAssetProvider.h"
#include "IAssetTypeFactory.h"
#include "IAssetBundleTypeFactory.h"
#include "IAssetUploadTransfer.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "AssetCache.h"

#include "Framework.h"
#include "LoggingFunctions.h"
#include "CoreException.h"
#include "Application.h"
#include "Profiler.h"
#include "CoreStringUtils.h"
#include "FileUtils.h"

#include <QDir>
#include <QFileSystemWatcher>
#include <QList>
#include <QMap>

#include "MemoryLeakCheck.h"

AssetAPI::AssetAPI(Framework *framework, bool headless) :
    fw(framework),
    isHeadless(headless),
    assetCache(0),
    diskSourceChangeWatcher(0)
{
    // The Asset API always understands at least this single built-in asset type "Binary".
    // You can use this type to request asset data as binary, without generating any kind of in-memory representation or loading for it.
    // Your module/component can then parse the content in a custom way.
    RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Binary", "")));
}

AssetAPI::~AssetAPI()
{
    Reset();
}

void AssetAPI::OpenAssetCache(QString directory)
{
    SAFE_DELETE(assetCache);
    SAFE_DELETE(diskSourceChangeWatcher);
    assetCache = new AssetCache(this, directory);
// On Android, we get spurious file change notifications. Disable watcher for now.
#ifndef ANDROID
    if (!fw->HasCommandLineParameter("--nofilewatcher"))
    {
        diskSourceChangeWatcher = new QFileSystemWatcher();
        connect(diskSourceChangeWatcher, SIGNAL(fileChanged(QString)), this, SLOT(OnAssetDiskSourceChanged(QString)), Qt::UniqueConnection);
    }
#endif
}

std::vector<AssetProviderPtr> AssetAPI::AssetProviders() const
{
    return providers;
}

void AssetAPI::RegisterAssetProvider(AssetProviderPtr provider)
{
    for(size_t i = 0; i < providers.size(); ++i)
    {
        if (providers[i]->Name() == provider->Name())
        {
            LogWarning("AssetAPI::RegisterAssetProvider: Provider with name '" + provider->Name() + "' already registered.");
            return;
        }
    }
    providers.push_back(provider);
}

AssetStoragePtr AssetAPI::AssetStorageByName(const QString &name) const
{
    foreach(AssetProviderPtr provider, GetAssetProviders())
        foreach(AssetStoragePtr storage, provider->GetStorages())
            if (storage->Name().compare(name, Qt::CaseInsensitive) == 0)
                return storage;
    return AssetStoragePtr();
}

AssetStoragePtr AssetAPI::StorageForAssetRef(const QString &ref) const
{
    PROFILE(AssetAPI_GetStorageForAssetRef);
    foreach(AssetProviderPtr provider, GetAssetProviders())
    {
        AssetStoragePtr storage = provider->GetStorageForAssetRef(ref);
        if (storage)
            return storage;
    }
    return AssetStoragePtr();
}

bool AssetAPI::RemoveAssetStorage(const QString &name)
{
    ///\bug Currently it is possible to have e.g. a local storage with name "Foo" and a http storage with name "Foo", and it will
    /// not be possible to specify which storage to delete.
    foreach(AssetProviderPtr provider, GetAssetProviders())
        if (provider->RemoveAssetStorage(name))
            return true;

    return false;
}

AssetStoragePtr AssetAPI::DeserializeAssetStorageFromString(const QString &storage, bool fromNetwork)
{
    for(size_t i = 0; i < providers.size(); ++i)
    {
        AssetStoragePtr assetStorage = providers[i]->TryDeserializeStorageFromString(storage, fromNetwork);
        // The above function will call back to AssetAPI::EmitAssetStorageAdded.
        if (assetStorage)
        {
            // Make this storage the default storage if it was requested so.
            QMap<QString, QString> s = AssetAPI::ParseAssetStorageString(storage);
            if (s.contains("default") && ParseBool(s["default"]))
                SetDefaultAssetStorage(assetStorage);
            return assetStorage;
        }
    }
    return AssetStoragePtr();
}

AssetStoragePtr AssetAPI::DefaultAssetStorage() const
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
    if (storage)
        LogInfo("Set asset storage \"" + storage->Name() + "\" as the default storage (" + storage->SerializeToString() + ").");
    else
        LogInfo("Set (null) as the default asset storage.");
}

AssetMap AssetAPI::AssetsOfType(const QString& type) const
{
    AssetMap ret;
    for(AssetMap::const_iterator i = assets.begin(); i != assets.end(); ++i)
        if (!i->second->Type().compare(type, Qt::CaseInsensitive))
            ret[i->first] = i->second;
    return ret;
}

std::vector<AssetStoragePtr> AssetAPI::AssetStorages() const
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

AssetAPI::FileQueryResult AssetAPI::ResolveLocalAssetPath(QString ref, QString baseDirectoryContext, QString &outFilePath, QString *subAssetName) const
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
                                               QString *outFilename, QString *outSubAssetName, QString *outFullRef, QString *outFullRefNoSubAssetName)
{
    if (outProtocolPart) *outProtocolPart = "";
    if (outNamedStorage) *outNamedStorage = "";
    if (outProtocol_Path) *outProtocol_Path = "";
    QString protocol_path = "";
    if (outPath_Filename_SubAssetName) *outPath_Filename_SubAssetName = "";
    if (outPath_Filename) *outPath_Filename = "";
    if (outPath) *outPath = "";
    if (outFilename) *outFilename = "";
    if (outSubAssetName) *outSubAssetName = "";
    if (outFullRef) *outFullRef = "";
    if (outFullRefNoSubAssetName) *outFullRefNoSubAssetName = "";

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
        protocol_path = protocol + "://";
        if (outProtocol_Path) // Partially save the beginning of the protocol & path part. This will be completed below with the full path.
            *outProtocol_Path = protocol_path;

        if (outFullRef)
            *outFullRef = protocol.toLower() + "://";
    }
    else if (assetRef.startsWith("www.", Qt::CaseInsensitive)) // Ref is of type 'b0)'?
    {
        refType = AssetRefExternalUrl;
        if (outProtocolPart)
            *outProtocolPart = "http";
        fullPath = ref;

        protocol_path = "http://";
        if (outProtocol_Path) // Partially save the beginning of the protocol & path part. This will be completed below with the full path.
            *outProtocol_Path = protocol_path;
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

        protocol_path = storage + ":";
        if (outProtocol_Path)
            *outProtocol_Path = protocol_path;
        if (outFullRef)
            *outFullRef = protocol_path;
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
    wregex expression4(L"(.*?)\\s*[#,]\\s*\"?\\s*(.*?)\\s*\"?\\s*"); // assetRef, "subAssetName". Note: this regex does not parse badly matched '"' signs, but it's a minor issue. (e.g. 'assetRef, ""jeejee' is incorrectly accepted) .
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
    protocol_path += path;
    if (outProtocol_Path)
        *outProtocol_Path += path;
    QString assetFilename = fullPathRef.mid(directorySeparatorIndex+1);
    if (outFilename)
        *outFilename = assetFilename;
    if (outFullRef)
    {
        *outFullRef += fullPathRef;
        if (!subAssetName.isEmpty())
        {
            if (subAssetName.contains(' '))
                *outFullRef += "#\"" + subAssetName + "\"";
            else
                *outFullRef += "#" + subAssetName;
        }
    }

    if (outFullRefNoSubAssetName)
        *outFullRefNoSubAssetName = GuaranteeTrailingSlash(protocol_path) + assetFilename;
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
    basePath = QDir::fromNativeSeparators(basePath.trimmed());
    filename = ExtractFilenameFromAssetRef(filename.trimmed());

    QFileInfo fileInfo(GuaranteeTrailingSlash(basePath) + filename);
    if (fileInfo.exists())
        return fileInfo.absoluteFilePath();

    foreach(QString dir, DirectorySearch(basePath, true, QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks))
    {
        QFileInfo fileInfo(GuaranteeTrailingSlash(dir) + filename);
        if (fileInfo.exists())
            return fileInfo.absoluteFilePath();
    }

    return "";
}

AssetPtr AssetAPI::CreateAssetFromFile(QString assetType, QString assetFile)
{
    AssetPtr asset = CreateNewAsset(assetType, assetFile);
    if (!asset)
        return AssetPtr();
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
        if (assetCache)
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
    AssetStoragePtr storage = GetAssetStorageByName(storageName);
    if (!storage.get())
    {
        LogError("AssetAPI::UploadAssetFromFile failed! No storage found with name " + storageName + "! Please add a storage with this name.");
        return AssetUploadTransferPtr();
    }
    
    // Protect crashes when as this function is called from scripts!
    AssetUploadTransferPtr transfer;
    try
    {
        transfer = UploadAssetFromFile(filename, storage, newAssetName);
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

AssetUploadTransferPtr AssetAPI::UploadAssetFromFile(const QString &filename, AssetStoragePtr destination, const QString &assetName)
{
    if (filename.isEmpty())
        throw Exception("AssetAPI::UploadAssetFromFile failed! No source filename given!");

    if (assetName.isEmpty())
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
    AssetStoragePtr storage = GetAssetStorageByName(storageName);
    if (!storage.get())
    {
        LogError("AssetAPI::UploadAssetFromFileInMemory failed! No storage found with name " + storageName + "! Please add a storage with this name.");
        return AssetUploadTransferPtr();
    }

    // Protect crashes when as this function is called from scripts!
    AssetUploadTransferPtr transfer;
    try
    {
        transfer = UploadAssetFromFileInMemory((const u8*)data.constData(), data.size(), storage, assetName);
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

AssetUploadTransferPtr AssetAPI::UploadAssetFromFileInMemory(const u8 *data, size_t numBytes, AssetStoragePtr destination, const QString &assetName)
{
    if (!data || numBytes == 0)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! Null source data passed!");

    if (assetName.isEmpty())
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! No destination asset name given!");

    if (!destination)
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The passed destination asset storage was null!");

    if (!destination->Writable())
        throw Exception("AssetAPI::UploadAssetFromFileInMemory failed! The storage is not writable.");

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
    readyTransfers.clear();
    readySubTransfers.clear();

    // Note that this wont unload the individual sub asset, only the bundle. 
    // Sub assets are unloaded from the assets map below.
    while(assetBundles.size() > 0)
    {
        AssetBundleMap::iterator iter = assetBundles.begin();
        if (iter != assetBundles.end())
        {
            AssetBundlePtr bundle = iter->second;
            if (bundle.get())
            {
                bundle->Unload();
                bundle.reset();
            }
            assetBundles.erase(iter);
        }
        else
            break; 
    }
    assetBundles.clear();
    bundleMonitors.clear();

    // ForgetAsset removes the asset it is given to from the assets list, so this loop terminates.
    while(assets.size() > 0)
        ForgetAsset(assets.begin()->second, false);
    assets.clear();
   
    // Abort all current transfers.
    while (currentTransfers.size() > 0)
    {
        AssetTransferPtr abortTransfer = currentTransfers.begin()->second;
        if (!abortTransfer.get())
        {
            currentTransfers.erase(currentTransfers.begin());
            continue;
        }
        QString abortRef = abortTransfer->source.ref;
        abortTransfer->Abort();
        abortTransfer.reset();
        
        // Make sure the abort chain removed the transfer, otherwise we are in a infinite loop.
        AssetTransferMap::iterator iter = currentTransfers.find(abortRef);
        if (iter != currentTransfers.end())
            currentTransfers.erase(iter);
    }
    currentTransfers.clear();
}

void AssetAPI::Reset()
{
    ForgetAllAssets();
    SAFE_DELETE(assetCache);
    SAFE_DELETE(diskSourceChangeWatcher);
    assets.clear();
    assetBundles.clear();
    bundleMonitors.clear();
    pendingDownloadRequests.clear();
    assetTypeFactories.clear();
    assetBundleTypeFactories.clear();
    defaultStorage.reset();
    readyTransfers.clear();
    readySubTransfers.clear();
    assetDependencies.clear();
    currentUploadTransfers.clear();
    currentTransfers.clear();
    providers.clear();
}

std::vector<AssetTransferPtr> AssetAPI::PendingTransfers() const
{
    std::vector<AssetTransferPtr> transfers;
    for(AssetTransferMap::const_iterator iter = currentTransfers.begin(); iter != currentTransfers.end(); ++iter)
        transfers.push_back(iter->second);

    transfers.insert(transfers.end(), readyTransfers.begin(), readyTransfers.end());
    return transfers;
}

AssetTransferPtr AssetAPI::GetPendingTransfer(QString assetRef) const
{
    AssetTransferMap::const_iterator iter = currentTransfers.find(assetRef);
    if (iter != currentTransfers.end())
        return iter->second;
    for(size_t i = 0; i < readyTransfers.size(); ++i)
        if (readyTransfers[i]->source.ref == assetRef)
            return readyTransfers[i];

    return AssetTransferPtr();
}

AssetTransferPtr AssetAPI::RequestAsset(QString assetRef, QString assetType, bool forceTransfer)
{
    // This is a function that handles all asset requests made to the Tundra asset system.
    // Note that touching this function has many implications all around the complex asset load routines
    // and its multiple steps. It is quite easy to introduce bugs and break subtle things by modifying this function.
    // * You can request both direct asset references <ref> or asset references into bundles <ref>#subref.
    // * This single function handles requests to not yet loaded assets and already loaded assets.
    //   Here we also handle forced reloads from the source via the forceTransfer parameter if
    //   the asset is already loaded to the system. This function also handles reloading existing
    //   but at the moment unloaded assets.

    PROFILE(AssetAPI_RequestAsset);

    // Turn named storage and default storage specifiers to absolute specifiers.
    assetRef = ResolveAssetRef("", assetRef);
    assetType = assetType.trimmed();
    if (assetRef.isEmpty())
        return AssetTransferPtr();

    // Parse out full reference, main asset ref and sub asset ref.
    QString fullAssetRef, subAssetPart, mainAssetPart;
    ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, 0, &subAssetPart, &fullAssetRef, &mainAssetPart);
    
    // Detect if the requested asset is a sub asset. Replace the lookup ref with the parent bundle reference.
    // Note that bundle handling has its own code paths as we need to load the bundle first before
    // querying the sub asset(s) from it. Bundles cannot be implemented via IAsset, or could but
    // the required hacks would be much worse than a bit more internal AssetAPI code.
    //   1) If we have the sub asset already loaded we return a virtual transfer for it as normal.
    //   2) If we don't have the sub asset but we have the bundle we request the data from the bundle.
    //   3) If we don't have the bundle we request it. If that succeeds we go to 2), if not we fail the sub asset transfer.
    /// @todo Live reloading on disk source changes is not implemented for bundles. Implement if there seems to be need for it. 
    bool isSubAsset = !subAssetPart.isEmpty();
    if (isSubAsset) 
        assetRef = mainAssetPart;

    // To optimize, we first check if there is an outstanding request to the given asset. If so, we return that request. In effect, we never
    // have multiple transfers running to the same asset. Important: This must occur before checking the assets map for whether we already have the asset in memory, since
    // an asset will be stored in the AssetMap when it has been downloaded, but it might not yet have all its dependencies loaded.
    AssetTransferMap::iterator ongoingTransferIter = currentTransfers.find(assetRef);
    if (ongoingTransferIter != currentTransfers.end())
    {
        AssetTransferPtr transfer = ongoingTransferIter->second;
        if (forceTransfer && dynamic_cast<VirtualAssetTransfer*>(transfer.get()))
        {
            // If forceTransfer is on, but the transfer is virtual, log error. This case can not be currently handled properly.
            LogError("AssetAPI::RequestAsset: Received forceTransfer for asset " + assetRef + " while a virtual transfer is already going on");
            return transfer;
        }

        // If this is a sub asset ref to a bundle, we just found the bundle transfer with assetRef. 
        // We need to add this sub asset transfer to the bundles monitor. We return the virtual transfer that
        // will get loaded once the bundle is loaded.
        AssetBundleMonitorMap::iterator bundleMonitorIter = bundleMonitors.find(assetRef);
        if (bundleMonitorIter != bundleMonitors.end())
        {
            AssetBundleMonitorPtr assetBundleMonitor = (*bundleMonitorIter).second;
            if (assetBundleMonitor)
            {
                AssetTransferPtr subTransfer = assetBundleMonitor->SubAssetTransfer(fullAssetRef);
                if (!subTransfer.get())
                {
                    subTransfer = AssetTransferPtr(new VirtualAssetTransfer());
                    subTransfer->source.ref = fullAssetRef;
                    subTransfer->assetType = GetResourceTypeFromAssetRef(subTransfer->source.ref);
                    subTransfer->provider = transfer->provider;
                    subTransfer->storage = transfer->storage;

                    assetBundleMonitor->AddSubAssetTransfer(subTransfer);
                }
                return subTransfer;
            }
        }

        if (assetType.isEmpty())
            assetType = GetResourceTypeFromAssetRef(mainAssetPart);
        if (!assetType.isEmpty() && !transfer->assetType.isEmpty() && assetType != transfer->assetType)
        {
            // Check that the requested types were the same. Don't know what to do if they differ, so only print a warning if so.
            LogWarning("AssetAPI::RequestAsset: Asset \"" + assetRef + "\" first requested by type " + 
                transfer->assetType + ", but now requested by type " + assetType + ".");
        }
        
        return transfer;
    }

    // Check if we've already downloaded this asset before and it already is loaded in the system. We never reload an asset we've downloaded before, 
    // unless the client explicitly forces so, or if we get a change notification signal from the source asset provider telling the asset was changed.
    // Note that we are using fullRef here as it has the complete sub asset ref also in it. If this is a sub asset request the assetRef has already been modified.
    AssetPtr existingAsset;
    AssetMap::iterator existingAssetIter = assets.find(fullAssetRef);
    if (existingAssetIter != assets.end())
    {
        existingAsset = existingAssetIter->second;
        if (!assetType.isEmpty() && assetType != existingAsset->Type())
            LogWarning("AssetAPI::RequestAsset: Tried to request asset \"" + assetRef + "\" by type \"" + assetType + "\". Asset by that name exists, but it is of type \"" + existingAsset->Type() + "\"!");
        assetType = existingAsset->Type();
    }
    else
    {
        // If this is a sub asset we must set the type from the parent bundle.
        if (assetType.isEmpty())
            assetType = GetResourceTypeFromAssetRef(mainAssetPart);

        // Null factories are used for not loading particular assets.
        // Having this option we can return a null transfer here to not
        // get lots of error logging eg. on a headless Tundra for UI assets.
        if (dynamic_cast<NullAssetFactory*>(GetAssetTypeFactory(assetType).get()))
            return AssetTransferPtr();
    }
    
    // Whenever the client requests an asset that was loaded before, we create a request for that asset nevertheless.
    // The idea is to have the code path run the same independent of whether the asset existed or had to be downloaded, i.e.
    // a request is always made, and the receiver writes only a single asynchronous code path for handling the asset.
    /// @todo Evaluate whether existing->IsLoaded() should rather be existing->IsEmpty().
    if (existingAsset && existingAsset->IsLoaded() && !forceTransfer)
    {
        // The asset was already downloaded. Generate a 'virtual asset transfer' 
        // and return it to the client. Fill in the valid existing asset ptr to the transfer.
        AssetTransferPtr transfer = AssetTransferPtr(new VirtualAssetTransfer());
        transfer->asset = existingAsset;
        transfer->source.ref = assetRef;
        transfer->assetType = assetType;
        transfer->provider = transfer->asset->GetAssetProvider();
        transfer->storage = transfer->asset->GetAssetStorage();
        transfer->diskSourceType = transfer->asset->DiskSourceType();
        
        // There is no asset provider processing this 'transfer' that would "push" the AssetTransferCompleted call. 
        // We have to remember to do it ourselves via readyTransfers list in Update().
        readyTransfers.push_back(transfer); 
        return transfer;
    }    

    // If this is a sub asset request check if its parent bundle is already available.
    // If it is load/reload the asset data to the transfer and use the readyTransfers
    // list to do the AssetTransferCompleted callback on the next frame.
    if (isSubAsset)
    {
        // Check if sub asset transfer is ongoing, meaning we already have been below and its still being processed.
        AssetTransferMap::iterator ongoingSubAssetTransferIter = currentTransfers.find(fullAssetRef);
        if (ongoingSubAssetTransferIter != currentTransfers.end())
            return ongoingSubAssetTransferIter->second;
        
        // Create a new transfer and load the asset from the bundle to it
        AssetBundleMap::iterator bundleIter = assetBundles.find(assetRef);
        if (bundleIter != assetBundles.end())
        {
            // Return existing loader transfer
            for(size_t i = 0; i < readySubTransfers.size(); ++i)
            {
                AssetTransferPtr subTransfer = readySubTransfers[i].subAssetTransfer;
                if (subTransfer.get() && subTransfer->source.ref.compare(fullAssetRef, Qt::CaseSensitive) == 0)
                    return subTransfer;
            }
            // Create a loader for this sub asset.
            AssetTransferPtr transfer = AssetTransferPtr(new VirtualAssetTransfer());
            transfer->asset = existingAsset;
            transfer->source.ref = fullAssetRef;
            readySubTransfers.push_back(SubAssetLoader(assetRef, transfer));
            return transfer;
        }
        else
        {
            // For a sub asset the parent bundle needs to 
            // be requested by its proper asset type.
            assetType = GetResourceTypeFromAssetRef(mainAssetPart);
        }
    }

    // See if there is an asset upload that should block this download. If the same asset is being
    // uploaded and downloaded simultaneously, make the download wait until the upload completes.
    if (currentUploadTransfers.find(assetRef) != currentUploadTransfers.end())
    {
        LogDebug("The download of asset \"" + assetRef + "\" needs to wait, since the same asset is being uploaded at the moment.");
        PendingDownloadRequest pendingRequest;
        pendingRequest.assetRef = assetRef;
        pendingRequest.assetType = assetType;
        pendingRequest.transfer = AssetTransferPtr(new IAssetTransfer);

        pendingDownloadRequests[assetRef] = pendingRequest;

        /// @bug Problem. When we return this structure, the client will connect to this.
        return pendingRequest.transfer; 
    }

    // Find the asset provider that will fulfill this request.
    AssetProviderPtr provider = GetProviderForAssetRef(assetRef, assetType);
    if (!provider)
    {
        LogError("AssetAPI::RequestAsset: Failed to find a provider for asset \"" + assetRef + "\", type: \"" + assetType + "\"");
        return AssetTransferPtr();
    }

    // Perform the actual request from the provider.
    AssetTransferPtr transfer = provider->RequestAsset(assetRef, assetType);
    if (!transfer)
    {
        LogError("AssetAPI::RequestAsset: Failed to request asset \"" + assetRef + "\", type: \"" + assetType + "\"");
        return AssetTransferPtr();
    }

    // Note that existingAsset can be a valid loaded asset at this point. In this case
    // we can only be this far in this function if forceTransfer is true. This makes the 
    // upcoming download and load process will reload the asset data instead of creating a new asset.
    transfer->asset = existingAsset;
    transfer->provider = provider;

    // Store the newly allocated AssetTransfer internally, so that any duplicated requests to this asset 
    // will return the same request pointer, so we'll avoid multiple downloads to the exact same asset.
    currentTransfers[assetRef] = transfer;

    // Request for a direct asset reference.
    if (!isSubAsset)
        return transfer;
    // Request for a sub asset in a bundle. 
    else
    {       
        // AssetBundleMonitor is for tracking object for this bundle and all pending sub asset transfers.
        // It will connect to the asset transfer and create the bundle on download succeeded or handle it failing
        // by notifying all the child transfer failed.
        AssetBundleMonitorPtr bundleMonitor;
        AssetBundleMonitorMap::iterator bundleMonitorIter = bundleMonitors.find(assetRef);
        if (bundleMonitorIter != bundleMonitors.end())
        {
            // Add the sub asset to an existing bundle monitor.
            bundleMonitor = (*bundleMonitorIter).second;
        }
        else
        {
            // Create new bundle monitor.
            bundleMonitor = AssetBundleMonitorPtr(new AssetBundleMonitor(this, transfer));
            bundleMonitors[assetRef] = bundleMonitor;
        }
        if (!bundleMonitor.get())
        {
            LogError("AssetAPI::RequestAsset: Failed to get or create asset bundle: " + assetRef);
            return AssetTransferPtr();
        }
        
        AssetTransferPtr subTransfer = bundleMonitor->SubAssetTransfer(fullAssetRef);
        if (!subTransfer.get())
        {
            // In this case we return a virtual transfer for the sub asset.
            // This transfer will be loaded once the bundle can provide the content.
            subTransfer = AssetTransferPtr(new VirtualAssetTransfer());
            subTransfer->source.ref = fullAssetRef;
            subTransfer->assetType = GetResourceTypeFromAssetRef(subTransfer->source.ref);
            subTransfer->provider = transfer->provider;
            subTransfer->storage = transfer->storage;
            
            bundleMonitor->AddSubAssetTransfer(subTransfer);
        }
        return subTransfer;
    }
}

AssetTransferPtr AssetAPI::RequestAsset(const AssetReference &ref, bool forceTransfer)
{
    return RequestAsset(ref.ref, ref.type, forceTransfer);
}

AssetProviderPtr AssetAPI::ProviderForAssetRef(QString assetRef, QString assetType) const
{
    PROFILE(AssetAPI_GetProviderForAssetRef);

    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromAssetRef(assetRef.toLower());

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
        AssetStoragePtr storage = GetAssetStorageByName(namedStorage);
        AssetProviderPtr provider = (storage ? storage->provider.lock() : AssetProviderPtr());
        return provider;
    }

    std::vector<AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
        if (providers[i]->IsValidRef(assetRef, assetType))
            return providers[i];

    return AssetProviderPtr();
}

QString AssetAPI::ResolveAssetRef(QString context, QString assetRef) const
{
    if (assetRef.trimmed().isEmpty())
        return "";

    context = context.trimmed();

    // First see if we have an exact match for the ref to an existing asset.
    AssetMap::const_iterator iter = assets.find(assetRef);
    if (iter != assets.end())
        return assetRef; // Use the ref as-is, there's an existing asset to map this string to.

    // If the assetRef is by local filename without a reference to a provider or storage, use the default asset storage in the system for this assetRef.
    QString assetPath;
    QString namedStorage;
    QString fullRef;
    AssetRefType assetRefType = ParseAssetRef(assetRef, 0, &namedStorage, 0, &assetPath, 0, 0, 0, 0, &fullRef);
    assetRef = fullRef; // The first thing we do is normalize the form of the ref. This means e.g. adding 'http://' in front of refs that look like 'www.server.com/'.
    
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
                QString contextSubAssetPart;
                QString contextMainPart;
                AssetRefType contextRefType = ParseAssetRef(context, &contextProtocolSpecifier, &contextNamedStorage, 0, 0, 0, &contextPath, 0, &contextSubAssetPart, 0, &contextMainPart);
                if (contextRefType == AssetRefRelativePath || contextRefType == AssetRefInvalid)
                {
                    LogError("Asset ref context \"" + contextPath + "\" is a relative path and cannot be used as a context for lookup for ref \"" + assetRef + "\"!");
                    return assetRef; // Return at least something.
                }
                
                // Context is a asset bundle sub asset
                if (!contextSubAssetPart.isEmpty())
                {
                    // Context: bundle#subfolder/subasset -> bundle#subfolder/<assetPath>
                    if (contextSubAssetPart.contains("/"))
                        return contextMainPart + "#" + contextSubAssetPart.left(contextSubAssetPart.lastIndexOf("/")+1) + assetPath;
                    // Context: bundle#subasset -> bundle#<assetPath>
                    else
                        return contextMainPart + "#" + assetPath;
                }
                
                QString newAssetRef;
                if (!contextNamedStorage.isEmpty())
                    newAssetRef = contextNamedStorage + ":";
                else if (!contextProtocolSpecifier.isEmpty())
                    newAssetRef = contextProtocolSpecifier + "://";
                newAssetRef += QDir::cleanPath(contextPath + assetPath);
                return newAssetRef;
            }
        }
        break;
    case AssetRefNamedStorage: // The asset ref explicitly points to a named storage. Use the provider for that storage.
        {
            AssetStoragePtr storage = GetAssetStorageByName(namedStorage);
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

void AssetAPI::RegisterAssetBundleTypeFactory(AssetBundleTypeFactoryPtr factory)
{
    AssetBundleTypeFactoryPtr existingFactory = GetAssetBundleTypeFactory(factory->Type());
    if (existingFactory)
    {
        LogWarning("AssetAPI::RegisterAssetBundleTypeFactory: Factory with type '" + factory->Type() + "' already registered.");
        return;
    }

    assert(factory->Type() == factory->Type().trimmed());
    assetBundleTypeFactories.push_back(factory);
}

QString AssetAPI::GenerateUniqueAssetName(QString assetTypePrefix, QString assetNamePrefix) const
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

QString AssetAPI::GenerateTemporaryNonexistingAssetFilename(QString filenameSuffix) const
{
    static unsigned long uniqueRunningFilenameCounter = 1;

    // Create this file path into the cache dir to avoid
    // windows non-admin users having no write permission to the run folder
    QDir cacheDir;
    if (assetCache)
        cacheDir = QDir(assetCache->CacheDirectory());
    else
        cacheDir = QDir(Application::UserDataDirectory());
    if (cacheDir.exists())
    {
        QString filename;
        // We loop until we manage to generate a single filename that does not exist, incrementing a running counter at each iteration.
        for(int i = 0; i < 10000; ++i) // The intent is to loop 'infinitely' until a name is found, but do an artificial limit to avoid voodoo bugs.
        {
            filename = cacheDir.absoluteFilePath("temporary_" + QString::number(uniqueRunningFilenameCounter++) + "_" + SanitateAssetRef(filenameSuffix.trimmed()));
            if (!QFile::exists(filename))
                return filename;
        }
    }
    assert(false);
    throw Exception("GenerateTemporaryNonexistingAssetFilename failed!");
}

AssetPtr AssetAPI::CreateNewAsset(QString type, QString name)
{
    return CreateNewAsset(type, name, AssetStoragePtr());
}

AssetPtr AssetAPI::CreateNewAsset(QString type, QString name, AssetStoragePtr storage)
{
    PROFILE(AssetAPI_CreateNewAsset);
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
        // This spams too much with the server giving us storages, debug should be fine for things we dont have a factory.
        LogDebug("AssetAPI:CreateNewAsset: Cannot create asset of type \"" + type + "\", name: \"" + name + "\". No type factory registered for the type!");
        return AssetPtr();
    }
    if (dynamic_cast<NullAssetFactory*>(factory.get()))
        return AssetPtr();
    AssetPtr asset = factory->CreateEmptyAsset(this, name);

    if (!asset)
    {
        LogError("AssetAPI:CreateNewAsset: IAssetTypeFactory::CreateEmptyAsset(type \"" + type + "\", name: \"" + name + "\") failed to create asset!");
        return AssetPtr();
    }
    assert(asset->IsEmpty());

    // Fill the provider & storage for the new asset already here if possible
    ///\todo Revisit the logic below, and the callers of CreateNewAsset. Is it possible to remove the following
    /// calls, and have the caller directly set the storage?
    if (!storage)
    {
        asset->SetAssetProvider(GetProviderForAssetRef(type, name));
        asset->SetAssetStorage(GetStorageForAssetRef(name));
    }
    else
    {
        asset->SetAssetProvider(storage->provider.lock());
        asset->SetAssetStorage(storage);
    }

    // Remember this asset in the global AssetAPI storage.
    assets[name] = asset;

    ///\bug DiskSource and DiskSourceType are not set yet.
    {
        PROFILE(AssetAPI_CreateNewAsset_emit_AssetCreated);
        emit AssetCreated(asset);
    }
    
    return asset;
}

AssetBundlePtr AssetAPI::CreateNewAssetBundle(QString type, QString name)
{
    PROFILE(AssetAPI_CreateNewAssetBundle);
    type = type.trimmed();
    name = name.trimmed();
    if (name.length() == 0)
    {
        LogError("AssetAPI:CreateNewAssetBundle: Trying to create an asset with name=\"\"!");
        return AssetBundlePtr();
    }

    AssetBundleTypeFactoryPtr factory = GetAssetBundleTypeFactory(type);
    if (!factory)
    {
        // This spams too much with the server giving us storages, debug should be fine for things we don't have a factory.
        LogError("AssetAPI:CreateNewAssetBundle: Cannot create asset of type \"" + type + "\", name: \"" + name + "\". No type factory registered for the type!");
        return AssetBundlePtr();
    }
    AssetBundlePtr assetBundle = factory->CreateEmptyAssetBundle(this, name);
    if (!assetBundle)
    {
        LogError("AssetAPI:CreateNewAssetBundle: IAssetTypeFactory::CreateEmptyAsset(type \"" + type + "\", name: \"" + name + "\") failed to create asset!");
        return AssetBundlePtr();
    }

    // Remember this asset bundle in the global AssetAPI storage.
    assetBundles[name] = assetBundle;
    return assetBundle;
}

bool AssetAPI::LoadSubAssetToTransfer(AssetTransferPtr transfer, const QString &bundleRef, const QString &fullSubAssetRef, QString subAssetType)
{
    if (!transfer)
    {
        LogError("LoadSubAssetToTransfer: Transfer is null, cannot continue to load '" + fullSubAssetRef + "'.");
        return false;
    }

    AssetBundleMap::iterator bundleIter = assetBundles.find(bundleRef);
    if (bundleIter != assetBundles.end())
        return LoadSubAssetToTransfer(transfer, (*bundleIter).second.get(), fullSubAssetRef, subAssetType);
    else
    {
        LogError("LoadSubAssetToTransfer: Asset bundle '" + bundleRef + "' not loaded, cannot continue to load '" + fullSubAssetRef + "'.");
        return false;
    }
}

bool AssetAPI::LoadSubAssetToTransfer(AssetTransferPtr transfer, IAssetBundle *bundle, const QString &fullSubAssetRef, QString subAssetType)
{
    if (!transfer)
    {
        LogError("LoadSubAssetToTransfer: Transfer is null, cannot continue to load '" + fullSubAssetRef + "'.");
        return false;
    }
    if (!bundle)
    {
        LogError("LoadSubAssetToTransfer: Source AssetBundle is null, cannot continue to load '" + fullSubAssetRef + "'.");
        return false;
    }

    QString subAssetRef;
    if (subAssetType.isEmpty())
        subAssetType = GetResourceTypeFromAssetRef(fullSubAssetRef);
    ParseAssetRef(fullSubAssetRef, 0, 0, 0, 0, 0, 0, 0, &subAssetRef);
    
    transfer->source.ref = fullSubAssetRef;
    transfer->source.type = subAssetType;
    transfer->assetType = subAssetType;

    // Avoid data shuffling if disk source is valid. IAsset loading can 
    // manage with one of these, it does not need them both.
    std::vector<u8> subAssetData;
    QString subAssetDiskSource = bundle->GetSubAssetDiskSource(subAssetRef);
    if (subAssetDiskSource.isEmpty()) 
    {
        subAssetData = bundle->GetSubAssetData(subAssetRef);
        if (subAssetData.size() == 0)
        {
            QString error("AssetAPI: Failed to load sub asset '" + fullSubAssetRef + " from bundle '" + bundle->Name() + "': Sub asset does not exist.");
            LogError(error);
            transfer->EmitAssetFailed(error);
            return false;
        }
    }

    if (!transfer->asset)
        transfer->asset = CreateNewAsset(subAssetType, fullSubAssetRef);
    if (!transfer->asset)
    {
        QString error("AssetAPI: Failed to create new sub asset of type \"" + subAssetType + "\" and name \"" + fullSubAssetRef + "\"");
        LogError(error);
        transfer->EmitAssetFailed(error);
        return false;
    }
    
    transfer->asset->SetDiskSource(subAssetDiskSource);
    transfer->asset->SetDiskSourceType(IAsset::Bundle);
    transfer->asset->SetAssetStorage(transfer->storage.lock());
    transfer->asset->SetAssetProvider(transfer->provider.lock());

    // Add the sub asset transfer to the current transfers map now.
    // This will ensure that the rest of the loading procedure will continue
    // like it normally does in AssetAPI and the requesting parties don't have to
    // know about how asset bundles are packed or request them before the sub asset in any way.
    currentTransfers[fullSubAssetRef] = transfer;

    // Connect to Loaded() signal of the asset to be able to notify any dependent assets
    connect(transfer->asset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAssetLoaded(AssetPtr)), Qt::UniqueConnection);

    // Tell everyone this transfer has now been downloaded. Note that when this signal is fired, the asset dependencies may not yet be loaded.
    transfer->EmitAssetDownloaded();

    bool success = false;
    if (subAssetData.size() > 0)
        success = transfer->asset->LoadFromFileInMemory(&subAssetData[0], subAssetData.size());
    else if (!transfer->asset->DiskSource().isEmpty())
        success = transfer->asset->LoadFromFile(subAssetDiskSource);

    // If the load from either of in memory data or file data failed, update the internal state.
    // Otherwise the transfer will be left dangling in currentTransfers. For successful loads
    // we do no need to call AssetLoadCompleted because success can mean asynchronous loading,
    // in which case the call will arrive once the asynchronous loading is completed.
    if (!success)
        AssetLoadFailed(fullSubAssetRef);
    return success;
}

AssetTypeFactoryPtr AssetAPI::AssetTypeFactory(QString typeName) const{
    PROFILE(AssetAPI_GetAssetTypeFactory);
    for(size_t i = 0; i < assetTypeFactories.size(); ++i)
        if (assetTypeFactories[i]->Type().toLower() == typeName.toLower())
            return assetTypeFactories[i];

    return AssetTypeFactoryPtr();
}

AssetBundleTypeFactoryPtr AssetAPI::GetAssetBundleTypeFactory(QString typeName) const
{
    PROFILE(AssetAPI_GetAssetBundleTypeFactory);
    for(size_t i = 0; i < assetBundleTypeFactories.size(); ++i)
        if (assetBundleTypeFactories[i]->Type().toLower() == typeName.toLower())
            return assetBundleTypeFactories[i];

    return AssetBundleTypeFactoryPtr();
}

AssetPtr AssetAPI::GetAsset(QString assetRef) const
{
    // First try to see if the ref has an exact match.
    AssetMap::const_iterator iter = assets.find(assetRef);
    if (iter != assets.end())
        return iter->second;

    // If not, normalize and resolve the lookup of the given asset.
    assetRef = ResolveAssetRef("", assetRef);

    iter = assets.find(assetRef);
    if (iter != assets.end())
        return iter->second;
    return AssetPtr();
}

void AssetAPI::Update(f64 frametime)
{
    PROFILE(AssetAPI_Update);

    for(size_t i = 0; i < providers.size(); ++i)
        providers[i]->Update(frametime);

    // Proceed with ready transfers.
    if (readyTransfers.size() > 0)
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
    
    // Proceed with ready sub asset transfers.
    if (readySubTransfers.size() > 0)
    {
        // readySubTransfers contains sub asset transfers to loaded bundles. The sub asset loading cannot be completed in RequestAsset
        // as it would trigger signals before the calling code can receive and hook to the AssetTransfer. We delay calling LoadSubAssetToTransfer
        // into this function so that all is hooked and loading can be done normally. This is very similar to the above case for readyTransfers.
        for(size_t i = 0; i < readySubTransfers.size(); ++i)
        {
            AssetTransferPtr subTransfer = readySubTransfers[i].subAssetTransfer;
            LoadSubAssetToTransfer(subTransfer, readySubTransfers[i].parentBundleRef, subTransfer->source.ref);
            subTransfer.reset();
        }
        readySubTransfers.clear();
    }
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

std::map<QString, QString> ParseAssetRefArgs(const QString &url, QString *body)
{
    std::map<QString, QString> keyValues;

    int paramsStartIndex = url.indexOf("?");
    if (paramsStartIndex == -1)
    {
        if (body)
            *body = url;
        return keyValues;
    }
    QString args = url.mid(paramsStartIndex+1);
    if (body)
        *body = url.left(paramsStartIndex);
    QStringList params = args.split("&");
    foreach(QString param, params)
    {
        QStringList keyValue = param.split("=");
        if (keyValue.size() == 0 || keyValue.size() > 2)
            continue; // Silently ignore malformed data.
        QString key = keyValue[0].trimmed();
        QString value = keyValue[1].trimmed();

        keyValues[key] = value;
    }
    return keyValues;
}

AssetTransferMap::iterator AssetAPI::FindTransferIterator(QString assetRef)
{
    return currentTransfers.find(assetRef);
}

AssetTransferMap::const_iterator AssetAPI::FindTransferIterator(QString assetRef) const
{
    return currentTransfers.find(assetRef);
}

AssetTransferMap::iterator AssetAPI::FindTransferIterator(IAssetTransfer *transfer)
{
    if (!transfer)
        return currentTransfers.end();

    for(AssetTransferMap::iterator iter = currentTransfers.begin(); iter != currentTransfers.end(); ++iter)
        if (iter->second.get() == transfer)
            return iter;

    return currentTransfers.end();
}

AssetTransferMap::const_iterator AssetAPI::FindTransferIterator(IAssetTransfer *transfer) const
{
    if (!transfer)
        return currentTransfers.end();

    for(AssetTransferMap::const_iterator iter = currentTransfers.begin(); iter != currentTransfers.end(); ++iter)
        if (iter->second.get() == transfer)
            return iter;

    return currentTransfers.end();
}

void AssetAPI::AssetTransferCompleted(IAssetTransfer *transfer_)
{
    PROFILE(AssetAPI_AssetTransferCompleted);
    
    assert(transfer_);
    
    // At this point, the transfer can originate from several different things:
    // 1) It could be a real AssetTransfer from a real AssetProvider.
    // 2) It could be an AssetTransfer to an Asset that was already downloaded before, in which case transfer_->asset is already filled and loaded at this point.
    // 3) It could be an AssetTransfer that was fulfilled from the disk cache, in which case no AssetProvider was invoked to get here. (we used the readyTransfers queue for this).
        
    AssetTransferPtr transfer = transfer_->shared_from_this(); // Elevate to a SharedPtr immediately to keep at least one ref alive of this transfer for the duration of this function call.
    //LogDebug("Transfer of asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" succeeded.");

    // This is a duplicated transfer to an asset that has already been previously loaded. Only signal that the asset's been loaded and finish.
    if (dynamic_cast<VirtualAssetTransfer*>(transfer_) && transfer->asset && transfer->asset->IsLoaded()) 
    {
        transfer->EmitAssetDownloaded();
        transfer->EmitTransferSucceeded();
        pendingDownloadRequests.erase(transfer->source.ref);
        AssetTransferMap::iterator iter = FindTransferIterator(transfer.get());
        if (iter != currentTransfers.end())
            currentTransfers.erase(iter);
        return;
    }

    // We should be tracking this transfer in an internal data structure.
    AssetTransferMap::iterator iter = FindTransferIterator(transfer_);
    if (iter == currentTransfers.end())
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    // Transfer is for an asset bundle.
    AssetBundleMonitorMap::iterator bundleIter = bundleMonitors.find(transfer->source.ref);
    if (bundleIter != bundleMonitors.end())
    {
        AssetBundlePtr assetBundle = CreateNewAssetBundle(transfer->assetType, transfer->source.ref);
        if (assetBundle)
        {
            // Hook to the success and fail signals. They can either be emitted when we load the asset below or after asynch loading.
            connect(assetBundle.get(), SIGNAL(Loaded(IAssetBundle*)), this, SLOT(AssetBundleLoadCompleted(IAssetBundle*)), Qt::UniqueConnection);
            connect(assetBundle.get(), SIGNAL(Failed(IAssetBundle*)), this, SLOT(AssetBundleLoadFailed(IAssetBundle*)), Qt::UniqueConnection);

            // Cache the bundle.
            QString bundleDiskSource = transfer->DiskSource(); // The asset provider may have specified an explicit filename to use as a disk source.
            if (transfer->CachingAllowed() && transfer->rawAssetData.size() > 0 && assetCache)
                bundleDiskSource = assetCache->StoreAsset(&transfer->rawAssetData[0], transfer->rawAssetData.size(), transfer->source.ref);
            assetBundle->SetDiskSource(bundleDiskSource);

            // The bundle has now been downloaded and cached (if allowed by policy).
            transfer->EmitAssetDownloaded();

            // Load the bundle assets from the transfer data or cache.
            // 1) Try to load from above disk source.
            // 2) If disk source deserialization fails, try in memory loading if bundle allows it with RequiresDiskSource.
            // Note: Be careful not to use bundleIter after DeserializeFromDiskSource is called, as it can righ away emit
            // Loaded and we end up to AssetBundleLoadCompleted that will remove this bundleIter from bundleMonitors map.
            bool success = assetBundle->DeserializeFromDiskSource();
            if (!success && !assetBundle->RequiresDiskSource())
            {
                const u8 *bundleData = (transfer->rawAssetData.size() > 0 ? &transfer->rawAssetData[0] : 0);
                if (bundleData)
                    success = assetBundle->DeserializeFromData(bundleData, transfer->rawAssetData.size());
            }

            // If all of the above returned false, this means asset could not be loaded.
            // Call AssetLoadFailed for the bundle to propagate this information to the waiting sub asset transfers. 
            // Propagating will be done automatically in the AssetBundleMonitor when the bundle fails.
            if (!success)
                AssetLoadFailed(transfer->asset->Name());
        }
        else
        {
            // Note that this is monitored by the AssetBundleMonitor and it fail all sub asset transfers at that point.
            // These child transfers are not in the currentTransfers map so we don't have to do cleanup there.
            QString error("AssetAPI: Failed to create new asset bundle of type \"" + transfer->assetType + "\" and name \"" + transfer->source.ref + "\"");
            LogError(error);
            transfer->EmitAssetFailed(error);
            
            // Cleanup
            currentTransfers.erase(iter);
            bundleMonitors.erase(bundleIter);
            return;
        }
    }
    // Transfer is for a normal asset.
    else
    {
        // We've finished an asset data download, now create an actual instance of an asset of that type if it did not exist already
        if (!transfer->asset)
            transfer->asset = CreateNewAsset(transfer->assetType, transfer->source.ref);
        if (!transfer->asset)
        {
            QString error("AssetAPI: Failed to create new asset of type \"" + transfer->assetType + "\" and name \"" + transfer->source.ref + "\"");
            LogError(error);
            transfer->EmitAssetFailed(error);
            return;
        }

        // Connect to Loaded() signal of the asset to be able to notify any dependent assets
        connect(transfer->asset.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnAssetLoaded(AssetPtr)), Qt::UniqueConnection);

        // Save this asset to cache, and find out which file will represent a cached version of this asset.
        QString assetDiskSource = transfer->DiskSource(); // The asset provider may have specified an explicit filename to use as a disk source.
        if (transfer->CachingAllowed() && transfer->rawAssetData.size() > 0 && assetCache)
            assetDiskSource = assetCache->StoreAsset(&transfer->rawAssetData[0], transfer->rawAssetData.size(), transfer->source.ref);

        // If disksource is still empty, forcibly look up if the asset exists in the cache now.
        if (assetDiskSource.isEmpty() && assetCache)
            assetDiskSource = assetCache->FindInCache(transfer->source.ref);
        
        // Save for the asset the storage and provider it came from.
        transfer->asset->SetDiskSource(assetDiskSource.trimmed());
        transfer->asset->SetDiskSourceType(transfer->diskSourceType);
        transfer->asset->SetAssetStorage(transfer->storage.lock());
        transfer->asset->SetAssetProvider(transfer->provider.lock());

        // Tell everyone this transfer has now been downloaded. Note that when this signal is fired, the asset dependencies may not yet be loaded.
        transfer->EmitAssetDownloaded();

        bool success = false;
        const u8 *data = (transfer->rawAssetData.size() > 0 ? &transfer->rawAssetData[0] : 0);
        if (data)
            success = transfer->asset->LoadFromFileInMemory(data, transfer->rawAssetData.size());
        else
            success = transfer->asset->LoadFromFile(transfer->asset->DiskSource());

        // If the load from either of in memory data or file data failed, update the internal state.
        // Otherwise the transfer will be left dangling in currentTransfers. For successful loads
        // we do no need to call AssetLoadCompleted because success can mean asynchronous loading,
        // in which case the call will arrive once the asynchronous loading is completed.
        if (!success)
            AssetLoadFailed(transfer->asset->Name());
    }
}

void AssetAPI::AssetTransferFailed(IAssetTransfer *transfer, QString reason)
{
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

void AssetAPI::AssetTransferAborted(IAssetTransfer *transfer)
{
    if (!transfer)
        return;
        
    // Don't log any errors for aborter transfers. This is unwanted spam when we disconnect 
    // from a server and have x amount of pending transfers that get aborter.
    AssetTransferMap::iterator iter = currentTransfers.find(transfer->source.ref);
    
    transfer->EmitAssetFailed("Transfer aborted.");   

    // Propagate the failure of this asset transfer to all assets which depend on this asset.
    std::vector<AssetPtr> dependents = FindDependents(transfer->source.ref);
    for(size_t i = 0; i < dependents.size(); ++i)
    {
        AssetTransferPtr dependentTransfer = GetPendingTransfer(dependents[i]->Name());
        if (dependentTransfer)
            AssetTransferAborted(dependentTransfer.get());
    }

    pendingDownloadRequests.erase(transfer->source.ref);
    if (iter != currentTransfers.end())
        currentTransfers.erase(iter);
}

void AssetAPI::AssetLoadCompleted(const QString assetRef)
{
    PROFILE(AssetAPI_AssetLoadCompleted);

    AssetPtr asset;
    AssetTransferMap::const_iterator iter = FindTransferIterator(assetRef);
    AssetMap::iterator iter2 = assets.find(assetRef);
    
    // Check for new transfer: not in the assets map yet
    if (iter != currentTransfers.end())
        asset = iter->second->asset;
    // Check for a reload: is in the known asset map.
    if (!asset.get() && iter2 != assets.end())
    {
        /** The above transfer might have been found but its IAsset can be null
            in the case that this is a IAsset created by cloning. The code that 
            requested the clone/generated ref might have created the above valid transfer
            but it might have failed, resulting in a null IAsset, if the clone was not made
            before the request. */
        asset = iter2->second;
    }

    if (asset.get())
    {
        asset->LoadCompleted();

        // Add to watch this path for changed, note this does nothing if the path is already added
        // so we should not be having duplicate paths and/or double emits on changes.
        const QString diskSource = asset->DiskSource();
        if (diskSourceChangeWatcher && !diskSource.isEmpty())
        {
            PROFILE(AssetAPI_AssetLoadCompleted_DiskWatcherSetup);

            // If available, check the storage whether assets loaded from it should be live-updated.
            
            // By default disable liveupdate for all assets which come outside any known Tundra asset storage.
            // This is because the feature is most likely not used, and setting up the watcher below consumes
            // system resources and CPU time. To enable the disk watcher, make sure that the asset comes from
            // a storage known to Tundra and set liveupdate==true for that asset storage.
            // Note that this means that also local assets outside all storages with absolute path names like 
            // 'C:\mypath\asset.png' will always have liveupdate disabled.
            AssetStoragePtr storage = asset->GetAssetStorage();
            if (storage)
            {
                bool shouldLiveUpdate = storage->HasLiveUpdate();

                // Localassetprovider implements its own watcher. Therefore only add paths which refer to the assetcache to the AssetAPI watcher
                if (shouldLiveUpdate && (!assetCache || !diskSource.startsWith(assetCache->CacheDirectory(), Qt::CaseInsensitive)))
                    shouldLiveUpdate = false;

                if (shouldLiveUpdate)
                {
                    diskSourceChangeWatcher->removePath(diskSource);
                    diskSourceChangeWatcher->addPath(diskSource);
                }
            }
        }

        PROFILE(AssetAPI_AssetLoadCompleted_ProcessDependencies);

        // If this asset depends on any other assets, we have to make asset requests for those assets as well (and all assets that they refer to, and so on).
        RequestAssetDependencies(asset);

        // If we don't have any outstanding dependencies for the transfer, succeed and remove the transfer.
        // Find the iter again as AssetDependenciesCompleted can be called from OnAssetLoaded for synchronous (eg. local://) loads.
        iter = FindTransferIterator(assetRef);
        if (iter != currentTransfers.end() && !HasPendingDependencies(asset))
            AssetDependenciesCompleted(iter->second);
    }
    else
        LogError("AssetAPI: Asset \"" + assetRef + "\" load completed, but no corresponding transfer or existing asset is being tracked!");
}

void AssetAPI::AssetLoadFailed(const QString assetRef)
{
    AssetTransferMap::iterator iter = FindTransferIterator(assetRef);
    AssetMap::const_iterator iter2 = assets.find(assetRef);

    if (iter != currentTransfers.end())
    {
        AssetTransferPtr transfer = iter->second;
        transfer->EmitAssetFailed("Failed to load " + transfer->assetType + " '" + transfer->source.ref + "' from asset data.");
        currentTransfers.erase(iter);
    }
    else if (iter2 != assets.end())
        LogError("AssetAPI: Failed to reload asset '" + iter2->second->Name() + "'");
    else
        LogError("AssetAPI: Asset '" + assetRef + "' load failed, but no corresponding transfer or existing asset is being tracked!");
}

void AssetAPI::AssetBundleLoadCompleted(IAssetBundle *bundle)
{
    LogDebug("Asset bundle load completed: " + bundle->Name());
    
    // First erase the transfer as the below sub asset loading can trigger new
    // dependency asset requests to the bundle. In this case we want to load them from the
    // completed asset bundle not add them to the monitors queue.
    AssetTransferMap::iterator bundleTransferIter = FindTransferIterator(bundle->Name());
    if (bundleTransferIter != currentTransfers.end())
        currentTransfers.erase(bundleTransferIter);
    else
        LogWarning("AssetAPI: Asset bundle load completed, but transfer was not tracked: " + bundle->Name());
    
    AssetBundleMonitorMap::iterator monitorIter = bundleMonitors.find(bundle->Name());
    if (monitorIter != bundleMonitors.end())
    {
        // We have no need for the monitor anymore as the AssetBundle has been added to the assetBundles map earlier for reuse.
        AssetBundleMonitorPtr bundleMonitor = (*monitorIter).second;
        std::vector<AssetTransferPtr> subTransfers = bundleMonitor->SubAssetTransfers();
        bundleMonitors.erase(monitorIter);
        
        // Start the load process for all sub asset transfers now. From here on out the normal asset request flow should followed.
        for (std::vector<AssetTransferPtr>::iterator subIter = subTransfers.begin(); subIter != subTransfers.end(); ++subIter)
            LoadSubAssetToTransfer((*subIter), bundle, (*subIter)->source.ref);
    }
    else
        LogWarning("AssetAPI: Asset bundle load completed, but bundle monitor cannot be found: " + bundle->Name());
}

void AssetAPI::AssetBundleLoadFailed(IAssetBundle *bundle)
{
    AssetLoadFailed(bundle->Name());
    
    // We have no need for the monitor anymore as the AssetBundle has been added to the assetBundles map earlier for reuse.
    AssetBundleMonitorMap::iterator monitorIter = bundleMonitors.find(bundle->Name());
    if (monitorIter != bundleMonitors.end())
        bundleMonitors.erase(monitorIter);
}

void AssetAPI::AssetUploadTransferCompleted(IAssetUploadTransfer *uploadTransfer)
{
    QString assetRef = uploadTransfer->AssetRef();
    
    // Clear our cache of this data.
    /// @note We could actually update our cache with the same version of the asset that we just uploaded,
    /// to avoid downloading what we just uploaded. That can be implemented later.
    if (assetCache)
        assetCache->DeleteAsset(assetRef);
    
    // If we have the asset (with possible old contents) in memory, unload it now
    {
        AssetPtr asset = GetAsset(assetRef);
        if (asset && asset->IsLoaded())
            asset->Unload();
    }
    
    uploadTransfer->EmitTransferCompleted();
    
    emit AssetUploaded(assetRef);
    
    // We've completed an asset upload transfer. See if there is an asset download transfer that is waiting
    // for this upload to complete. 
    
    currentUploadTransfers.erase(assetRef); // Note: this might kill the 'transfer' ptr if we were the last one to hold on to it. Don't dereference transfer below this.
    PendingDownloadRequestMap::iterator iter = pendingDownloadRequests.find(assetRef);
    if (iter != pendingDownloadRequests.end())
    {
        PendingDownloadRequest req = iter->second;

        AssetTransferPtr transfer = RequestAsset(req.assetRef, req.assetType);
        if (!transfer)
            return; ///\todo Evaluate the path to take here.
        connect(transfer.get(), SIGNAL(Downloaded(IAssetTransfer*)), req.transfer.get(), SIGNAL(Downloaded(IAssetTransfer*)), Qt::UniqueConnection);
        connect(transfer.get(), SIGNAL(Succeeded(AssetPtr)), req.transfer.get(), SIGNAL(Succeeded(AssetPtr)), Qt::UniqueConnection);
        connect(transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), req.transfer.get(), SIGNAL(Failed(IAssetTransfer*, QString)), Qt::UniqueConnection);
    }
}

void AssetAPI::AssetDependenciesCompleted(AssetTransferPtr transfer)
{    
    PROFILE(AssetAPI_AssetDependenciesCompleted);
    // Emit success for this transfer
    transfer->EmitTransferSucceeded();
    
    // This asset transfer has finished - remove it from the internal list of ongoing transfers.
    AssetTransferMap::iterator iter = FindTransferIterator(transfer.get());
    if (iter != currentTransfers.end())
        currentTransfers.erase(iter);
    else // Even if we didn't know about this transfer, just print a warning and continue execution here nevertheless.
        LogError("AssetAPI: Asset \"" + transfer->assetType + "\", name \"" + transfer->source.ref + "\" transfer finished, but no corresponding AssetTransferPtr was tracked by AssetAPI!");

    pendingDownloadRequests.erase(transfer->source.ref);
}

void AssetAPI::NotifyAssetDependenciesChanged(AssetPtr asset)
{
    PROFILE(AssetAPI_NotifyAssetDependenciesChanged);

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
    PROFILE(AssetAPI_RequestAssetDependencies);
    // Make sure we have most up-to-date internal view of the asset dependencies.
    NotifyAssetDependenciesChanged(asset);

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        AssetReference ref = refs[i];
        if (ref.ref.isEmpty())
            continue;

        AssetPtr existing = GetAsset(ref.ref);
        if (!existing || !existing->IsLoaded())
        {
//            LogDebug("Asset " + asset->ToString() + " depends on asset " + ref.ref + " (type=\"" + ref.type + "\") which has not been loaded yet. Requesting..");
            RequestAsset(ref);
        }
    }
}

void AssetAPI::RemoveAssetDependencies(QString asset)
{
    PROFILE(AssetAPI_RemoveAssetDependencies);
    for(size_t i = 0; i < assetDependencies.size(); ++i)
        if (QString::compare(assetDependencies[i].first, asset, Qt::CaseInsensitive) == 0)
        {
            assetDependencies.erase(assetDependencies.begin() + i);
            --i;
        }
}

std::vector<AssetPtr> AssetAPI::FindDependents(QString dependee)
{
    PROFILE(AssetAPI_FindDependents);

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

int AssetAPI::NumPendingDependencies(AssetPtr asset) const
{
    PROFILE(AssetAPI_NumPendingDependencies);
    int numDependencies = 0;

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        QString ref = refs[i].ref;
        if (ref.isEmpty())
            continue;

        // We silently ignore this dependency if the asset type in question is disabled.
        if (dynamic_cast<NullAssetFactory*>(GetAssetTypeFactory(GetResourceTypeFromAssetRef(refs[i])).get()))
            continue;

        AssetPtr existing = GetAsset(refs[i].ref);
        if (!existing)
        {
            // Not loaded, just mark the single one
            ++numDependencies;
        }
        else
        {
            // If asset is empty, count it as an unloaded dependency
            if (existing->IsEmpty())
                ++numDependencies;
            else
            {
                if (!existing->IsLoaded())
                    ++numDependencies;
                // Ask the dependencies of the dependency, we want all of the asset
                // down the chain to be loaded before we load the base asset
                // Note: if the dependency is unloaded, it may or may not be able to tell the dependencies correctly
                numDependencies += NumPendingDependencies(existing);
            }
        }
    }

    return numDependencies;
}

bool AssetAPI::HasPendingDependencies(AssetPtr asset) const
{
    PROFILE(AssetAPI_HasPendingDependencies);

    std::vector<AssetReference> refs = asset->FindReferences();
    for(size_t i = 0; i < refs.size(); ++i)
    {
        if (refs[i].ref.isEmpty())
            continue;

        // We silently ignore this dependency if the asset type in question is disabled.
        if (dynamic_cast<NullAssetFactory*>(GetAssetTypeFactory(GetResourceTypeFromAssetRef(refs[i])).get()))
            continue;

        AssetPtr existing = GetAsset(refs[i].ref);
        if (!existing) // Not loaded, just mark the single one
            return true;
        else
        {            
            if (existing->IsEmpty())
                return true; // If asset is empty, count it as an unloaded dependency
            else
            {
                if (!existing->IsLoaded())
                    return true;
                // Ask the dependencies of the dependency, we want all of the asset
                // down the chain to be loaded before we load the base asset
                // Note: if the dependency is unloaded, it may or may not be able to tell the dependencies correctly
                bool dependencyHasDependencies = HasPendingDependencies(existing);
                if (dependencyHasDependencies)
                    return true;
            }
        }
    }

    return false;
}

void AssetAPI::HandleAssetDiscovery(const QString &assetRef, const QString &assetType)
{
    HandleAssetDiscovery(assetRef, assetType, AssetStoragePtr());
}

void AssetAPI::HandleAssetDiscovery(const QString &assetRef, const QString &assetType, AssetStoragePtr storage)
{
    AssetPtr existing = GetAsset(assetRef);
    // If asset did not exist, create new empty asset
    if (!existing)
    {
        // If assettype is empty, guess it
        QString newType = assetType;
        if (newType.isEmpty())
            newType = GetResourceTypeFromAssetRef(assetRef);
        CreateNewAsset(newType, assetRef, storage);
    }
    // If asset exists and is already loaded, forcibly request updated data
    else if (existing->IsLoaded())
        RequestAsset(assetRef, assetType, true);
}

void AssetAPI::HandleAssetDeleted(const QString &assetRef)
{
    // If the asset is unloaded, delete it from memory. If it is loaded, it might be in use, so do nothing
    AssetPtr existing = GetAsset(assetRef);
    if (!existing)
        return;
    if (!existing->IsLoaded())
        ForgetAsset(existing, false);
}

void AssetAPI::EmitAssetDeletedFromStorage(const QString &assetRef)
{
    emit AssetDeletedFromStorage(assetRef);
}

void AssetAPI::EmitAssetStorageAdded(AssetStoragePtr newStorage)
{
    // Connect to the asset storage's AssetChanged signal, so that we can create actual empty assets
    // from its refs whenever new assets are added to this storage from external sources.
    connect(newStorage.get(), SIGNAL(AssetChanged(QString, QString, IAssetStorage::ChangeType)),
        SLOT(OnAssetChanged(QString, QString, IAssetStorage::ChangeType)), Qt::UniqueConnection);
    emit AssetStorageAdded(newStorage);
}

QMap<QString, QString> AssetAPI::ParseAssetStorageString(QString storageString)
{
    storageString = storageString.trimmed();
    // Swallow the right-most ';' if it exists to allow both forms "http://www.server.com/" and "http://www.server.com/;" below,
    // although the latter is somewhat odd form.
    if (storageString.endsWith(";")) 
        storageString = storageString.left(storageString.length()-1);

    // Treat simple strings of form "http://myserver.com/" as "src=http://myserver.com/".
    if (storageString.indexOf(';') == -1 && storageString.indexOf('=') == -1)
        storageString = "src=" + storageString;

    QMap<QString, QString> m;
    QStringList items = storageString.split(";", QString::SkipEmptyParts);
    foreach(QString str, items)
    {
        QStringList keyValue = str.split("=");
        if (keyValue.count() > 2 || keyValue.count() == 0)
        {
            LogError("Failed to parse asset storage string \"" + str + "\"!");
            return QMap<QString, QString>();
        }
        if (keyValue.count() == 1)
            keyValue.push_back("1");
        m[keyValue[0]] = keyValue[1];
    }
    return m;
}

void AssetAPI::OnAssetLoaded(AssetPtr asset)
{
    PROFILE(AssetAPI_OnAssetLoaded);

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
            if (!HasPendingDependencies(dependent))
                AssetDependenciesCompleted(transfer);
        }
    }
}

void AssetAPI::OnAssetDiskSourceChanged(const QString &path_)
{
    QDir path(path_);
    for(AssetMap::iterator iter = assets.begin(); iter != assets.end(); ++iter)
    {
        QString assetDiskSource = iter->second->DiskSource();
        if (!assetDiskSource.isEmpty() && QDir(assetDiskSource) == path && QFile::exists(assetDiskSource))
        {
            AssetPtr asset = iter->second;
            AssetStoragePtr storage = asset->GetAssetStorage();
            if (storage)
            {
                if (storage->HasLiveUpdate())
                {
                    LogInfo("AssetAPI: Detected file changes in '" + path_ + "', reloading asset.");
                    bool success = asset->LoadFromCache();
                    if (!success)
                        LogError("Failed to reload changed asset \"" + asset->ToString() + "\" from file \"" + path_ + "\"!");
                    else
                        LogDebug("Reloaded changed asset \"" + asset->ToString() + "\" from file \"" + path_ + "\".");
                }
                
                emit AssetDiskSourceChanged(asset);
            }
            else
                LogError("Detected file change for a storageless asset " + asset->Name());
        }
    }
}

///\todo Delete this whole function and logic when OnAssetChanged is implemented
/*
void AssetAPI::OnAssetStorageRefsChanged(AssetStoragePtr storage)
{
    QStringList refs = storage->GetAllAssetRefs();
    for(int i = 0; i < refs.size(); ++i)
    {
        // If the asset does not exist at all, create a new empty asset.
        // However, if the asset already exists, do not refresh its data now (as we may be getting a huge amount of refs)
        if (!GetAsset(refs[i]))
            // Use optimized discovery: the storage does not have to be looked up as it is known
            HandleAssetDiscovery(refs[i], "", storage);
    }
}
*/

void AssetAPI::OnAssetChanged(QString localName, QString diskSource, IAssetStorage::ChangeType change)
{
    PROFILE(AssetAPI_OnAssetChanged);

    IAssetStorage *storage = dynamic_cast<IAssetStorage *>(sender());
    assert(storage);
    if (!storage)
        return;

    QString assetRef = storage->GetFullAssetURL(localName);
    QString assetType = GetResourceTypeFromAssetRef(assetRef);
    AssetPtr existing = GetAsset(assetRef);
    if (change == IAssetStorage::AssetCreate && existing)
    {
        LogDebug("AssetAPI: Received AssetCreate notification for existing and loaded asset " + assetRef + ". Handling this as AssetModify.");
        change = IAssetStorage::AssetModify;
    }

    switch(change)
    {
    case IAssetStorage::AssetCreate:
        {
            AssetPtr asset = CreateNewAsset(assetType, assetRef, storage->shared_from_this());
            if (asset && !diskSource.isEmpty())
            {
                asset->SetDiskSource(diskSource);
                //bool cached = (assetCache && diskSource.contains(GetAssetCache()->GetCacheDirectory(), Qt::CaseInsensitive));
                //asset->SetDiskSourceType(cached ? IAsset::Cached : IAsset::Original);
            }
        }
        break;
    case IAssetStorage::AssetModify:
        if (existing)
        {
            ///\todo Profile performance difference between LoadFromCache and RequestAsset
            if (existing->IsLoaded()) // Note: exists->LoadFromCache() could be used here as well.
                RequestAsset(assetRef, assetType, true); // If asset exists and is already loaded, forcibly request updated data
            else
                LogDebug("AssetAPI: Ignoring AssetModify notification for unloaded asset " + assetRef + ".");
        }
        else
            LogWarning("AssetAPI: Received AssetModify notification for non-existing asset " + assetRef + ".");
        break;
    case IAssetStorage::AssetDelete:
        if (existing)
        {
            if (existing->IsLoaded()) // Deleted asset is currently loaded. Invalidate disk source, but do not forget asset.
                existing->SetDiskSource("");
            else
                ForgetAsset(existing, false); // The asset should be already deleted; do not delete disk source.
        }
        else
            LogWarning("AssetAPI: Received AssetDelete notification for non-existing asset " + assetRef + ".");
        break;
    default:
        assert(false);
        break;
    }
}

bool LoadFileToVector(const QString &filename, std::vector<u8> &dst)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    if (!file.isOpen())
    {
        LogError("AssetAPI::LoadFileToVector: Failed to open file '" + filename + "' for reading.");
        return false;
    }
    qint64 fileSize = file.size();
    if (fileSize <= 0)
    {
        LogError("AssetAPI::LoadFileToVector: Failed to read file '" + filename + "' of " + fileSize + " bytes in size.");
        return false;
    }
    dst.resize(fileSize);
    qint64 numRead = file.read((char*)&dst[0], fileSize);
    if (numRead < fileSize)
    {
        LogError("AssetAPI::LoadFileToVector: Failed to read " + QString::number(numRead) + " bytes from file '" + filename + "'.");
        return false;
    }

    return true;
}

namespace
{
    bool IsFileOfType(const QString &filename, const char **suffixes, int numSuffixes)
    {
        for(int i = 0;i < numSuffixes; ++i)
            if (filename.endsWith(suffixes[i], Qt::CaseInsensitive))
                return true;

        return false;
    }
}

QString AssetAPI::GetResourceTypeFromAssetRef(const AssetReference &ref) const
{
    QString type = ref.type.trimmed();
    if (!type.isEmpty())
        return type;
    return GetResourceTypeFromAssetRef(ref.ref);
}

QString AssetAPI::GetResourceTypeFromAssetRef(QString assetRef) const
{
    QString filenameParsed;
    QString subAssetFilename;
    ParseAssetRef(assetRef, 0, 0, 0, 0, 0, 0, &filenameParsed, &subAssetFilename);
    if (!subAssetFilename.isEmpty())
        filenameParsed = subAssetFilename;
    QString filename = filenameParsed.trimmed();

    // Query all registered asset factories if they provide this asset type.
    for(size_t i=0; i<assetTypeFactories.size(); ++i)
    {
        // Note that we cannot ask endsWith from 'Binary' factory as the extension
        // is an empty string. It will always return true and this factory should 
        // only be defaulted to if nothing else can provide the queried file extension.
        if (assetTypeFactories[i]->Type() == "Binary")
            continue;

        foreach (QString extension, assetTypeFactories[i]->TypeExtensions())
            if (filename.endsWith(extension, Qt::CaseInsensitive))
                return assetTypeFactories[i]->Type();
    }

    // Query all registered bundle factories if they provide this asset type.
    for(size_t i=0; i<assetBundleTypeFactories.size(); ++i)
        foreach (QString extension, assetBundleTypeFactories[i]->TypeExtensions())
            if (filename.endsWith(extension, Qt::CaseInsensitive))
                return assetBundleTypeFactories[i]->Type();
                
    /** @todo Make these hardcoded ones go away and move to the provider when provided (like above). 
        Seems the resource types have leaked here without the providers being in the code base.
        Where ever the code might be, remove these once the providers have been updated to return
        the type extensions correctly. */
    if (filename.endsWith(".qml", Qt::CaseInsensitive) || filename.endsWith(".qmlzip", Qt::CaseInsensitive))
        return "QML";
    if (filename.endsWith(".pdf", Qt::CaseInsensitive))
        return "PdfAsset";
    const char *openDocFileTypes[] = { ".odt", ".doc", ".rtf", ".txt", ".docx", ".docm", ".ods", ".xls", ".odp", ".ppt", ".odg" };
    if (IsFileOfType(filename, openDocFileTypes, NUMELEMS(openDocFileTypes)))
        return "DocAsset";

    // Could not resolve the asset extension to any registered asset factory. Return Binary type.
    return "Binary";
}

QString AssetAPI::SanitateAssetRef(const QString& input)
{
    QString ret = input;
    if (ret.contains('$'))
        return ret; // Already sanitated

    ret.replace(":", "$1");
    ret.replace("/", "$2");
    ret.replace("\\", "$3");
    ret.replace("*", "$4");
    ret.replace("?", "$5");
    ret.replace("\"", "$6");
    ret.replace("'", "$7");
    ret.replace("<", "$8");
    ret.replace(">", "$9");
    ret.replace("|", "$0");
    return ret;
}

QString AssetAPI::DesanitateAssetRef(const QString& input)
{
    QString ret = input;
    ret.replace("$1", ":");
    ret.replace("$2", "/");
    ret.replace("$3", "\\");
    ret.replace("$4", "*");
    ret.replace("$5", "?");
    ret.replace("$6", "\"");
    ret.replace("$7", "'");
    ret.replace("$8", "<");
    ret.replace("$9", ">");
    ret.replace("$0", "|");
    return ret;
}

std::string AssetAPI::SanitateAssetRef(const std::string& input)
{
    return SanitateAssetRef(QString::fromStdString(input)).toStdString();
}

std::string AssetAPI::DesanitateAssetRef(const std::string& input)
{
    return DesanitateAssetRef(QString::fromStdString(input)).toStdString();
}

bool CopyAssetFile(const QString &sourceFile, const QString &destFile)
{
    assert(!sourceFile.trimmed().isEmpty());
    assert(!destFile.trimmed().isEmpty());

    QFile asset_in(sourceFile);
    if (!asset_in.open(QFile::ReadOnly))
    {
        LogError("Could not open input asset file \"" + sourceFile + "\"");
        return false;
    }

    QByteArray bytes = asset_in.readAll();
    asset_in.close();
    
    QFile asset_out(destFile);
    if (!asset_out.open(QFile::WriteOnly))
    {
        LogError("Could not open output asset file \"" + destFile + "\"");
        return false;
    }

    asset_out.write(bytes);
    asset_out.close();
    
    return true;
}

bool SaveAssetFromMemoryToFile(const u8 *data, size_t numBytes, const QString &destFile)
{
    assert(data);
    assert(!destFile.trimmed().isEmpty());

    QFile asset_out(destFile);
    if (!asset_out.open(QFile::WriteOnly))
    {
        LogError("Could not open output asset file \"" + destFile + "\"");
        return false;
    }

    asset_out.write((const char *)data, numBytes);
    asset_out.close();
    
    return true;
}
