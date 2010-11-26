// For conditions of distribution and use, see copyright notice in license.txt

#include "AssetAPI.h"
#include "Framework.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "IAssetStorage.h"
#include "AssetServiceInterface.h"
#include "AssetProviderInterface.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"
#include "EventManager.h"
#include "ResourceInterface.h"
#include "CoreException.h"
#include "IAssetTypeFactory.h"
#include "../AssetModule/AssetEvents.h"

DEFINE_POCO_LOGGING_FUNCTIONS("Asset")

using namespace Foundation;

AssetAPI::AssetAPI(Foundation::Framework *owner)
:framework(owner)
{
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

std::vector<Foundation::AssetProviderPtr> AssetAPI::GetAssetProviders() const
{
    ServiceManagerPtr service_manager = framework->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        throw Exception("Unagle to get AssetServiceInterface!");

    std::vector<Foundation::AssetProviderPtr> providers = asset_service->Providers();
    return providers;
}

AssetStoragePtr AssetAPI::GetAssetStorage(const QString &name) const
{
    foreach(Foundation::AssetProviderPtr provider, GetAssetProviders())
        foreach(AssetStoragePtr storage, provider->GetStorages())
            if (storage->Name() == name)
                return storage;

    return AssetStoragePtr();
}

std::vector<AssetStoragePtr> AssetAPI::GetAssetStorages() const
{
    std::vector<AssetStoragePtr> storages;

    std::vector<Foundation::AssetProviderPtr> providers = GetAssetProviders();

    for(size_t i = 0; i < providers.size(); ++i)
    {
        std::vector<AssetStoragePtr> stores = providers[i]->GetStorages();
        storages.insert(storages.end(), stores.begin(), stores.end());
    }

    return storages;
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

AssetTransferPtr AssetAPI::RequestAsset(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());

    Foundation::AssetProviderPtr provider = GetProviderForAssetRef(assetRef, assetType);
    if (!provider.get())
    {
        LogError("AssetAPI::RequestAsset: Failed to find a provider for asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
        return AssetTransferPtr();
    }

    if (assetType == "Script") // NEW PATH: Uses asset providers directly.
    {
        AssetTransferPtr transfer = provider->RequestAsset(assetRef, assetType);
        if (!transfer.get())
        {
            LogError("AssetAPI::RequestAsset: Failed to request asset \"" + assetRef.toStdString() + "\", type: \"" + assetType.toStdString() + "\"");
            return AssetTransferPtr();
        }
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

Foundation::AssetProviderPtr AssetAPI::GetProviderForAssetRef(QString assetRef, QString assetType)
{
    assetType = assetType.trimmed();
    assetRef = assetRef.trimmed();

    if (assetType.length() == 0)
        assetType = GetResourceTypeFromResourceFileName(assetRef.toLower().toStdString().c_str());

    std::vector<Foundation::AssetProviderPtr> providers = GetAssetProviders();
    for(size_t i = 0; i < providers.size(); ++i)
        if (providers[i]->IsValidRef(assetRef.toStdString(), assetType.toStdString()))
            return providers[i];

    return Foundation::AssetProviderPtr();
}

void AssetAPI::RegisterAssetTypeFactory(AssetTypeFactoryPtr factory)
{
    AssetTypeFactoryPtr existingFactory = GetAssetTypeFactory(factory->Type());
    if (existingFactory.get())
        return; ///\todo Log out warning.

    assert(factory->Type() == factory->Type().trimmed());
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
            }
        }
    }

    return false;
}

QString AssetAPI::GuaranteeTrailingSlash(const QString &source)
{
    QString s = source.trimmed();
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

    return "";
    // Note: There's a separate OgreImageTextureResource which isn't handled above.
}
