// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureRequest_h
#define incl_TextureDecoder_TextureRequest_h

namespace TextureDecoder
{
    class TextureRequest
    {
    public:
        //! constructor
        TextureRequest();
        
        //! constructor
        TextureRequest(const std::string& asset_id);
        
        //! destructor
        ~TextureRequest();

        //! estimate needed data size 
        Core::uint EstimateDataSize(int level);

        //! asset on which this request is based
        std::string asset_id_;
        //! whether asset request has been queued
        bool requested_;
        //! total data size, 0 if unknown
        Core::uint size_;
        //! last checked size of download progress (continuous received bytes)
        Core::uint received_;
        //! texture original width, 0 if unknown
        Core::uint width_;
        //! texture original height, 0 if unknown
        Core::uint height_;
        //! components in image, 0 if unknown
        Core::uint components_;
        //! number of quality levels, -1 if unknown
        int levels_;
        //! last decoded quality level, 0 = full quality, -1 if none decoded so far
        int decoded_level_;
        //! next quality level to decode
        int next_level_;
    };
}
#endif