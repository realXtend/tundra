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
#include "../AssetModule/AssetEvents.h"

DEFINE_POCO_LOGGING_FUNCTIONS("Asset")

using namespace Foundation;

AssetAPI::AssetAPI(Foundation::Framework *owner)
:framework(owner)
{
}

void IAssetTransfer::EmitAssetDownloaded()
{
    emit Downloaded();
}

void IAssetTransfer::EmitAssetDecoded()
{
    emit Decoded();
}

void IAssetTransfer::EmitAssetLoaded()
{
    emit Loaded();
}

namespace
{
    std::string GetResourceTypeFromResourceName(const std::string &name)
    {
        if (name.find(".mesh") != std::string::npos)
            return "OgreMesh";
        if (name.find(".skeleton") != std::string::npos)
            return "OgreSkeleton";
        if (name.find(".material") != std::string::npos)
            return "OgreMaterial";
        if (name.find(".jpg") != std::string::npos || name.find(".png") != std::string::npos || name.find(".tga") != std::string::npos
            || name.find(".bmp") != std::string::npos || name.find(".dds") != std::string::npos)
            return "OgreTexture";
        if (name.find(".particle") != std::string::npos)
            return "OgreParticle";
        
        if (name.find("3d") != std::string::npos ||
            name.find("b3d") != std::string::npos ||
            name.find("dae") != std::string::npos ||
            name.find("bvh") != std::string::npos ||
            name.find("3ds") != std::string::npos ||
            name.find("ase") != std::string::npos ||
            name.find("obj") != std::string::npos ||
            name.find("ply") != std::string::npos ||
            name.find("dxf") != std::string::npos ||
            name.find("nff") != std::string::npos ||
            name.find("smd") != std::string::npos ||
            name.find("vta") != std::string::npos ||
            name.find("mdl") != std::string::npos ||
            name.find("md2") != std::string::npos ||
            name.find("md3") != std::string::npos ||
            name.find("mdc") != std::string::npos ||
            name.find("md5mesh") != std::string::npos ||
            name.find("x") != std::string::npos ||
            name.find("q3o") != std::string::npos ||
            name.find("q3s") != std::string::npos ||
            name.find("raw") != std::string::npos ||
            name.find("ac") != std::string::npos ||
            name.find("stl") != std::string::npos ||
            name.find("irrmesh") != std::string::npos ||
            name.find("irr") != std::string::npos ||
            name.find("off") != std::string::npos ||
            name.find("ter") != std::string::npos ||
            name.find("mdl") != std::string::npos ||
            name.find("hmp") != std::string::npos ||
            name.find("ms3d") != std::string::npos ||
            name.find("lwo") != std::string::npos ||
            name.find("lws") != std::string::npos ||
            name.find("lxo") != std::string::npos ||
            name.find("csm") != std::string::npos ||
            name.find("ply") != std::string::npos ||
            name.find("cob") != std::string::npos ||
            name.find("scn") != std::string::npos )
            return "OgreMesh";

        return "";
        // Note: There's a separate OgreImageTextureResource which isn't handled above.
    }
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

    std::string foundAssetType = GetResourceTypeFromResourceName(assetRef.toLower().toStdString());
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
