// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AssetDefines.h"
#include "AssetInterface.h"
#include "ResourceInterface.h"
#include "TextureDecoderModule.h"
#include "Texture.h"
#include "TextureService.h"

#include <openjpeg.h>

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
            event_manager->RegisterEvent(resourcecategory_id_, Resource::Event::RESOURCE_READY, "ResourceReady");
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
            return; // already requested
            
        TextureRequest new_request(asset_id);
        requests_[asset_id] = new_request;
    }
    
    void TextureService::Update(Core::f64 frametime)
    {
        Foundation::AssetServiceInterface* asset_service = NULL;

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager(); 
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
        {
            asset_service = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset);
        }
        else
        {
            // No asset service, clear any pending requests and do nothing
            if (requests_.size())
                requests_.clear();
            return;
        }
            
        //! todo: check/service only a couple of requests per frame, not all
        TextureRequestMap::iterator i = requests_.begin();     
        while (i != requests_.end())
        {
            UpdateRequest(i->second, asset_service);
            ++i;
        }

        // Check for any decode results
        DecodeResult result;
        while (decoder_.GetResult(result))
        {
            i = requests_.find(result.id_);
            if (i != requests_.end())
            {
                if (UpdateRequestWithResult(i->second, result)) 
                    requests_.erase(i);
            }
        }
    }
    
    void TextureService::UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service)
    {
        // If pending decode request, do nothing; wait for the result
        if (request.IsDecodeRequested())
            return;

        // If asset not requested yet, get the request running now
        if (!request.IsRequested())
        {
            asset_service->RequestAsset(request.GetId(), Asset::RexAT_Texture);
            request.SetRequested(true);
        }
        
        Core::uint size = 0;
        Core::uint received = 0;
        Core::uint received_continuous = 0;
        
        if (!asset_service->QueryAssetStatus(request.GetId(), size, received, received_continuous))
        {
            // If cannot query asset status, the asset request wasn't queued (not connected, for example). Request again later       
            request.SetRequested(false);
            return;
        }

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
    
    bool TextureService::UpdateRequestWithResult(TextureRequest& request, DecodeResult& result)
    {     
        request.SetDecodeRequested(false);

        // Update texture amount of quality levels, should now be known
        request.SetLevels(result.max_levels_);

        // See if successfully decoded data
        if (result.texture_)
        {
            request.DecodeSuccess();     

            // Update texture original dimensions, should now be known
            request.SetSize(result.original_width_, result.original_height_, result.components_);
      
            Texture* texture = checked_static_cast<Texture*>(result.texture_.get());
            TextureDecoderModule::LogInfo("Decoded texture w " + Core::ToString<Core::uint>(texture->GetWidth()) + " h " +
                Core::ToString<Core::uint>(texture->GetHeight()) + " level " + Core::ToString<int>(request.GetDecodedLevel()));

            // Send resource ready event
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            Resource::Event::ResourceReady event_data(request.GetId(), result.texture_);
            event_manager->SendEvent(resourcecategory_id_, Resource::Event::RESOURCE_READY, &event_data);

            // If max level, the request is finished and can be erased
            if (request.GetDecodedLevel() == 0)
                return true;
        }

        request.SetNextLevelToDecode();
        return false;
    }
}

