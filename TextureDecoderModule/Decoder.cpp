// For conditions of distribution and use, see copyright notice in license.txt

// based on OpenJpeg j2k_to_image.c sample code

/*
 * Copyright (c) 2002-2007, Communications and Remote Sensing Laboratory, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2007, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux and Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2006-2007, Parvatha Elangovan
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "StableHeaders.h"
#include "AssetDefines.h"
#include "AssetInterface.h"
#include "TextureDecoderModule.h"
#include "TextureEvents.h"
#include "Texture.h"
#include "Decoder.h"

#include <openjpeg.h>

namespace TextureDecoder
{

    Decoder::Decoder(Foundation::Framework* framework) : 
        framework_(framework)
    {
        Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

        event_category_ = event_manager->RegisterEventCategory("Texture");
        event_manager->RegisterEvent(event_category_, Event::TEXTURE_READY, "TextureReady");
    }
    
    Decoder::~Decoder()
    {
    }

    void Decoder::QueueTextureRequest(const std::string& asset_id)
    {
        if (requests_.find(asset_id) != requests_.end())
            return; // already requested
            
        TextureRequest new_request(asset_id);
        requests_[asset_id] = new_request;
    }
    
    void Decoder::Update(Core::f64 frametime)
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
            if (UpdateRequest(i->second, asset_service))
                i = requests_.erase(i);
            else
                ++i;
        }
    }
    
    bool Decoder::UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service)
    {
        // If asset not requested yet, get the request running now
        if (!request.IsRequested())
        {
            asset_service->RequestAsset(request.GetAssetId(), Asset::RexAT_Texture);
            request.SetRequested(true);
        }
        
        Core::uint size = 0;
        Core::uint received = 0;
        Core::uint received_continuous = 0;
        
        if (!asset_service->QueryAssetStatus(request.GetAssetId(), size, received, received_continuous))
        {
            // If cannot query asset status, the asset request wasn't queued (not connected, for example). Request again later       
            request.SetRequested(false);
            return false;
        }

        request.UpdateSizeReceived(size, received_continuous);

        if (request.HasEnoughData())
        {
            bool success = DecodeNextLevel(request, asset_service);
            request.SetNextLevelToDecode();

            // If max level, the request is finished and can be erased
            if ((success) && (request.GetDecodedLevel() == 0))
                return true;
        }
        
        return false;
    }
    

    void HandleError(const char *msg, void *client_data)
    {
        //if (msg)
        //    TextureDecoderModule::LogError("Texture decode error " + std::string(msg));
    }
    
    void HandleWarning(const char *msg, void *client_data)
    {
    }

    void HandleInfo(const char *msg, void *client_data)
    {
    }

    bool Decoder::DecodeNextLevel(TextureRequest& request, Foundation::AssetServiceInterface* asset_service)
    {
        bool success = false;

        Foundation::AssetPtr asset = asset_service->GetIncompleteAsset(request.GetAssetId(), Asset::RexAT_Texture, request.GetReceived());
        if (!asset)
            return false;
        
        opj_dinfo_t* dinfo = NULL; // decoder
        opj_image_t *image = NULL; // decoded image
        opj_dparameters_t parameters; // decoder parameters
        opj_cio_t *cio = NULL; // decode stream
        opj_codestream_info_t cstr_info;  // codestream info
        memset(&cstr_info, 0, sizeof(opj_codestream_info_t));
       
        opj_event_mgr_t event_mgr; // decode event manager
        memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
        event_mgr.error_handler = HandleError;
        //event_mgr.warning_handler = HandleWarning;
        //event_mgr.info_handler = HandleInfo;
        
        opj_set_default_decoder_parameters(&parameters);
        parameters.cp_reduce = request.GetNextLevel();
        
        dinfo = opj_create_decompress(CODEC_J2K);
        opj_setup_decoder(dinfo, &parameters);
        opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, this);
        
        cio = opj_cio_open((opj_common_ptr)dinfo, (unsigned char *)asset->GetData(), asset->GetSize());
        
        image = opj_decode_with_info(dinfo, cio, &cstr_info);
        request.SetLevels(cstr_info.numlayers);

        opj_cio_close(cio);
        opj_destroy_decompress(dinfo);
        
        if ((image) && (image->numcomps))
        {
            request.DecodeSuccess();
            request.SetSize(image->x1 - image->x0, image->y1 - image->y0, image->numcomps);

            TextureDecoderModule::LogInfo("Texture decode successful, level " + Core::ToString<int>(request.GetDecodedLevel()));
        
            TextureDecoderModule::LogInfo("Width: " + Core::ToString<int>(request.GetWidth() >> request.GetDecodedLevel()) + 
                                          " Height: " + Core::ToString<int>(request.GetHeight() >> request.GetDecodedLevel()) + 
                                          " Components: " + Core::ToString<int>(image->numcomps));

            // Assume all components are same size
            int actual_width = image->comps[0].w;
            int actual_height = image->comps[0].h;

            // Create a texture object
            Foundation::TexturePtr texture(new Texture(request.GetAssetId(), actual_width, actual_height, image->numcomps));
            Core::u8* data = texture->GetData();
            for (int y = 0; y < actual_width; ++y)
            {
                for (int x = 0; x < actual_height; ++x)
                {
                    for (int c = 0; c < image->numcomps; ++c)
                    {
                        *data = image->comps[c].data[y * actual_width + x];
                        data++;
                    }
                }
            }

            // Send texture ready event
            Foundation::EventManagerPtr event_manager = framework_->GetEventManager();
            Event::TextureReady event_data(request.GetAssetId(), request.GetDecodedLevel(), texture);
            event_manager->SendEvent(event_category_, Event::TEXTURE_READY, &event_data);

            success = true;
        }
        else
        {
            TextureDecoderModule::LogInfo("Texture decode failed");
        }

        if (image)
            opj_image_destroy(image);

        return success;
    }
}

