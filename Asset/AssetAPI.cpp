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
#include "BinaryAssetFactory.h"
#include "../AssetModule/AssetEvents.h"
#include <QDir>

DEFINE_POCO_LOGGING_FUNCTIONS("Asset")

using namespace Foundation;

AssetAPI::AssetAPI(Foundation::Framework *owner)
:framework(owner)
{
    // The Asset API always understands at least this single built-in asset type "Binary".
    // You can use this type to request asset data as binary, without generating any kind of in-memory representation or loading for it.
    // Your module/component can then parse the content in a custom way.
    RegisterAssetTypeFactory(AssetTypeFactoryPtr(new BinaryAssetFactory("Binary")));
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

void AssetAPI::ForgetAllAssets()
{
    assets.clear();
    currentTransfers.clear();
}

AssetTransferPtr AssetAPI::RequestAsset(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetRef.length() == 0)
    {
        // Removed this print - seems like a bad idea to print out this warning, since there are lots of scenes with null assetrefs.
        // Perhaps have a verbose log channel for these kinds of sanity checks.
//        LogError("AssetAPI::RequestAsset: Request by empty url \"\" of type \"" + assetType.toStdString() + " received!");
        return AssetTransferPtr();
    }

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());

    AssetProviderPtr provider = GetProviderForAssetRef(assetRef, assetType);
    if (!provider.get())
    {
        LogError("AssetAPI::RequestAsset: Failed to find a provider for asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
        return AssetTransferPtr();
    }

    if (assetType == "Script" || assetType == "Terrain")// || assetType == "Texture") // NEW PATH: Uses asset providers directly.
    {
        AssetTransferPtr transfer = provider->RequestAsset(assetRef, assetType);
        if (!transfer.get())
        {
            LogError("AssetAPI::RequestAsset: Failed to request asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
            return AssetTransferPtr();
        }
        connect(transfer.get(), SIGNAL(Downloaded(IAssetTransfer*)), this, SLOT(AssetDownloaded(IAssetTransfer*)));
        return transfer;
    }
    else // OLD PATH: Uses the event-based asset managers.
    {
        // Find an asset provider that can take in the request for the desired assetRef.
        AssetTransferPtr transfer = AssetTransferPtr(new IAssetTransfer()); ///\todo Don't new here, but have the asset provider new it.
        transfer->source = AssetReference(assetRef/*, assetType*/);
        // (the above leaks, but not fixing before the above todo is properly implemented -jj.)

        // Get the asset service. \todo This will be removed. There will be no asset service. -jj.
        AssetServiceInterface *asset_service = framework->GetService<AssetServiceInterface>();
        if (!asset_service)
        {
            LogError("Asset service doesn't exist.");
            return AssetTransferPtr();
        }

        Foundation::RenderServiceInterface *renderer = framework->GetService<Foundation::RenderServiceInterface>();
        if (!renderer)
        {
            LogError("Renderer service doesn't exist.");
            return AssetTransferPtr();
        }

        request_tag_t tag;

        // Depending on the asset type, we must request the asset from the Renderer or from the asset service.

        QString foundAssetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());
        if (foundAssetType != "")
            tag = renderer->RequestResource(assetRef.toStdString(), foundAssetType.toStdString());
        else
            tag = asset_service->RequestAsset(assetRef.toStdString(), assetType.toStdString());

        currentTransfers[tag] = transfer;

        return transfer;
    }
}

AssetTransferPtr AssetAPI::RequestAsset(const AssetReference &ref)
{
    return RequestAsset(ref.ref, ""/*ref.type*/);
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
    AssetPtr asset = factory->CreateEmptyAsset(name.toStdString().c_str());
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

bool AssetAPI::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    if (category_id == framework->GetEventManager()->QueryEventCategory("Asset"))
    {
        if (event_id == Asset::Events::ASSET_READY)
        {
            Asset::Events::AssetReady *assetReady = checked_static_cast<Asset::Events::AssetReady*>(data);
            std::map<request_tag_t, AssetTransferPtr>::iterator iter = currentTransfers.find(assetReady->tag_);
            if (iter != currentTransfers.end())
            {
                AssetTransferPtr transfer = iter->second;
                transfer->assetPtr = assetReady->asset_;
                assert(transfer);
                transfer->EmitAssetDownloaded();
                currentTransfers.erase(iter);
            }
        }
    }

    if (category_id == framework->GetEventManager()->QueryEventCategory("Resource"))
    {
        if (event_id == Resource::Events::RESOURCE_READY)
        {
            Resource::Events::ResourceReady *resourceReady = checked_static_cast<Resource::Events::ResourceReady*>(data);
            std::map<request_tag_t, AssetTransferPtr>::iterator iter = currentTransfers.find(resourceReady->tag_);
            if (iter != currentTransfers.end())
            {
                AssetTransferPtr transfer = iter->second;
                transfer->resourcePtr = resourceReady->resource_;
                assert(transfer);
                //! \todo Causes linker error in debug build, must be disabled for now
                //transfer->internalResourceName = QString::fromStdString(resourceReady->resource_->GetInternalName());
                transfer->EmitAssetLoaded();
                currentTransfers.erase(iter);
            }
        }
    }

    return false;
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

void AssetAPI::AssetDownloaded(IAssetTransfer *transfer)
{
    if (transfer->rawAssetData.size() == 0)
    {
        LogError("AssetAPI: Asset \"" + transfer->assetType.toStdString() + "\", name \"" + transfer->source.ref.toStdString() + "\" transfer finished: but data size was 0 bytes!");
        return;
    }

    // We've finished an asset data download, now create an actual instance of an asset of that type.
    transfer->asset = CreateNewAsset(transfer->assetType, transfer->source.ref);
    if (!transfer->asset.get())
    {
        LogError("AssetAPI: Failed to create new asset of type \"" + transfer->assetType.toStdString() + "\" and name \"" + transfer->source.ref.toStdString() + "\"");
        return;
    }

    bool success = transfer->asset->LoadFromFileInMemory(&transfer->rawAssetData[0], transfer->rawAssetData.size());
    if (!success)
    {
        LogError("AssetAPI: Failed to load asset of type \"" + transfer->assetType.toStdString() + "\" and name \"" + transfer->source.ref.toStdString() + "\" from asset data.");
        return;
    }

    // Remember the newly created asset in AssetAPI's internal data structure to allow clients to later fetch it without re-requesting it.
    assets.push_back(transfer->asset);

    ///\todo Specify the following flow better.

    transfer->EmitAssetDecoded();
    transfer->EmitAssetLoaded();
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
        return "OgreTexture";

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
