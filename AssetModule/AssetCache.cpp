// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexAsset.h"
#include "AssetModule.h"
#include "AssetTransfer.h"
#include "AssetEvents.h"
#include "AssetCache.h"

namespace Asset
{
    const char *AssetCache::DEFAULT_ASSET_CACHE_PATH = "/assetcache";
    
    AssetCache::AssetCache(Foundation::Framework* framework) :
        framework_(framework)
    {
        // Create asset cache directory
        cache_path_ = framework_->GetPlatform()->GetApplicationDataDirectory() + DEFAULT_ASSET_CACHE_PATH;
        if (boost::filesystem::exists(cache_path_) == false)
        {
            boost::filesystem::create_directory(cache_path_);
        }    
    }

    AssetCache::~AssetCache()
    {
    }

    Foundation::AssetPtr AssetCache::GetAsset(const std::string& asset_id, bool check_memory, bool check_disk)
    {
        if (check_memory)
        {
            AssetMap::iterator i = assets_.find(asset_id);
            if (i != assets_.end())
                return i->second;
        }
        
        if (check_disk)
        {
            boost::filesystem::path file_path(cache_path_ + "/" + asset_id);      
            std::ifstream filestr(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
            if (filestr.good())
            {
                filestr.seekg(0, std::ios::end);
                Core::uint length = filestr.tellg();
                filestr.seekg(0, std::ios::beg);

                Core::uint type;
                if (length > sizeof(type))
                {
                    length -= sizeof(type);
                    
                    filestr.read((char *)&type, sizeof(type));
            
                    RexAsset* new_asset = new RexAsset(asset_id, type);
                    assets_[asset_id] = Foundation::AssetPtr(new_asset);
                    
                    RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
                    data.resize(length);
                    filestr.read((char *)&data[0], length);
                    filestr.close();
                    
                    return assets_[asset_id];
                }
                else
                {
                    AssetModule::LogError("Malformed asset file " + asset_id + " found in cache.");
                }
                
                filestr.close();
            }
        }
        
        return Foundation::AssetPtr();
    }
           
    void AssetCache::StoreAsset(Foundation::AssetPtr asset)
    {    
        const std::string& asset_id = asset->GetId();
        
        AssetModule::LogInfo("Storing complete asset " + asset_id);

        // Store to memory cache
        assets_[asset_id] = asset;
                
        // Store to disk cache
        boost::filesystem::path file_path(cache_path_ + "/" + asset_id);
        std::ofstream filestr(file_path.native_directory_string().c_str(), std::ios::out | std::ios::binary);
        if (filestr.good())
        {
            Core::uint type = asset->GetType();
            const Core::u8* data = asset->GetData();
            Core::uint size = asset->GetSize();
            
            // Store first the asset type, then the actual data
            filestr.write((const char *)&type, sizeof(type));
            filestr.write((const char *)&data[0], size);
            filestr.close();
        }
        else
        {
            AssetModule::LogError("Error storing asset " + asset_id + " to cache.");
        }
        
        // Send asset ready event
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetReady event_data(asset->GetId(), asset->GetType(), asset);
        event_manager->SendEvent(event_manager->QueryEventCategory("Asset"), Events::ASSET_READY, &event_data);
    }    
}