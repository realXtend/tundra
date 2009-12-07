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
        //! Constructor
        Sound();
        //! Destructor
        ~Sound();
        
        //! Reset age of sound (for caching)
        void ResetAge();
        //! Add age to sound (for caching)
        void AddAge(Core::f64 time);
                
        //! Return OpenAL handle
        ALuint GetHandle() const { return handle_; }
        //! Return datasize of sound in bytes
        Core::uint GetSize() const { return size_; }
        //! Return age of sound (for caching)
        Core::f64 GetAge() const { return age_; }
        
    private:
        //! OpenAL handle
        ALuint handle_;
        //! Total size of audio data
        Core::uint size_;    
        //! Age of sound (resetted when last accessed)
        Core::f64 age_;
    };
    
    typedef boost::shared_ptr<Sound> SoundPtr;        
}

#endif