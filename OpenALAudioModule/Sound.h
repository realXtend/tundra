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
        
        //! Load WAV data from file
        bool LoadWavFromFile(const std::string& filename);
        //! Load WAV data from data buffer
        bool LoadWavFromBuffer(Core::u8* data, Core::uint size);
                        
        //! Return OpenAL handle
        ALuint GetHandle() const { ResetAge(); return handle_; }
        //! Return datasize of sound in bytes
        Core::uint GetSize() const { ResetAge(); return size_; }

        //! Return age of sound (for caching)
        Core::f64 GetAge() const { return age_; }
        //! Reset age of sound (for caching)
        void ResetAge() const { age_ = 0.0; }
        //! Add age to sound (for caching)
        void AddAge(Core::f64 time) const { age_ += time; }        
        
    private:
        //! Create sound buffer if one does not exist
        bool CreateBuffer();        
        //! Delete sound buffer
        void DeleteBuffer();
        
        //! OpenAL handle
        ALuint handle_;
        //! Total size of audio data
        Core::uint size_;    
        //! Age of sound (resetted when last accessed)
        mutable Core::f64 age_;
    };
    
    typedef boost::shared_ptr<Sound> SoundPtr;        
}

#endif