// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_Decoder_h
#define incl_TextureDecoder_Decoder_h

#include "TextureRequest.h"
#include "TextureServiceInterface.h"
#include "TextureCache.h"

namespace Foundation
{
    class Framework;
    class AssetServiceInterface;
}

namespace TextureDecoder
{
    class TextureResource;

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
            \return request tag, will be used in eventual RESOURCE_READY event
         */
        virtual request_tag_t RequestTexture(const std::string& asset_id);
        
        //! Updates texture requests. Called by TextureDecoderModule
        void Update(f64 frametime);

        //! Handles an asset event. Called by TextureDecoderModule
        bool HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data);
        
        //! Handles a thread task event. Called by TextureDecoderModule
        bool HandleTaskEvent(event_id_t event_id, Foundation::EventDataInterface* data);
        
    private:
        //! Updates a texture request
        /*! Polls the asset service & queues decode requests to the decode thread as necessary
         */
        void UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service);

        typedef std::map<std::string, TextureRequest> TextureRequestMap;

        typedef std::map<std::string, CacheReply> CacheReplys;
        
        //! Framework we belong to
        Foundation::Framework* framework_;
                
        //! Resource event category
        event_category_id_t resource_event_category_;

        //! Ongoing texture requests
        TextureRequestMap requests_;

        //! Decoded texture cache
        TextureCache *cache_;

        CacheReplys cache_replys_;

        //! Max decodes per frame
        int max_decodes_per_frame_;
    };
}

#endif
