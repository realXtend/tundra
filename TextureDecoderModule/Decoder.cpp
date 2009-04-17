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
#include "Decoder.h"

#include <openjpeg.h>

namespace TextureDecoder
{

    Decoder::Decoder(Foundation::Framework* framework) : 
        framework_(framework),
        asset_service_(NULL)
    {
        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();
        
        if (service_manager->IsRegistered(Foundation::Service::ST_Asset))
            asset_service_ = service_manager->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset);
    }
    
    Decoder::~Decoder()
    {
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

    void Decoder::RequestTexture(const std::string& asset_id)
    {
        if (requests_.find(asset_id) != requests_.end())
            return; // already requested
            
        TextureRequest new_request(asset_id);
        requests_[asset_id] = new_request;
    }
    
    void Decoder::Update(Core::f64 frametime)
    {
        if (!asset_service_)
            return;
            
        //! todo: check/service only a couple of requests per frame, not all
        TextureRequestMap::iterator i = requests_.begin();
        
        while (i != requests_.end())
        {
            if (UpdateRequest(i->second))
                i = requests_.erase(i);
            else
                ++i;
        }
    }
    
    bool Decoder::UpdateRequest(TextureRequest& request)
    {
        // If asset not requested yet, get the request running now
        if (!request.requested_)
        {
            asset_service_->GetAsset(request.asset_id_, Asset::RexAT_Texture);
            request.requested_ = true;
        }
        
        Core::uint size = 0;
        Core::uint received_discontinuous = 0;
        Core::uint received = 0;
        
        if (!asset_service_->QueryAssetStatus(request.asset_id_, size, received_discontinuous, received))
        {
            request.requested_ = false; // If cannot query asset status, the asset request wasn't queued (not connected, for example). Request again        
            return false;
        }

        if (size)
            request.size_ = size;
        request.received_ = received;
        
        // If all bytes received, can go directly to the max. quality level
        if ((size) && (received >= size))
            request.next_level_ = 0;
            
        if (received >= request.EstimateDataSize(request.next_level_))
        {
            bool success = DecodeNextLevel(request);
            if (request.next_level_)
                request.next_level_--;

            // If max level, the request is finished and can be erased
            if ((success) && (request.decoded_level_ == 0))
                return true;
        }
        
        return false;
    }
    
    bool Decoder::DecodeNextLevel(TextureRequest& request)
    {
        Foundation::AssetPtr asset = asset_service_->GetIncompleteAsset(request.asset_id_, Asset::RexAT_Texture, request.received_);
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
        parameters.cp_reduce = request.next_level_;
        
        dinfo = opj_create_decompress(CODEC_J2K);
        opj_setup_decoder(dinfo, &parameters);
        opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, this);
        
        cio = opj_cio_open((opj_common_ptr)dinfo, (unsigned char *)asset->GetData(), asset->GetSize());
        
        image = opj_decode_with_info(dinfo, cio, &cstr_info);
        request.levels_ = cstr_info.numlayers;

        opj_cio_close(cio);
        opj_destroy_decompress(dinfo);
        
        if (image)
        {
            request.decoded_level_ = request.next_level_;
            request.width_ = image->x1 - image->x0;
            request.height_ = image->y1 - image->y0;
            request.components_ = image->numcomps;

            TextureDecoderModule::LogInfo("Texture decode successful, level " + Core::ToString<int>(request.decoded_level_));
        
            TextureDecoderModule::LogInfo("Width: " + Core::ToString<int>(request.width_ >> request.decoded_level_) + 
                                          " Height: " + Core::ToString<int>(request.height_ >> request.decoded_level_) + 
                                          " Components: " + Core::ToString<int>(image->numcomps));

            //for (int i = 0; i < image->numcomps; ++i)
            //{
            //    TextureDecoderModule::LogInfo("Component " + Core::ToString<int>(i));
            //    TextureDecoderModule::LogInfo("Data width " + Core::ToString<int>(image->comps[i].w) + 
            //        " Data height " + Core::ToString<int>(image->comps[i].h) + 
            //        " Factor " + Core::ToString<int>(image->comps[i].factor));
            //}
            
            opj_image_destroy(image);
            return true;
        }
        else
        {
            TextureDecoderModule::LogInfo("Texture decode failed");
        }

        return false;
    }
}

