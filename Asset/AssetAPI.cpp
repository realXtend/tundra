// For conditions of distribution and use, see copyright notice in license.txt

#include "AssetAPI.h"
#include "Framework.h"
#include "IAssetTransfer.h"
#include "IAsset.h"
#include "AssetServiceInterface.h"
#include "RenderServiceInterface.h"
#include "LoggingFunctions.h"
#include "EventManager.h"
#include "ResourceInterface.h"
#include "CoreException.h"
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

namespace
{
    bool IsFileOfType(const QString &filename, const char **suffixes, int numSuffixes)
    {
        for(int i = 0;i < numSuffixes; ++i)
            if (filename.endsWith(suffixes[i]))
                return true;

        return false;
    }

    std::string GetResourceTypeFromResourceName(const char *name)
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

        return "";
        // Note: There's a separate OgreImageTextureResource which isn't handled above.
    }
}

std::vector<Foundation::AssetProviderPtr> AssetAPI::GetAssetProviders()
{
    ServiceManagerPtr service_manager = framework->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        throw Exception("Unagle to get AssetServiceInterface!");

    std::vector<Foundation::AssetProviderPtr> providers = asset_service->Providers();
    return providers;
}

std::vector<IAssetStorage*> AssetAPI::GetAssetStorages()
{
    std::vector<IAssetStorage*> storages;

    std::vector<Foundation::AssetProviderPtr> providers = GetAssetProviders();

    for(size_t i = 0; i < providers.size(); ++i)
    {
        std::vector<IAssetStorage*> stores = providers[i]->GetStorages();
        storages.insert(storages.end(), stores.begin(), stores.end());
    }

    return storages;
}

IAssetTransfer *AssetAPI::RequestAsset(QString assetRef, QString assetType)
{
    // Find an asset provider that can take in the request for the desired assetRef.
    IAssetTransfer *transfer = new IAssetTransfer(); ///\todo Don't new here, but have the asset provider new it.
    transfer->source = AssetReference(assetRef/*, assetType*/);
    // (the above leaks, but not fixing before the above todo is properly implemented -jj.)

    // Get the asset service. \todo This will be removed. There will be no asset service. -jj.
    AssetServiceInterface *asset_service = framework->GetService<AssetServiceInterface>();
    if (!asset_service)
    {
        LogError("Asset service doesn't exist.");
        return 0;
    }

    Foundation::RenderServiceInterface *renderer = framework->GetService<Foundation::RenderServiceInterface>();
    if (!renderer)
    {
        LogError("Renderer service doesn't exist.");
        return 0;
    }

    request_tag_t tag;

    // Depending on the asset type, we must request the asset from the Renderer or from the asset service.

    std::string foundAssetType = GetResourceTypeFromResourceName(assetRef.toLower().toStdString().c_str());
    if (foundAssetType != "")
        tag = renderer->RequestResource(assetRef.toStdString(), foundAssetType);
    else
        tag = asset_service->RequestAsset(assetRef.toStdString(), assetType.toStdString());

    currentTransfers[tag] = transfer;

    return transfer;
}

IAssetTransfer *AssetAPI::RequestAsset(const AssetReference &ref)
{
    return RequestAsset(ref.ref, ""/*ref.type*/);
}

bool AssetAPI::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    if (category_id == framework->GetEventManager()->QueryEventCategory("Asset"))
    {
        if (event_id == Asset::Events::ASSET_READY)
        {
            Asset::Events::AssetReady *assetReady = checked_static_cast<Asset::Events::AssetReady*>(data);
            std::map<request_tag_t, IAssetTransfer*>::iterator iter = currentTransfers.find(assetReady->tag_);
            if (iter != currentTransfers.end())
            {
                IAssetTransfer *transfer = iter->second;
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
            std::map<request_tag_t, IAssetTransfer*>::iterator iter = currentTransfers.find(resourceReady->tag_);
            if (iter != currentTransfers.end())
            {
                IAssetTransfer *transfer = iter->second;
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
