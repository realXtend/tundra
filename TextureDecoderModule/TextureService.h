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
    //! texture decoder service interface
    class TextureService : public Foundation::TextureServiceInterface
    {
    public:
        //! constructor
        TextureService(Foundation::Framework* framework);
        
        //! destructor
        ~TextureService();
        
        //! updates texture requests
        void Update(Core::f64 frametime);

        //! queues a texture request
        /*! \param asset_id asset ID of texture
         */
        void RequestTexture(const std::string& asset_id);
        
    private:
        //! updates a texture request
        /*! queues decode requests to the decode thread as necessary
         */
        void UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service);
        
        //! processes a decode result from the decode thread
        /*! \return true if texture request has reached maximum quality level and can be removed
         */
        bool UpdateRequestWithResult(TextureRequest& request, DecodeResult& result);

        typedef std::map<std::string, TextureRequest> TextureRequestMap;
        
        //! framework we belong to
        Foundation::Framework* framework_;
                
        //! resource event category
        Core::event_category_id_t resourcecategory_id_;

        //! ongoing texture requests
        TextureRequestMap requests_;

        //! thread for actual decoding work
        Core::Thread thread_;

        //! openjpeg decoder that's run in a thread
        OpenJpegDecoder decoder_;
        
    };
}

#endif
