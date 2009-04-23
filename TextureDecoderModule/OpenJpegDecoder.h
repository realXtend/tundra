// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_OpenJpegDecoder_h
#define incl_TextureDecoder_OpenJpegDecoder_h

#include "AssetInterface.h"
#include "TextureInterface.h"

namespace TextureDecoder
{
    //! decode request
    struct DecodeRequest
    {
        //! texture asset id
        std::string id_;

        //! source asset data
        Foundation::AssetPtr source_;

        //! quality level, 0 = highest
        int level_;
    };

    //! decode result
    struct DecodeResult
    {
        //! texture asset id
        std::string id_;
        
        //! pointer to resulting raw texture, NULL if decode failed
        Foundation::ResourcePtr texture_;

        //! amount of quality levels found
        int max_levels_;

        //! original texture width 
        Core::uint original_width_;

        //! original texture height
        Core::uint original_height_;

        //! amount of components in texture
        Core::uint components_;
    };

    //! OpenJpeg decoder that runs in a thread and serves decode requests, used internally by TextureService
    class OpenJpegDecoder
    {
    public:
        //! constructor
        OpenJpegDecoder();

        //! destructor
        ~OpenJpegDecoder();

        //! (thread) entry point
        void operator()();

        //! adds a decode request to the thread
        /*! \param request a filled DecodeRequest structure
         */
        void AddRequest(const DecodeRequest& request);

        //! gets the next decode result 
        /*! \param result a DecodeResult structure to be filled with the result
            \return true if a result was available and the structure was filled
         */
        bool GetResult(DecodeResult& result);  

    private:
        //! perform decode, runs in the decode thread
        /*! \param request decode request to serve
         */
        void PerformDecode(DecodeRequest& request);

        //! request queue
        std::list<DecodeRequest> requests_;
 
        //! request queue mutex
        Core::Mutex request_mutex_;

        //! result queue
        std::list<DecodeResult> results_;

        //! result queue mutex
        Core::Mutex result_mutex_;
    };
}
#endif