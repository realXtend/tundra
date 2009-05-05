// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_OpenJpegDecoder_h
#define incl_TextureDecoder_OpenJpegDecoder_h

#include "AssetInterface.h"
#include "TextureInterface.h"
#include "TextureRequest.h"

namespace TextureDecoder
{
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

        //! stops the decode thread
        void Stop() { running_ = false; }

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

        //! whether should keep running
        volatile bool running_;
    };
}
#endif