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
#include <QByteArray>
#include <QFile>

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
    
    ServiceManagerPtr service_manager = framework_->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
    if (!asset_service)
        return false;
    
    AssetModule::LogDebug("New local asset request: " + asset_id);
    
    // Strip file: trims asset provider id (f.ex. 'file://') and potential mesh name inside the file (everything after last slash)
    std::string filename = asset_id.substr(7);
    size_t lastSlash = filename.find_last_of('/');
    if (lastSlash != std::string::npos)
        filename = filename.substr(0, lastSlash);
    
    std::string assetpath = GetPathForAsset(filename); // Look up all known local file asset storages for this asset.
    if (assetpath.empty())
    {
        AssetModule::LogInfo("Failed to load local asset " + filename);
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
    
    AssetModule::LogInfo("Failed to load local asset " + filename);
    return true;
}

std::string LocalAssetProvider::GetPathForAsset(const std::string& assetname)
{
    // Check first all subdirs without recursion, because recursion is potentially slow
    for(size_t i = 0; i < storages.size(); ++i)
    {
        std::string path = storages[i]->GetFullPathForAsset(assetname, false);
        if (path != "")
            return path;
    }

    for(size_t i = 0; i < storages.size(); ++i)
    {
        std::string path = storages[i]->GetFullPathForAsset(assetname, true);
        if (path != "")
            return path;
    }
    
    return "";
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

void LocalAssetProvider::AddStorageDirectory(const std::string &directory, const std::string &storageName, bool recursive)
{
    ///\todo Check first if the given directory exists as a storage, and don't add it as a duplicate if so.

    LocalAssetStoragePtr storage = LocalAssetStoragePtr(new LocalAssetStorage());
    storage->directory = directory;
    storage->name = storageName;
    storage->recursive = recursive;

    storages.push_back(storage);
}

std::vector<IAssetStorage*> LocalAssetProvider::GetStorages()
{
    std::vector<IAssetStorage*> stores;
    for(size_t i = 0; i < storages.size(); ++i)
        stores.push_back(storages[i].get());
    return stores;
}


}
