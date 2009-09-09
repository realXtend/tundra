// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_OpenJpegDecoder_h
#define incl_TextureDecoder_OpenJpegDecoder_h

#include "AssetInterface.h"
#include "TextureInterface.h"
#include "TextureRequest.h"

#include "ThreadTask.h"

namespace TextureDecoder
{
    //! OpenJpeg decoder that runs in a thread and serves decode requests, used internally by TextureService
    class OpenJpegDecoder : public Foundation::ThreadTask
    {
    public:
        //! Constructor
        OpenJpegDecoder();
        
        //! Work function
        virtual void Work();
        
    private:
        //! perform a decode & queue result
        /*! \param request decode request to serve
         */
        void PerformDecode(DecodeRequestPtr request);
    };
}
#endif