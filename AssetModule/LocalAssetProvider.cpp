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

LocalAssetProvider::LocalAssetProvider(Foundation::Framework* framework) :
    framework_(framework)
{
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
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
    
    Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
    boost::shared_ptr<Foundation::AssetServiceInterface> asset_service =
        service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
    if (!asset_service)
        return false;
    
    AssetModule::LogDebug("New local asset request: " + asset_id);
    
    // Strip file: trims asset provider id (f.ex. 'file://') and potential mesh name inside the file (everything after last slash)
    std::string filename = asset_id.substr(7);
    size_t lastSlash = filename.find_last_of('/');
    if (lastSlash != std::string::npos)
        filename = filename.substr(0, lastSlash);
    
    std::string assetpath = GetPathForAsset(filename);
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
    for (uint i = 0; i < directories_.size(); ++i)
    {
        if (boost::filesystem::exists(directories_[i].dir_ + "/" + assetname))
            return directories_[i].dir_;
    }
    
    // Now check recursively if not yet found
    for (uint i = 0; i < directories_.size(); ++i)
    {
        if (directories_[i].recursive_)
        {
            try
            {
                boost::filesystem::recursive_directory_iterator iter(directories_[i].dir_);
                boost::filesystem::recursive_directory_iterator end_iter;
                for(; iter != end_iter; ++iter )
                    if (!fs::is_regular_file(iter->status()))
                {
                    // Check the subdir
                    if (boost::filesystem::exists(iter->path().string() + "/" + assetname))
                        return iter->path().string();
                }
            }
            catch (...)
            {
            }
        }
    }
    
    return std::string();
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

void LocalAssetProvider::AddDirectory(const std::string& dir, bool recursive)
{
    if (dir.empty())
        return;
    
    // Remove in case it exists already
    RemoveDirectory(dir);
    
    AssetDirectory newdir;
    newdir.dir_ = dir;
    newdir.recursive_ = recursive;
    directories_.push_back(newdir);
}

void LocalAssetProvider::RemoveDirectory(const std::string& dir)
{
    for (uint i = 0; i < directories_.size(); ++i)
    {
        if (directories_[i].dir_ == dir)
        {
            directories_.erase(directories_.begin() + i);
            break;
        }
    }
}

}
