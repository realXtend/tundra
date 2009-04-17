// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_Decoder_h
#define incl_TextureDecoder_Decoder_h

#include "TextureRequest.h"
#include "TextureServiceInterface.h"

namespace Foundation
{
    class Framework;
    class AssetServiceInterface;
}

namespace TextureDecoder
{
    //! texture decoder worker
    class Decoder : public Foundation::TextureServiceInterface
    {
    public:
        //! constructor
        Decoder(Foundation::Framework* framework);
        
        //! destructor
        ~Decoder();
        
        //! updates texture requests
        void Update(Core::f64 frametime);

        //! queues a texture request
        /*! \param asset_id asset ID of texture
         */
        void QueueTextureRequest(const std::string& asset_id);
        
    private:
        //! updates a texture request
        /*! \return true if highest quality level has been decoded and the request can be erased
         */
        bool UpdateRequest(TextureRequest& request, Foundation::AssetServiceInterface* asset_service);
        
        //! decodes next level of a texture request, as enough data has been received
        bool DecodeNextLevel(TextureRequest& request, Foundation::AssetServiceInterface* asset_service);
        
        typedef std::map<std::string, TextureRequest> TextureRequestMap;
        
        //! framework we belong to
        Foundation::Framework* framework_;
                
        //! texture event category
        Core::event_category_id_t event_category_;

        //! ongoing texture requests
        TextureRequestMap requests_;
    };
}

#endif
