// For conditions of distribution and use, see copyright notice in license.txt


#include "StableHeaders.h"

#include "Poco/URIStreamOpener.h"
#include "Poco/StreamCopier.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/Net/HTTPStreamFactory.h"
#include "Poco/Net/FTPStreamFactory.h"
#include <memory>
#include <iostream>

#include "NetworkEvents.h"
#include "OpenSimProtocolModule.h"
#include "RexProtocolMsgIDs.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "HttpAssetProvider.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
	const Core::Real HttpAssetProvider::DEFAULT_ASSET_TIMEOUT = 120.0;

	HttpAssetProvider::HttpAssetProvider(Foundation::Framework* framework) : framework_(framework)
	{
		buffer_ = new Core::u8[BUFFER_SIZE];

		asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("UDPAssetProvider", "Timeout", DEFAULT_ASSET_TIMEOUT);            
		Poco::Net::HTTPStreamFactory::registerFactory();

		Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        // Assume that the asset manager has been instantiated at this point
        event_category_ = event_manager->QueryEventCategory("Asset");
        if (!event_category_)
        {
            AssetModule::LogWarning("Could not get event category for Asset events");
        }
	}

	HttpAssetProvider::~HttpAssetProvider()
	{
		delete [] buffer_;
	}
	
	const std::string& HttpAssetProvider::Name()
	{
        static const std::string name("Http asset provider");
        return name;
	}

	bool HttpAssetProvider::RequestAsset(const std::string& asset_url, const std::string& asset_type, Core::request_tag_t tag)
	{
        if (RexUUID::IsValid(asset_url))
            return false;

        asset_type_t asset_type_int = GetAssetTypeFromTypeName(asset_type);
        if (asset_type_int < 0)
            return false;

		HttpAssetTransfer* t = new HttpAssetTransfer();
		t->SetAssetId(asset_url);
		t->SetAssetType(asset_type_int);
		t->InsertTag(tag);
		asset_transfers_[asset_url] = HttpAssetTransferPtr(t);
		t->StartTransfer();
        return true;
	}

    bool HttpAssetProvider::InProgress(const std::string& asset_id)
    {
        HttpAssetTransfer* transfer = GetTransfer(asset_id);
        return (transfer != NULL);
		return false;
    }

	Foundation::AssetPtr HttpAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, Core::uint received)
    {
        HttpAssetTransfer* transfer = GetTransfer(asset_id);
            
        if ((transfer) && (transfer->GetReceivedContinuous() >= received))
        {
            // Make new temporary asset for the incomplete data
            RexAsset* new_asset = new RexAsset(transfer->GetAssetId(), GetTypeNameFromAssetType(transfer->GetAssetType()));
            Foundation::AssetPtr asset_ptr(new_asset);
            
            RexAsset::AssetDataVector& data = new_asset->GetDataInternal();
            data.resize(transfer->GetReceivedContinuous());
            transfer->AssembleData(&data[0]);
            
            return asset_ptr;
        }
        
        return Foundation::AssetPtr();
    }

    bool HttpAssetProvider::QueryAssetStatus(const std::string& asset_id, Core::uint& size, Core::uint& received, Core::uint& received_continuous)    
    {
        HttpAssetTransfer* transfer = GetTransfer(asset_id);
            
        if (transfer)
        {
            size = transfer->GetSize();
            received = transfer->GetReceived();
            received_continuous = transfer->GetReceivedContinuous();

            return true;
        }
        
        return false;
    }

	// \todo Current implementation get all asset data before return 
	//       We should only download what is available and get rest later
	void HttpAssetProvider::Update(Core::f64 frametime)
	{
		HttpAssetTransferMap::iterator i = asset_transfers_.begin();
		while (i != asset_transfers_.end())
		{
			HttpAssetTransferPtr t = i->second;
			int received = 0;
			std::istream* s = t->GetResponseStream();
			if (s == NULL)
				continue;

			s->read((char*)(buffer_), BUFFER_SIZE);
			received = s->gcount();

			t->ReceiveData(t->GetNextPacketId(), buffer_, received);
                
			SendAssetProgress(*t.get());

			if (t->Ready())
			{
				StoreAsset(*t.get());
		        asset_transfers_.erase(i);
				return;
			}
			i++;
		}
	}

    HttpAssetTransfer* HttpAssetProvider::GetTransfer(const std::string& asset_id)
    {
        HttpAssetTransferMap::iterator j = asset_transfers_.begin();
        while (j != asset_transfers_.end())
        {
            if (j->second->GetAssetId() == asset_id)
                return (j->second).get();
            ++j;
        }
        
        return NULL;
    }       

	void HttpAssetProvider::SendAssetProgress(HttpAssetTransfer& transfer)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetProgress event_data(transfer.GetAssetId(), GetTypeNameFromAssetType(transfer.GetAssetType()), transfer.GetSize(), transfer.GetReceived(), transfer.GetReceivedContinuous());
        event_manager->SendEvent(event_category_, Events::ASSET_PROGRESS, &event_data);
    }

    void HttpAssetProvider::SendAssetCanceled(HttpAssetTransfer& transfer)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        Events::AssetCanceled event_data(transfer.GetAssetId(), GetTypeNameFromAssetType(transfer.GetAssetType()));
        event_manager->SendEvent(event_category_, Events::ASSET_CANCELED, &event_data);
    }

    void HttpAssetProvider::StoreAsset(HttpAssetTransfer& transfer)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
        {
            const std::string& asset_id = transfer.GetAssetId();
        
            Foundation::AssetPtr new_asset = Foundation::AssetPtr(new RexAsset(asset_id, GetTypeNameFromAssetType(transfer.GetAssetType())));
            RexAsset::AssetDataVector& data = checked_static_cast<RexAsset*>(new_asset.get())->GetDataInternal();
            data.resize(transfer.GetReceived());
            transfer.AssembleData(&data[0]);
      
            asset_service->StoreAsset(new_asset);
            
            // Send asset ready event for each request tag
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            const Core::RequestTagVector& tags = transfer.GetTags();
            for (Core::uint i = 0; i < tags.size(); ++i)
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

} // end of namespace
