// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetEvents.h"
#include "AssetInterface.h"
#include "OpenJpegDecoder.h"
#include "ResourceInterface.h"
#include "TextureDecoderModule.h"
#include "TextureResource.h"
#include "TextureService.h"
#include "RexTypes.h"
#include "AssetServiceInterface.h"
#include "Framework.h"
#include "EventManager.h"
#include "ServiceManager.h"
#include "ThreadTaskManager.h"
#include "ConfigurationManager.h"

namespace TextureDecoder
{
    static const int DEFAULT_MAX_DECODES = 4;
    
    TextureService::TextureService(Foundation::Framework* framework) : 
        framework_(framework)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        resource_event_category_ = event_manager->QueryEventCategory("Resource");

        max_decodes_per_frame_ = framework_->GetDefaultConfig().DeclareSetting("TextureDecoder", "max_decodes_per_frame", DEFAULT_MAX_DECODES);
        if (max_decodes_per_frame_ <= 0) 
            max_decodes_per_frame_ = 1;

        // Create decoder thread task and let the framework thread task manager handle it
        OpenJpegDecoder* decoder = new OpenJpegDecoder();
        decoder->SetDecodesPerFrame(max_decodes_per_frame_);

        framework_->GetThreadTaskManager()->AddThreadTask(Foundation::ThreadTaskPtr(decoder));
    }
    
    TextureService::~TextureService()
    {
    }

    request_tag_t TextureService::RequestTexture(const std::string& asset_id)
    {
        request_tag_t tag = framework_->GetEventManager()->GetNextRequestTag();
    
        if (requests_.find(asset_id) != requests_.end())
        {
            // Already requested, just add request tag
            requests_.find(asset_id)->second.InsertTag(tag);
            return tag; 
        }
     
        TextureRequest new_request(asset_id); 
        new_request.InsertTag(tag);
        requests_[asset_id] = new_request;
        
        return tag;
    }
    
    void TextureService::Update(f64 frametime)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (!service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            // No asset service, clear asset request status of requests & do nothing
            TextureRequestMap::iterator i = requests_.begin();
            while (i != requests_.end())
            {
                i->second.SetRequested(false);
                ++i;
            }
            return;
        }

        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (!asset_service)
            return;

        // Check if assets have enough data to queue decode requests
        TextureRequestMap::iterator i = requests_.begin();
        while (i != requests_.end())
        {
            UpdateRequest(i->second, asset_service.get());
            ++i;
        }
    }
    
    void TextureService::UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service)
    {
        // If pending decode request, do nothing; wait for the result
        if (request.IsDecodeRequested())
            return;

        // If asset not yet requested, request now
        if (!request.IsRequested())
        {
            asset_service->RequestAsset(request.GetId(), "Texture");
            request.SetRequested(true);
        }

        uint size = 0;
        uint received = 0;
        uint received_continuous = 0;
             
        if (!asset_service->QueryAssetStatus(request.GetId(), size, received, received_continuous))
            return;
        
        request.UpdateSizeReceived(size, received_continuous);

        if (request.HasEnoughData())
        {
            // Queue decode request to decode thread
            Foundation::AssetPtr asset = asset_service->GetIncompleteAsset(request.GetId(), RexTypes::ASSETTYPENAME_TEXTURE, request.GetReceived());
            if (asset)
            {
                DecodeRequestPtr new_decode_request(new DecodeRequest());
                new_decode_request->id_ = request.GetId();
                new_decode_request->level_ = request.GetNextLevel();
                new_decode_request->source_ = asset;
                framework_->GetThreadTaskManager()->AddRequest<DecodeRequest>("TextureDecoder", new_decode_request);
                
                request.SetDecodeRequested(true);
            }
        }
    }  
    
    bool TextureService::HandleTaskEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id != Task::Events::REQUEST_COMPLETED)
            return false;
        DecodeResult* result = dynamic_cast<DecodeResult*>(data);
        if (!result || result->task_description_ != "TextureDecoder")
            return false;
        
        TextureRequestMap::iterator i = requests_.find(result->id_);
        if (i != requests_.end())
        {
            bool done = i->second.UpdateWithDecodeResult(result);
  
            if (result->texture_)
            {
                TextureResource* texture = checked_static_cast<TextureResource*>(result->texture_.get());
                TextureDecoderModule::LogDebug("Decoded texture w " + ToString<uint>(texture->GetWidth()) + " h " +
                    ToString<uint>(texture->GetHeight()) + " level " + ToString<int>(result->level_));

                // Send resource ready event for each request tag in the request
                const RequestTagVector& tags = i->second.GetTags();

                Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
                for (uint j = 0; j < tags.size(); ++j)
                { 
                    Resource::Events::ResourceReady event_data(i->second.GetId(), result->texture_, tags[j]);
                    event_manager->SendEvent(resource_event_category_, Resource::Events::RESOURCE_READY, &event_data);    
                }      
            }   
            
            // Remove request if final quality level was decoded
            if (done)
                requests_.erase(i);
        }
        
        return true;
    }
    
    bool TextureService::HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Asset::Events::ASSET_CANCELED)
        {
            Asset::Events::AssetCanceled* event_data = checked_static_cast<Asset::Events::AssetCanceled*>(data);
            TextureRequestMap::iterator i = requests_.find(event_data->asset_id_);
            if (i != requests_.end())
            {
                TextureDecoderModule::LogDebug("Texture decode request " + i->second.GetId() + " canceled");
                
                // Send a RESOURCE_CANCELED event for each request that was made for this texture
                const RequestTagVector& tags = i->second.GetTags();
                for (uint j = 0; j < tags.size(); ++j)
                {
                    Resource::Events::ResourceCanceled canceled_event_data(i->second.GetId(), tags[j]);
                    framework_->GetEventManager()->SendEvent(resource_event_category_, Resource::Events::RESOURCE_CANCELED, &canceled_event_data);
                }
                
                requests_.erase(i);
            }
        }
                
        return false;
    }     
}

