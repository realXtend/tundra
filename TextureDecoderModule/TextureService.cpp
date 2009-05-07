// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetDefines.h"
#include "AssetEvents.h"
#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "TextureDecoderModule.h"
#include "Texture.h"
#include "TextureService.h"

namespace TextureDecoder
{
    TextureService::TextureService(Foundation::Framework* framework) : 
        framework_(framework)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        resourcecategory_id_ = event_manager->QueryEventCategory("Resource");
        if (!resourcecategory_id_)
        {
            resourcecategory_id_ = event_manager->RegisterEventCategory("Resource");
            event_manager->RegisterEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, "ResourceReady");
        }

        thread_ = boost::thread(boost::ref(decoder_));
    }
    
    TextureService::~TextureService()
    {
        decoder_.Stop();
        thread_.join();
    }

    void TextureService::RequestTexture(const std::string& asset_id)
    {
        if (requests_.find(asset_id) != requests_.end())
            return; // Already requested
     
        TextureRequest new_request(asset_id);                   
        requests_[asset_id] = new_request;
    }
    
    void TextureService::Update(Core::f64 frametime)
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

        // Check if assets have enough data to queue decode requests
        TextureRequestMap::iterator i = requests_.begin();     
        while (i != requests_.end())
        {
            UpdateRequest(i->second, asset_service.get());
            ++i;
        }

        // Check for any decode results
        DecodeResult result;
        while (decoder_.GetResult(result))
        {
            i = requests_.find(result.id_);
            if (i != requests_.end())
            {
                bool done = i->second.UpdateWithDecodeResult(result);
      
                if (result.texture_)
                {
                    Texture* texture = checked_static_cast<Texture*>(result.texture_.get());
                    TextureDecoderModule::LogInfo("Decoded texture w " + Core::ToString<Core::uint>(texture->GetWidth()) + " h " +
                        Core::ToString<Core::uint>(texture->GetHeight()) + " level " + Core::ToString<int>(result.level_));
    
                    // Send resource ready event
                    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
                    Resource::Events::ResourceReady event_data(i->second.GetId(), result.texture_);
                    event_manager->SendEvent(resourcecategory_id_, Resource::Events::RESOURCE_READY, &event_data);          
                }   
                
                // Remove request if final quality level was decoded
                if (done)
                    requests_.erase(i);                
            }
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
            asset_service->RequestAsset(request.GetId(), Asset::RexAT_Texture);
            request.SetRequested(true);
        }

        Core::uint size = 0;
        Core::uint received = 0;
        Core::uint received_continuous = 0;
             
        if (!asset_service->QueryAssetStatus(request.GetId(), size, received, received_continuous))
            return;
        
        request.UpdateSizeReceived(size, received_continuous);

        if (request.HasEnoughData())
        {
            // Queue decode request to decode thread     
            Foundation::AssetPtr asset = asset_service->GetIncompleteAsset(request.GetId(), Asset::RexAT_Texture, request.GetReceived());
            if (asset)
            {
                DecodeRequest new_request;
                new_request.id_ = request.GetId();
                new_request.level_ = request.GetNextLevel();
                new_request.source_ = asset;
                decoder_.AddRequest(new_request);
                request.SetDecodeRequested(true);
            }
        }
    }  
    
    bool TextureService::HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Asset::Events::ASSET_CANCELED)
        {
            Asset::Events::AssetCanceled* event_data = checked_static_cast<Asset::Events::AssetCanceled*>(data);
            TextureRequestMap::iterator i = requests_.find(event_data->asset_id_);
            if (i != requests_.end())
            {
                TextureDecoderModule::LogInfo("Texture decode request " + i->second.GetId() + " canceled");
                requests_.erase(i);
            }
        }
                
        return false;
    }     
}

