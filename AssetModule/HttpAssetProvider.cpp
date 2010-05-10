// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Poco/Base64Encoder.h"
#include <memory>
#include <iostream>

#include "NetworkEvents.h"
#include "RealXtend/RexProtocolMsgIDs.h"
#include "ProtocolModuleOpenSim.h"
#include "AssetEvents.h"
#include "AssetManager.h"
#include "AssetModule.h"
#include "RexAsset.h"
#include "RexTypes.h"
#include "HttpAssetProvider.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ConfigurationManager.h"

using namespace OpenSimProtocol;
using namespace RexTypes;

namespace Asset
{
    const Real HttpAssetProvider::DEFAULT_ASSET_TIMEOUT = 120.0;

    HttpAssetProvider::HttpAssetProvider(Foundation::Framework* framework) : framework_(framework)
    {
        asset_timeout_ = framework_->GetDefaultConfig().DeclareSetting("AssetSystem", "http_timeout", DEFAULT_ASSET_TIMEOUT);            

        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
        
        event_category_ = event_manager->QueryEventCategory("Asset");
    }

    HttpAssetProvider::~HttpAssetProvider()
    {
    }
    
    const std::string& HttpAssetProvider::Name()
    {
        static const std::string name("Http asset provider");
        return name;
    }

    bool HttpAssetProvider::IsValidId(const std::string& asset_url, const std::string& asset_type)
    {
        // Differentiate between UDP & URL based requests
        //! \todo check URL validity in more sophisticated way, if desired
        if (asset_url.find("://") == std::string::npos)
            return false;
        else
            return true;
    }

    bool HttpAssetProvider::RequestAsset(const std::string& asset_id, const std::string& asset_type, request_tag_t tag)
    {
        if (!IsValidId(asset_id, asset_type))
            return false;

        asset_type_t asset_type_int = GetAssetTypeFromTypeName(asset_type);
        if (asset_type_int < 0)
            return false;

        HttpAssetTransfer* transfer = GetTransfer(asset_id);
        if (transfer)
        {
            transfer->InsertTag(tag);
            return true;
        }

        HttpAssetTransfer* t = new HttpAssetTransfer();
        t->SetAssetId(asset_id);
        t->SetAssetType(asset_type_int);
        t->InsertTag(tag);
        asset_transfers_[asset_id] = HttpAssetTransferPtr(t);
        t->StartTransfer();
        return true;
    }

    bool HttpAssetProvider::InProgress(const std::string& asset_id)
    {
        HttpAssetTransfer* transfer = GetTransfer(asset_id);
        return (transfer != 0);
        return false;
    }

    Foundation::AssetPtr HttpAssetProvider::GetIncompleteAsset(const std::string& asset_id, const std::string& asset_type, uint received)
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

    bool HttpAssetProvider::QueryAssetStatus(const std::string& asset_id, uint& size, uint& received, uint& received_continuous)    
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

    void HttpAssetProvider::Update(f64 frametime)
    {
        HttpAssetTransferMap::iterator i = asset_transfers_.begin();
        while (i != asset_transfers_.end())
        {
            HttpAssetTransferPtr t = i->second;

            if (t->IsFailed())
            {
                std::stringstream buf;
                buf << "HttpAssetTransfer is failed for: ";
                buf << t->GetAssetId();
                AssetModule::LogError(buf.str());

                SendAssetCanceled(*t.get());

                asset_transfers_.erase(i);
                return;
            }

            t->Update(frametime);
            SendAssetProgress(*t.get());

            if (t->Ready())
            {
                StoreAsset(*t.get());
                asset_transfers_.erase(i);
                return;
            }
            else
            {
                t->AddTime(frametime);
                if (t->GetTime() > asset_timeout_)
                {
                    std::stringstream buf;
                    buf << "HttpAssetTransfer time out for: ";
                    buf << t->GetAssetId();
                    AssetModule::LogError(buf.str());

                    SendAssetCanceled(*t.get());

                    asset_transfers_.erase(i);
                    return;
                }
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
        
        return 0;
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

       
#if defined(__GNUC__)
        RexAssetMetadata* m = dynamic_cast<RexAssetMetadata*>(new_asset->GetMetadata());
#else       
        Foundation::AssetMetadataInterface* metadata = new_asset->GetMetadata();
        RexAssetMetadata* m = static_cast<RexAssetMetadata*>(metadata);
#endif    
        m->DesesrializeFromJSON(transfer.GetAssetMetadata());
      
            asset_service->StoreAsset(new_asset);
            
            // Send asset ready event for each request tag
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            const RequestTagVector& tags = transfer.GetTags();
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

    std::string HttpAssetProvider::SerializeToJSON(Foundation::AssetPtr asset) const
    {
#if defined(__GNUC__)
        RexAssetMetadata* metadata = dynamic_cast<RexAssetMetadata*>(asset->GetMetadata());
#else
        RexAssetMetadata* metadata = static_cast<RexAssetMetadata*>(asset->GetMetadata());
#endif        
        std::stringstream s;

        s << "{";
        s << "\"id\":\"" << metadata->GetId() << "\",";
        s << "\"name\":\"" << metadata->GetName() << "\",";
        s << "\"description\":\"" << metadata->GetDescription() << "\",";
        s << "\"type\":\"" << metadata->GetContentType() << "\",";
        if (metadata->IsTemporary())
            s << "\"temporary\":" << "true" << ",";
        else
            s << "\"temporary\":" << "false" << ",";
        s << "\"data\":\"";
        Poco::Base64Encoder encoder(s);

        encoder.write((char*)asset->GetData(),asset->GetSize());
        encoder.flush();
        encoder.close();
        s << "\"";
        s << "}";

        std::string t = s.str();

        return s.str();
    }

} // end of namespace
