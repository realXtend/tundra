// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_Sound_h
#define incl_OpenALAudio_Sound_h

#include <AL/al.h>
#include <AL/alc.h>

#include "SoundServiceInterface.h"

namespace OpenALAudio
{
    //! A sound buffer containing sound data. Uses OpenAL
    class Sound
    {
    public:
        //! Constructor
        Sound(const std::string& name);
        //! Destructor
        ~Sound();
        
        //! Load raw data from buffer
        /*! Any existing sound data will be erased.
         */
        bool LoadFromBuffer(const Foundation::SoundServiceInterface::SoundBuffer& buffer);

        //! Return sound name
        const std::string& GetName() const { return name_; }
        //! Return OpenAL handle
        ALuint GetHandle() const { ResetAge(); return handle_; }
        //! Return datasize of sound in bytes
        uint GetSize() const { return size_; }

        //! Return age of sound (for caching)
        f64 GetAge() const { return age_; }
        //! Reset age of sound (for caching)
        void ResetAge() const { age_ = 0.0; }
        //! Add age to sound (for caching)
        void AddAge(f64 time) const { age_ += time; }
        
    private:
        //! Create sound buffer if one does not exist
        bool CreateBuffer();
        //! Delete sound buffer
        void DeleteBuffer();
        
        //! Name or id
        std::string name_;
        //! OpenAL handle
        ALuint handle_;
        //! Total size of audio data
        uint size_;    
        //! Age of sound (resetted when last accessed)
        mutable f64 age_;
    };
    
    typedef boost::shared_ptr<Sound> SoundPtr;        
}

#endif
