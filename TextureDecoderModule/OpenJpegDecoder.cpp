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
#include "Texture.h"
#include "TextureDecoderModule.h"
#include "OpenJpegDecoder.h"

#include <openjpeg.h>

namespace TextureDecoder
{
    OpenJpegDecoder::OpenJpegDecoder() : running_(true)
    {
    }

    OpenJpegDecoder::~OpenJpegDecoder()
    {
    }

    void OpenJpegDecoder::operator ()()
    {
        while (running_)
        {
            boost::this_thread::interruption_point();

            DecodeRequest current_request;
            bool have_request = false;

            {
                Core::MutexLock lock(request_mutex_);
                if (!requests_.empty())
                {
                    have_request = true;
                    current_request = requests_.front();
                    requests_.pop_front();
                }
            }
            
            if (have_request)
                PerformDecode(current_request);
            else
                boost::this_thread::sleep(boost::posix_time::milliseconds(20));
        }
    }

    void OpenJpegDecoder::AddRequest(const DecodeRequest& request)
    {
        Core::MutexLock lock(request_mutex_);

        requests_.push_back(request);
    }

    bool OpenJpegDecoder::GetResult(DecodeResult& result)
    {
        Core::MutexLock lock(result_mutex_);

        if (results_.empty())
            return false;
        else
        {
            result = results_.front();
            results_.pop_front();
            return true;
        }
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

    void OpenJpegDecoder::PerformDecode(DecodeRequest& request)
    {
        DecodeResult result;

        result.id_ = request.id_;
        result.max_levels_ = 5;
        result.original_width_ = 0;
        result.original_height_ = 0;
        result.components_ = 0;

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
        parameters.cp_reduce = request.level_;
        
        dinfo = opj_create_decompress(CODEC_J2K);
        opj_setup_decoder(dinfo, &parameters);
        opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, this);
        
        cio = opj_cio_open((opj_common_ptr)dinfo, (unsigned char *)request.source_->GetData(), request.source_->GetSize());
        
        image = opj_decode_with_info(dinfo, cio, &cstr_info);
        result.max_levels_ = cstr_info.numlayers;

        opj_cio_close(cio);
        opj_destroy_decompress(dinfo);
        
        if ((image) && (image->numcomps))
        {
            result.original_width_ = image->x1 - image->x0;
            result.original_height_ = image->y1 - image->y0;
            result.components_ = image->numcomps;

            // Assume all components are same size
            int actual_width = image->comps[0].w;
            int actual_height = image->comps[0].h;

            // Create a (possibly temporary, if no-one stores the pointer) raw texture resource
            Foundation::ResourcePtr resource(new Texture(request.source_->GetId(), actual_width, actual_height, image->numcomps));
            Texture* texture = checked_static_cast<Texture*>(resource.get());
            Core::u8* data = texture->GetData();
            texture->SetLevel(request.level_);
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
     
            result.texture_ = resource;
        }

        if (image)
            opj_image_destroy(image);

        {
            Core::MutexLock lock(result_mutex_);
            results_.push_back(result);
        }
    }
}