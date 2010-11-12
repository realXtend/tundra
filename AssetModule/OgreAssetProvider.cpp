// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "OgreAssetProvider.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"


namespace Asset
{

OgreAssetProvider::OgreAssetProvider(Foundation::Framework* framework) :
    framework_(framework)
{
    EventManagerPtr event_manager = framework_->GetEventManager();
    event_category_ = event_manager->QueryEventCategory("Asset");
}

OgreAssetProvider::~OgreAssetProvider()
{
}

const std::string& OgreAssetProvider::Name()
{
    static const std::string name("Ogre");
    
    return name;
}

bool OgreAssetProvider::IsValidId(const std::string& asset_id, const std::string& asset_type)
{
    return (asset_id.find("mesh://") == 0);
}

bool OgreAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
{
    if (!IsValidId(asset_id, asset_type))
        return false;
    
    ServiceManagerPtr service_manager = framework_->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        return false;
    
    AssetModule::LogDebug("New Ogre asset request: " + asset_id);
    
 
    Asset::RexAsset* new_asset = new Asset::RexAsset(asset_id, asset_type);
    Foundation::AssetPtr asset_ptr(new_asset);
            
    // Store to memory cache only
    asset_service->StoreAsset(asset_ptr, false);
            
    // Send asset_ready event as delayed
    Asset::Events::AssetReady* event_data = new Asset::Events::AssetReady(asset_ptr->GetId(), asset_ptr->GetType(), asset_ptr, tag);
    framework_->GetEventManager()->SendDelayedEvent(event_category_, Asset::Events::ASSET_READY, EventDataPtr(event_data));
    
    return true;
}

bool OgreAssetProvider::InProgress(const std::string& asset_id)
{
    return false;
}

Foundation::AssetPtr OgreAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)
{
    // Not supported
    return Foundation::AssetPtr();
}

bool OgreAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)
{
    // Not supported
    return false;
}

void OgreAssetProvider::Update(f64 frametime)
{
}

}
