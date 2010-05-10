// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "HttpTask.h"
#include "HttpUtilities.h"
#include "LLSDUtilities.h"
#include "XMLRPCAssetProvider.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"

#include "Poco/URI.h"
#include "Poco/Base64Decoder.h"
#include "Poco/StreamCopier.h"

namespace Asset
{
    const Real XMLRPCAssetProvider::DEFAULT_ASSET_TIMEOUT = 10.0;
        
    XMLRPCAssetProvider::XMLRPCAssetProvider(Foundation::Framework* framework) :
        framework_(framework),
        manager_(framework)
    {
        asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "xmlrpc_timeout", DEFAULT_ASSET_TIMEOUT);

        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        event_category_ = event_manager->QueryEventCategory("Asset");
    }
    
    XMLRPCAssetProvider::~XMLRPCAssetProvider()
    {
    }
    
    const std::string& XMLRPCAssetProvider::Name()
    {
        static const std::string name("XMLRPC");
        
        return name;
    }

    bool XMLRPCAssetProvider::IsValidId(const std::string& asset_id, const std::string& asset_type)
    {
        // Has to be protocol based url
        if (asset_id.find("://") == std::string::npos)
            return false;
        // ..and a valid url
        if (HttpUtilities::GetHostFromUrl(asset_id).empty())
            return false;
        // This is somewhat hacky, we check for existence of "/item/" in the path, and for the right length of asset hash after that
        std::size_t index = asset_id.find("/item/");
        if (index == std::string::npos)
            return false;
        if (index != asset_id.length() - 34)
            return false;
            
        return true;
    }
    
    bool XMLRPCAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
    {
        if (!IsValidId(asset_id, asset_type))
            return false;

        // See if request already exists, just add tag in that case
        AssetRequestMap::iterator i = requests_.find(asset_id);
        if (i != requests_.end())
        {
            i->second.tags_.push_back(tag);
            return true;
        }
        else
        {
            std::string host = HttpUtilities::GetHostFromUrl(asset_id);

            AssetRequest new_request;
            new_request.asset_id_ = asset_id;
            new_request.asset_type_ = asset_type;
            new_request.tags_.push_back(tag);

            AssetModule::LogDebug("New XMLRPC asset request: " + asset_id);
            
            // See if threadtask for this host already exists
            if (!manager_.GetThreadTask(host))
            {
                Foundation::ThreadTaskPtr new_task(new HttpUtilities::HttpTask(host, true));
                manager_.AddThreadTask(new_task);
            }
            HttpUtilities::HttpTaskRequestPtr new_http_request(new HttpUtilities::HttpTaskRequest);
            new_http_request->url_ = asset_id;
            new_http_request->timeout_ = (Real)asset_timeout_;
            new_request.http_request_tag_ = manager_.AddRequest<HttpUtilities::HttpTaskRequest>(host, new_http_request);
            
            requests_[asset_id] = new_request;
            return true;
        }
    }
    
    bool XMLRPCAssetProvider::InProgress(const std::string& asset_id)
    {
        AssetRequestMap::iterator i = requests_.find(asset_id);
        if (i == requests_.end())
            return false;
        return true;
    }
    
    Foundation::AssetPtr XMLRPCAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)
    {
        // Not supported
        return Foundation::AssetPtr();
    }
    
    bool XMLRPCAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)    
    {
        AssetRequestMap::iterator i = requests_.find(asset_id);
        if (i == requests_.end())
            return false;
            
        // Don't know these before transfer is finished
        size = 0;
        received = 0;
        received_continuous = 0;

        return true;
    }
    
    void XMLRPCAssetProvider::Update(f64 frametime)
    {
        std::vector<Foundation::ThreadTaskResultPtr> results = manager_.GetResults();
        
        std::vector<Foundation::ThreadTaskResultPtr>::iterator i = results.begin();
        while (i != results.end())
        {
            HttpUtilities::HttpTaskResult* result = dynamic_cast<HttpUtilities::HttpTaskResult*>((*i).get());
            if (result)
                ProcessHttpResult(result);
            ++i;
        }
        
        // If no more requests to fulfill, remove all threadtasks
        if (!requests_.size())
            manager_.RemoveThreadTasks();
    }
    
    void XMLRPCAssetProvider::ProcessHttpResult(HttpUtilities::HttpTaskResult* result)
    {
        if (!result)
            return;
            
        // Match tag to xmlrpc asset request
        std::map<std::string, AssetRequest>::iterator i = requests_.begin();
        while (i != requests_.end())
        {
            if (i->second.http_request_tag_ == result->tag_)
                break;
            ++i;
        }
        if (i == requests_.end())
        {
            AssetModule::LogWarning("Unmatched XMLRPC asset http request tag " + ToString<int>(result->tag_));
            return;
        }
        
        AssetRequest& request = i->second;
        
        if (result->GetSuccess())
            DecodeAsset(result, request);
        else
            SendCanceled(request, result->reason_);
        
        // No matter if it was a success or failure, this request has been processed and can be erased
        requests_.erase(i);
    }
    
    void XMLRPCAssetProvider::DecodeAsset(HttpUtilities::HttpTaskResult* result, const AssetRequest& request)
    {
        // Transform result data to string for LLSD operations
        std::string result_data((char *)&result->data_[0], result->data_.size());
        
        // Find the binary tag, or else
        std::size_t index = 0;
        if (!RexTypes::GetNextLLSDTag("binary", result_data, index))
        {
            // No data, probably an error reply
            std::map<std::string, std::string> error_map = RexTypes::ParseLLSDMap(result_data);
            SendCanceled(request, error_map["Error"]);
            return;
        }
        
        std::istringstream base64_data(RexTypes::GetLLSDTagContent(result_data, index));

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
        {
            Foundation::AssetPtr new_asset = Foundation::AssetPtr(new RexAsset(request.asset_id_, request.asset_type_));
            RexAsset::AssetDataVector& data = checked_static_cast<RexAsset*>(new_asset.get())->GetDataInternal();
            
            Poco::Base64Decoder decoder(base64_data);
            while (decoder.good())
            {
                int c = decoder.get();
                if (decoder.good())
                    data.push_back(c);
            }
            
            AssetModule::LogDebug("XMLRPC asset " + request.asset_id_ + " encoded size: " + ToString<uint>(base64_data.str().length()) + 
                " decoded size: " + ToString<uint>(data.size()));
            asset_service->StoreAsset(new_asset);
            
            // Send asset ready event for each request tag
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            const RequestTagVector& tags = request.tags_;
            for (uint i = 0; i < tags.size(); ++i)
            {          
                Events::AssetReady event_data(new_asset->GetId(), new_asset->GetType(), new_asset, tags[i]);
                event_manager->SendEvent(event_category_, Events::ASSET_READY, &event_data);
            }
        }
        else
        {
            AssetModule::LogError("Asset service not found, could not store asset to cache");
        }
    }
    
    void XMLRPCAssetProvider::SendCanceled(const AssetRequest& request, const std::string& error_message)
    {
        AssetModule::LogDebug("XMLRPC asset request for " + request.asset_id_ + " failed: " + error_message);
        
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetCanceled event_data(request.asset_id_, request.asset_type_);
        event_manager->SendEvent(event_category_, Events::ASSET_CANCELED, &event_data);
    }
}