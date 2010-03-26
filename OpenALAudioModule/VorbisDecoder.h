// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_VorbisDecoder_h
#define incl_OpenALAudio_VorbisDecoder_h

#include "ThreadTask.h"

namespace OpenALAudio
{
    //! Ogg vorbis decode request
    class VorbisDecodeRequest : public Foundation::ThreadTaskRequest
    {
    public:
        //! Name/id of sound
        std::string name_;
        //! Vorbis datastream
        std::vector<u8> buffer_;
    };
    
    class VorbisDecodeResult : public Foundation::ThreadTaskResult
    {
    public:
        //! Name/id of sound
        std::string name_;    
        //! Decoded audio data buffer. Will always be 16bit signed
        /*! If decode failed, will be zero size
         */         
        std::vector<u8> buffer_;
        //! Frequency
        uint frequency_;
        //! Stereo flag
        bool stereo_;        
    };
    
    typedef boost::shared_ptr<VorbisDecodeRequest> VorbisDecodeRequestPtr;
    typedef boost::shared_ptr<VorbisDecodeResult> VorbisDecodeResultPtr;

    //! Ogg Vorbis decoder that runs in a thread and serves decode requests, used by SoundSystem
    class VorbisDecoder : public Foundation::ThreadTask
    {
    public:
        //! Constructor
        VorbisDecoder();
        
        //! Work function
        virtual void Work();
        
    private:
        //! perform a decode & queue result
        /*! \param request decode request to serve
         */
        void PerformDecode(VorbisDecodeRequestPtr request);
        
        uint decodes_per_frame_;
    };
}
#endif