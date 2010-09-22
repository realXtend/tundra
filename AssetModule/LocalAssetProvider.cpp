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

#include <QByteArray>
#include <QFile>

namespace Asset
{

LocalAssetProvider::LocalAssetProvider(Foundation::Framework* framework, const std::string& asset_dir) :
    framework_(framework),
    asset_dir_(asset_dir)
{
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
    event_category_ = event_manager->QueryEventCategory("Asset");
    
    if (!asset_dir_.empty())
    {
        char lastchar = asset_dir_[asset_dir_.length() - 1];
        if ((lastchar != '/') && (lastchar != '\\'))
            asset_dir_ += '/';
    }
}

LocalAssetProvider::~LocalAssetProvider()
{
}

const std::string& LocalAssetProvider::Name()
{
    static const std::string name("Local");
    
    return name;
}

bool LocalAssetProvider::IsValidId(const std::string& asset_id, const std::string& asset_type)
{
    return (asset_id.find("file://") == 0);
}

bool LocalAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
{
    if (!IsValidId(asset_id, asset_type))
        return false;
    
    Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
    if (!asset_service)
        return false;
    
    AssetModule::LogDebug("New local asset request: " + asset_id);
    
    // Strip file:
    std::string filepath = asset_dir_ + asset_id.substr(7);
    
    boost::filesystem::path file_path(filepath);
    std::ifstream filestr(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
    if (filestr.good())
    {
        filestr.seekg(0, std::ios::end);
        uint length = filestr.tellg();
        filestr.seekg(0, std::ios::beg);
        
        if (length > 0)
        {
            RexAsset* new_asset = new RexAsset(asset_id, asset_type);
            Foundation::AssetPtr asset_ptr(new_asset);
            
            RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
            data.resize(length);
            filestr.read((char *)&data[0], length);
            filestr.close();
            
            // Store to memory cache only
            asset_service->StoreAsset(asset_ptr, false);
            // Send asset_ready event as delayed
            Events::AssetReady* event_data = new Events::AssetReady(asset_ptr->GetId(), asset_ptr->GetType(), asset_ptr, tag);
            framework_->GetEventManager()->SendDelayedEvent(event_category_, Events::ASSET_READY, EventDataPtr(event_data));
            
            return true;
        }
        else
            filestr.close();
    }
    
    AssetModule::LogInfo("Failed to load local asset " + asset_id.substr(7));
    return false;
}

bool LocalAssetProvider::InProgress(const std::string& asset_id)
{
    return false;
}

Foundation::AssetPtr LocalAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)
{
    // Not supported
    return Foundation::AssetPtr();
}

bool LocalAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)
{
    // Not supported
    return false;
}

void LocalAssetProvider::Update(f64 frametime)
{
}

}
