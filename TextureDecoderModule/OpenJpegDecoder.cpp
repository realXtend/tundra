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
#include "CoreDefines.h"
#include "TextureResource.h"
#include "TextureDecoderModule.h"
#include "ThreadTaskManager.h"
#include "OpenJpegDecoder.h"
#include "Profiler.h"

#include <openjpeg.h>

#include <QImage>

namespace TextureDecoder
{
    OpenJpegDecoder::OpenJpegDecoder() :
        Foundation::ThreadTask("TextureDecoder"),
        decodes_per_frame_(1)
    {
    }
    
    void OpenJpegDecoder::SetDecodesPerFrame(uint decodes) 
    { 
        if (decodes)
            decodes_per_frame_ = decodes;
    }
    
    void OpenJpegDecoder::Work()
    {
        while (ShouldRun())
        {
            WaitForRequests();
            
            DecodeRequestPtr request = GetNextRequest<DecodeRequest>();
            if (request)
            {
                // Wait if "too many" results already produced, to prevent slowing down the main thread with 
                // too many texture creations per frame
                Foundation::ThreadTaskManager* manager = GetThreadTaskManager();
                if (manager)
                {
                    for (;;)
                    {
                        uint results = manager->GetNumResults(GetTaskDescription());
                        if (results < decodes_per_frame_)
                            break;
                        if (!ShouldRun())
                            return;
                        boost::this_thread::sleep(boost::posix_time::milliseconds(20));
                    }
                }
                
                {
                    PROFILE(OpenJpegDecoder_Decode);
                    PerformDecode(request);
                }
            }

            RESETPROFILER
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

    void OpenJpegDecoder::PerformDecode(DecodeRequestPtr request)
    {
        if (!request)
            return;

        bool texture_id_is_url = QString(request->id_.c_str()).startsWith("http");

        DecodeResultPtr result(new DecodeResult());

        result->id_ = request->id_;
        result->level_ = -1; // no level decoded yet
        result->max_levels_ = 5;
        result->original_width_ = 0;
        result->original_height_ = 0;
        result->components_ = 0;
        result->tag_ = request->tag_;

        if (!texture_id_is_url)
        {
            // Guard against OpenJpeg crash on illegal data at an early phase
            unsigned char *data = (unsigned char *)request->source_->GetData();
            if (data[0] != 0xFF)
            {
                TextureDecoderModule::LogError("Invalid data passed to PerformDecode!");
                QueueResult<DecodeResult>(result);
                return;
            }

            opj_dinfo_t* dinfo = 0; // decoder
            opj_image_t *image = 0; // decoded image
            opj_dparameters_t parameters; // decoder parameters
            opj_cio_t *cio = 0; // decode stream
            opj_codestream_info_t cstr_info;  // codestream info
            memset(&cstr_info, 0, sizeof(opj_codestream_info_t));
           
            opj_event_mgr_t event_mgr; // decode event manager
            memset(&event_mgr, 0, sizeof(opj_event_mgr_t));
            event_mgr.error_handler = HandleError;
            //event_mgr.warning_handler = HandleWarning;
            //event_mgr.info_handler = HandleInfo;
            
            opj_set_default_decoder_parameters(&parameters);
            parameters.cp_reduce = request->level_;
            
            dinfo = opj_create_decompress(CODEC_J2K);
            opj_setup_decoder(dinfo, &parameters);
            opj_set_event_mgr((opj_common_ptr)dinfo, &event_mgr, this);
            
            cio = opj_cio_open((opj_common_ptr)dinfo, (unsigned char *)request->source_->GetData(), request->source_->GetSize());
            
            image = opj_decode_with_info(dinfo, cio, &cstr_info);
            result->max_levels_ = cstr_info.numlayers;

            opj_cio_close(cio);
            opj_destroy_decompress(dinfo);
            
            if ((image) && (image->numcomps))
            {
                result->original_width_ = image->x1 - image->x0;
                result->original_height_ = image->y1 - image->y0;
                result->components_ = image->numcomps;
                result->level_ = request->level_;

                // Assume all components are same size
                int actual_width = image->comps[0].w;
                int actual_height = image->comps[0].h;

                // Create a (possibly temporary, if no-one stores the pointer) raw texture resource
                Foundation::ResourcePtr resource(new TextureResource(request->source_->GetId(), actual_width, actual_height, image->numcomps));
                TextureResource* texture = checked_static_cast<TextureResource*>(resource.get());
                u8* data = texture->GetData();
                texture->SetLevel(request->level_);
                texture->SetDataSize(actual_width * actual_height * image->numcomps);

                for (int y = 0; y < actual_height; ++y)
                {
                    for (int x = 0; x < actual_width; ++x)
                    {
                        for (int c = 0; c < image->numcomps; ++c)
                        {
                            *data = image->comps[c].data[y * actual_width + x];
                            data++;
                        }
                    }
                }
         
                result->texture_ = resource;
                result->is_jpeg2000_ = true;
            }

            if (image)
                opj_image_destroy(image);
        }
        else
        {
            // Try to load image with qt, see supported image types from
            // http://doc.trolltech.com/4.6/qimagereader.html#supportedImageFormats

            QImage raw_image = QImage::fromData((const uchar *)request->source_->GetData(), request->source_->GetSize());
            if (!raw_image.isNull())
            {
                int width = raw_image.width();
                int height = raw_image.height();
                int ogre_format = -1;
                uint comps = 0;

                // Check image format, support the ones 
                // we can map to ogres image formats later
                switch (raw_image.format())
                {
                    case QImage::Format_RGB16:
                        ogre_format = 6;
                        comps = 3;
                        break;

                    case QImage::Format_RGB32:
                        ogre_format = 26;
                        comps = 4;
                        break;

                    case QImage::Format_ARGB32:
                    case QImage::Format_ARGB32_Premultiplied:
                        ogre_format = 12;
                        comps = 4;
                        break;

                    default:
                        ogre_format = -1;
                        comps = 0;
                        break;
                }

                if (comps != 0 && ogre_format != -1)
                {
                    result->original_width_ = width;
                    result->original_height_ = height;
                    result->components_ = comps;
                    result->level_ = 0;

                    Foundation::ResourcePtr resource(new TextureResource(request->source_->GetId(), width, height, comps));
                    TextureResource* texture = checked_static_cast<TextureResource*>(resource.get());
                    texture->SetFormat(ogre_format);
                    texture->SetLevel(0);
                    texture->SetDataSize(raw_image.byteCount());
                    u8* data = texture->GetData();
                    
                    memcpy(data, raw_image.bits(), raw_image.byteCount());
                    result->texture_ = resource;
                    result->is_jpeg2000_ = false;
                }
                else
                {
                    TextureDecoderModule::LogError(QString("Your image was loaded as but QImage::Format = %1 is not supported. Canceling texture usage.").arg(raw_image.format()).toStdString());
                }
            }
            else
            {
                TextureDecoderModule::LogError("Could load texture image with qt. Are you sure you have all qt image format plugins loaded? Canceling texture usage.");
            }

        }

        QueueResult<DecodeResult>(result);
    }
}
