// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_Decoder_h
#define incl_TextureDecoder_Decoder_h

#include "TextureRequest.h"
#include "TextureServiceInterface.h"
#include "OpenJpegDecoder.h"

namespace Foundation
{
    class Framework;
    class AssetServiceInterface;
}

namespace TextureDecoder
{
    //! Texture decoder. Implements TextureServiceInterface.
    class TextureService : public Foundation::TextureServiceInterface
    {
    public:
        //! Constructor
        TextureService(Foundation::Framework* framework);
        
        //! Destructor
        virtual ~TextureService();

        //! Queues a texture request
        /*! \param asset_id asset ID of texture
         */
        virtual void RequestTexture(const std::string& asset_id);
        
        //! Updates texture requests. Called by TextureDecoderModule
        void Update(Core::f64 frametime);

        //! Handles an asset event. Called by TextureDecoderModule
        bool HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        //! Updates a texture request
        /*! Polls the asset service & queues decode requests to the decode thread as necessary
         */
        void UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service);

        typedef std::map<std::string, TextureRequest> TextureRequestMap;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
                
        //! Resource event category
        Core::event_category_id_t resourcecategory_id_;

        //! Ongoing texture requests
        TextureRequestMap requests_;

        //! Thread for actual decoding work
        Core::Thread thread_;

        //! OpenJpeg decoder that's run in a thread
        OpenJpegDecoder decoder_;
        
        //! Texture asset request retry interval
        Core::f64 asset_retry_interval_;
        
        //! Default asset request retry interval
        static const Core::Real DEFAULT_ASSET_RETRY_INTERVAL;   
    };
}

#endif
