// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexAsset.h"
#include "AssetModule.h"
#include "AssetEvents.h"
#include "AssetCache.h"
#include "Framework.h"
#include "Platform.h"
#include "ConfigurationManager.h"

#include <QCryptographicHash>
#include <QString>

namespace Asset
{

const char *DEFAULT_ASSET_CACHE_PATH = "/assetcache";
const int DEFAULT_MEMORY_CACHE_SIZE = 32 * 1024 * 1024;
const f64 CACHE_CHECK_INTERVAL = 1.0;
const int CACHE_MAX_DELETES = 10;

AssetCache::AssetCache(Foundation::Framework* framework) :
    framework_(framework), 
    memory_cache_size_(DEFAULT_MEMORY_CACHE_SIZE),
    update_time_(0.0),
    md5_engine_(0)
{
    // Create asset cache directory
    cache_path_ = framework_->GetPlatform()->GetApplicationDataDirectory() + DEFAULT_ASSET_CACHE_PATH;
    if (boost::filesystem::exists(cache_path_) == false)
        boost::filesystem::create_directory(cache_path_);

    // Set size of memory cache
    memory_cache_size_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "memory_cache_size", DEFAULT_MEMORY_CACHE_SIZE);

    // Get path of local secondary cache
    std::string local_cache_path = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "local_cache_path", std::string("./data/assetcache"));
    
    CheckDiskCache(cache_path_);
    CheckDiskCache(local_cache_path);

    md5_engine_ = new QCryptographicHash(QCryptographicHash::Md5);
}

AssetCache::~AssetCache()
{
    SAFE_DELETE(md5_engine_);
}

void AssetCache::CheckDiskCache(const std::string& path)
{
    try
    {
        boost::filesystem::directory_iterator i(path);
        boost::filesystem::directory_iterator end_iter;
        while (i != end_iter)
        {
            if (boost::filesystem::is_regular_file(i->status()))
            {
                disk_cache_contents_.insert(i->path().native_directory_string());
            }
            ++i;
        }
    }
    catch (std::exception e)
    {
    }
}

bool CompareAssetAge(RexAsset* lhs, RexAsset* rhs)
{
    // Favor oldest assets for deletion
    return lhs->GetAge() > rhs->GetAge();
}

void AssetCache::Update(f64 frametime)
{
    update_time_ += frametime;
    if (update_time_ < CACHE_CHECK_INTERVAL)
        return;
    
    AssetMap::iterator i = assets_.begin();
    std::vector<RexAsset*> oldest_assets;
    
    uint total_size = 0;
    while (i != assets_.end())
    {
        RexAsset* asset = dynamic_cast<RexAsset*>(i->second.get());
        if (asset)
        {
            asset->AddAge(update_time_);
            oldest_assets.push_back(asset);
            total_size += asset->GetSize();
        }
        
        ++i;
    }
    
    std::sort(oldest_assets.begin(), oldest_assets.end(), CompareAssetAge);
    
    int deletes = 0;
    while ((total_size > memory_cache_size_) && (deletes < CACHE_MAX_DELETES) && (oldest_assets.size()))
    {
        RexAsset* asset = *oldest_assets.begin();
        AssetMap::iterator i = assets_.begin();
        while (i != assets_.end())
        {
            if (i->second.get() == asset)
            {
                total_size -= asset->GetSize();
                AssetModule::LogDebug("Removed cached asset " + asset->GetId() + " age " + ToString<Real>(asset->GetAge()));
                assets_.erase(i);
                oldest_assets.erase(oldest_assets.begin());
                break;
            }
            ++i;
        }
        
        ++deletes;
    }
    
    update_time_ = 0.0;
}

Foundation::AssetPtr AssetCache::GetAsset(const std::string& asset_id, bool check_memory, bool check_disk)
{
    if (check_memory)
    {
        AssetMap::iterator i = assets_.find(asset_id);
        if (i != assets_.end())
        {
            RexAsset* asset = dynamic_cast<RexAsset*>(i->second.get());            
            if (asset)
                asset->ResetAge();
        
            return i->second;
        }
    }
    
    std::string asset_hash = GetHash(asset_id);
    
    if (check_disk)
    {
        std::set<std::string>::iterator i = disk_cache_contents_.begin();
        while (i != disk_cache_contents_.end())
        {
            if (i->find(asset_hash) != std::string::npos)
                break;
            ++i;
        }
        
        if (i != disk_cache_contents_.end())
        {
            boost::filesystem::path file_path(*i);
            std::ifstream filestr(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
            if (filestr.good())
            {
                filestr.seekg(0, std::ios::end);
                uint length = filestr.tellg();
                filestr.seekg(0, std::ios::beg);

                if (length > 1)
                {
                    std::string type;
                    char c = 0;
                    uint name_length = 0;
                    
                    do
                    {
                        filestr.read(&c, 1);
                        if (c)
                        {
                            type.append(1, c);
                            name_length++;
                        }
                        length--;
                    } while (c && name_length < 256);
                            
                    if ((name_length > 2) || (name_length < 256))
                    {
                        RexAsset* new_asset = new RexAsset(asset_id, type);
                        assets_[asset_id] = Foundation::AssetPtr(new_asset);
                    
                        RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
                        data.resize(length);
                        filestr.read((char *)&data[0], length);
                        filestr.close();
                        return assets_[asset_id];
                    }
                }

                AssetModule::LogError("Malformed asset file " + asset_id + " found in cache.");
                disk_cache_contents_.erase(i);

                filestr.close();
            }
            else
            {
                // File got deleted by someone else while program was running, or something, do not re-check
                disk_cache_contents_.erase(i);
            }
        }
    }
    
    return Foundation::AssetPtr();
}

void AssetCache::StoreAsset(Foundation::AssetPtr asset)
{
    const std::string& asset_id = asset->GetId();

    AssetModule::LogDebug("Storing complete asset " + asset_id);

    // Store to memory cache
    assets_[asset_id] = asset;

    // Store to disk cache
    boost::filesystem::path file_path(cache_path_ + "/" + GetHash(asset_id));
    std::ofstream filestr(file_path.native_directory_string().c_str(), std::ios::out | std::ios::binary);
    if (filestr.good())
    {
        const u8* data = asset->GetData();
        uint size = asset->GetSize();

        // Store first the asset type, then the actual data
        const std::string& type = asset->GetType();
        const char* type_cstr = type.c_str();

        filestr.write((const char *)type_cstr, type.size() + 1);
        filestr.write((const char *)&data[0], size);
        filestr.close();

        disk_cache_contents_.insert(file_path.native_directory_string());
    }
    else
    {
        AssetModule::LogError("Error storing asset " + asset_id + " to cache.");
    }
}

std::string AssetCache::GetHash(const std::string &asset_id)
{
    QCryptographicHash md5_engine_(QCryptographicHash::Md5);
    md5_engine_.addData(asset_id.c_str(), asset_id.size());
    QString md5_hash(md5_engine_.result().toHex());
    md5_engine_.reset();
    return md5_hash.toStdString();
}

}
