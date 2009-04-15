// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_Decoder_h
#define incl_TextureDecoder_Decoder_h

namespace Foundation
{
    class Framework;
    class AssetServiceInterface;
}

namespace TextureDecoder
{
    //! texture decoder worker
    class Decoder
    {
    public:
        //! constructor
        Decoder(Foundation::Framework* framework);
        
        //! destructor
        ~Decoder();

        //! decode a texture
        /*! \param asset_id asset ID of texture
            \param reduction quality reduction ("mipmap") level. 0 for full quality
         */
        void DecodeTexture(const std::string& asset_id, Core::uint reduction = 0);
        
        //! framework we belong to
        Foundation::Framework* framework_;
        
        //! asset service
        Foundation::AssetServiceInterface* asset_service_;
    };
}

#endif
