// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_Sound_h
#define incl_OpenALAudio_Sound_h

#include <al.h>
#include <alc.h>

namespace OpenALAudio
{
    //! A sound buffer containing sound data. Uses OpenAL
    class Sound
    {
    public:
        Sound();
        ~Sound();
        
        ALuint GetHandle() const { return handle_; }
        Core::uint GetDataSize() const { return datasize_; }
        
    private:
        //! OpenAL handle
        ALuint handle_;
        //! Total size of audio data
        Core::uint datasize_;    
    };
    
    typedef boost::shared_ptr<Sound> SoundPtr;        
}

#endif