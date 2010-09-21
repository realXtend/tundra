// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetCache.h"
#include "RexAsset.h"
#include "AssetModule.h"
#include "AssetEvents.h"

#include "Framework.h"
#include "Platform.h"
#include "ConfigurationManager.h"
#include "UiSettingsServiceInterface.h"

#include <QCryptographicHash>
#include <QString>
#include <QSettings>
#include <QMessageBox>

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
        disk_changes_after_last_check_(false),
        disk_cache_max_size_(0)
    {
        // Create asset cache directory
        cache_path_ = framework_->GetPlatform()->GetApplicationDataDirectory() + DEFAULT_ASSET_CACHE_PATH;
        if (boost::filesystem::exists(cache_path_) == false)
            boost::filesystem::create_directory(cache_path_);

        // Set size of memory cache
        memory_cache_size_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "memory_cache_size", DEFAULT_MEMORY_CACHE_SIZE);

        // Get path of local secondary cache
        std::string local_cache_path = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "local_cache_path", std::string("./data/assetcache"));
        
        // Init disk
        InitDiskCaching();

        // Read both disk caches 
        CheckDiskCache(cache_path_);
        CheckDiskCache(local_cache_path);
    }

    AssetCache::~AssetCache()
    {
    }

    void AssetCache::InitDiskCaching()
    {
        // Connect to ui signals
        Foundation::UiSettingsServiceInterface *ui_settings = framework_->GetService<Foundation::UiSettingsServiceInterface>();
        if (ui_settings)
        {
            QObject *cache_widget = ui_settings->GetCacheSettingsWidget();
            if (cache_widget)
            {
                connect(cache_widget, SIGNAL(ClearAssetsPushed()), SLOT(ClearDiskCache()));
                connect(cache_widget, SIGNAL(AssetSettingsSaved(int)), SLOT(CacheConfigChanged(int)));
            }
        }

        // Init Qt asset cache working dir
        cache_dir_ = QDir(cache_path_.c_str());

        // Read initial values from config
        QSettings cache_settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/CacheSettings");
        disk_cache_max_size_ = cache_settings.value("AssetCache/MaxSize", QVariant(0)).toInt();
    }

    void AssetCache::ClearDiskCache()
    {
        QFileInfoList file_list = cache_dir_.entryInfoList(QDir::Files);
        if (file_list.count() > 0)
        {
            int removed_files = 0;
            qint64 removed_bytes = 0;
            
            foreach(QFileInfo file_info, file_list)
            {
                QString native_absolute_path = file_info.absoluteFilePath().replace("/", QDir::separator());
                qint64 file_size = file_info.size();
                if (!cache_dir_.remove(file_info.fileName()))
                    continue;
                
                removed_files++;
                removed_bytes += file_size;

                std::set<std::string>::iterator find_result = disk_cache_contents_.find(native_absolute_path.toStdString());
                if (find_result != disk_cache_contents_.end())
                    disk_cache_contents_.erase(find_result);
            }

            // Notify user
            qreal removed_bytes_f = removed_bytes;
            QString mb_string = QString::number(((removed_bytes_f/1024)/1024));
            mb_string = mb_string.left(mb_string.indexOf(".")+3);
            QMessageBox::information(0, "Asset Cache", QString("Asset cache cleared, removed %1 files total of " + mb_string + " mb").arg(removed_files));
        }
        else
            QMessageBox::information(0, "Asset Cache", "There are currently no files in asset cache");
    }

    void AssetCache::CacheConfigChanged(int new_disk_max_size)
    {
        if (disk_cache_max_size_ != new_disk_max_size)
        {
            disk_cache_max_size_ = new_disk_max_size;
            CheckDiskCacheSize(false);
        }
    }

    void AssetCache::CheckDiskCacheSize(bool make_extra_space)
    {
        if (disk_cache_max_size_ == 0)
            return;

        int current_size = 0;
        QFileInfoList file_list = cache_dir_.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
        foreach(QFileInfo file, file_list)
            current_size += file.size();

        if (current_size > disk_cache_max_size_)
        {
            int removed_files = 0;
            qint64 removed_bytes = 0;

            int aimed_size = disk_cache_max_size_;
            if (make_extra_space)
                aimed_size -= (2*1024*1024);

            foreach(QFileInfo file_info, file_list)
            {
                QString native_absolute_path = file_info.absoluteFilePath().replace("/", QDir::separator());
                qint64 file_size = file_info.size();
                if (!cache_dir_.remove(file_info.fileName()))
                    continue;
                
                removed_files++;
                removed_bytes += file_size;
                current_size -= file_size;

                std::set<std::string>::iterator find_result = disk_cache_contents_.find(native_absolute_path.toStdString());
                if (find_result != disk_cache_contents_.end())
                    disk_cache_contents_.erase(find_result);

                if (current_size < aimed_size)
                    break;
            }

            AssetModule::LogInfo("Asset cache was over limit. Removed " + QString::number(removed_files).toStdString() + 
                " files, total of " + QString::number(removed_bytes).toStdString() + " bytes");
        }
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
                    disk_cache_contents_.insert(i->path().native_directory_string());
                ++i;
            }
        }
        catch (std::exception &/*e*/)
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
                    AssetModule::LogDebug("Removed cached asset " + asset->GetId() + " age " + ToString<float>(asset->GetAge()));
                    assets_.erase(i);
                    oldest_assets.erase(oldest_assets.begin());
                    break;
                }
                ++i;
            }
            
            ++deletes;
        }

        if (disk_changes_after_last_check_)
        {
            CheckDiskCacheSize();
            disk_changes_after_last_check_ = false;
        }
        
        update_time_ = 0.0;
    }

    Foundation::AssetPtr AssetCache::GetAsset(const std::string& asset_id, bool check_memory, bool check_disk, const std::string& asset_type)
    {
        if (check_memory)
        {
            AssetMap::iterator i = assets_.begin();
            while (i != assets_.end())
            {
                Foundation::AssetPtr asset = i->second;
                if ((asset->GetId() == asset_id) && (asset_type.empty() || (asset->GetType() == asset_type)))
                {
                    RexAsset* asset = dynamic_cast<RexAsset*>(i->second.get());
                    if (asset)
                        asset->ResetAge();
                    return i->second;
                }
                ++i;
            }
        }
        
        std::string asset_hash = GetHash(asset_id);
        
        if (check_disk)
        {
            std::set<std::string>::iterator i = disk_cache_contents_.begin();
            while (i != disk_cache_contents_.end())
            {
                if ((i->find(asset_hash) != std::string::npos) && (asset_type.empty() || i->find(asset_type) != std::string::npos))
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
                    
                    // Identify assettype from end of cached asset name
                    StringVector assetNameType = SplitString(*i, '.');
                    if (assetNameType.size() < 2)
                    {
                        AssetModule::LogDebug("Malformed assetcache filename " + *i);
                        filestr.close();
                        disk_cache_contents_.erase(i);
                        return Foundation::AssetPtr();
                    }
                    
                    std::string type = assetNameType[assetNameType.size() - 1];

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
                    // File got deleted by someone else while program was running, or something, do not re-check
                    disk_cache_contents_.erase(i);
                }
            }
        }
        
            
        return Foundation::AssetPtr();
    }

    void AssetCache::StoreAsset(Foundation::AssetPtr asset, bool store_to_disk)
    {
        const std::string& asset_id = asset->GetId();
        AssetModule::LogDebug("Storing complete asset " + asset_id);

        // Store to memory cache
        assets_[asset_id] = asset;

        if (!store_to_disk)
            return;
        
        // Store to disk cache
        const std::string& type = asset->GetType();
        boost::filesystem::path file_path(cache_path_ + "/" + GetHash(asset_id) + "." + type);
        std::ofstream filestr(file_path.native_directory_string().c_str(), std::ios::out | std::ios::binary);
        if (filestr.good())
        {
            const u8* data = asset->GetData();
            uint size = asset->GetSize();
            
            filestr.write((const char *)&data[0], size);
            filestr.close();

            disk_cache_contents_.insert(file_path.native_directory_string());
        }
        else
        {
            AssetModule::LogError("Error storing asset " + asset_id + " to cache.");
        }
    }

    bool AssetCache::DeleteAsset(Foundation::AssetPtr asset)
    {
        const std::string& asset_id = asset->GetId();

        // Delete from disk cache
        const std::string& type = asset->GetType();
        boost::filesystem::path file_path(cache_path_ + "/" + GetHash(asset_id)  + "." + type);
        if (boost::filesystem::exists(file_path))
        {
            if (boost::filesystem::remove(file_path))
            {
                AssetModule::LogDebug("Removed asset " + asset_id + " from cache");

                std::set<std::string>::iterator find_result = disk_cache_contents_.find(file_path.native_directory_string());
                if (find_result != disk_cache_contents_.end())
                    disk_cache_contents_.erase(find_result);

                assets_.erase(asset_id);
                disk_changes_after_last_check_ = true;

                return true;
            }
            else
            {
                AssetModule::LogDebug("Could not removed asset " + asset_id + " from cache");
                return false;
            }
        }
        else
        {
            AssetModule::LogDebug("File " + file_path.string() + " does not exist, could not delete from cache.");
            return true;
        }
    }

    std::string AssetCache::GetHash(const std::string &asset_id)
    {
        QCryptographicHash md5_engine(QCryptographicHash::Md5);
        md5_engine.addData(asset_id.c_str(), asset_id.size());
        QString md5_hash(md5_engine.result().toHex());
        md5_engine.reset();
        return md5_hash.toStdString();
    }
}
