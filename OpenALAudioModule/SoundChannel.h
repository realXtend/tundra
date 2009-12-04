// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_SoundChannel_h
#define incl_OpenALAudio_SoundChannel_h

#include <al.h>
#include <alc.h>

#include "SoundServiceInterface.h"
#include "Sound.h"

namespace OpenALAudio
{
    //! An OpenAL sound channel (source)
    class SoundChannel
    {
    public:
        SoundChannel();
        ~SoundChannel();
        
        void Play(SoundPtr sound, bool positional);
        void SetPositional(bool enable);
        void SetLooped(bool enable);
        void SetPosition(const Core::Vector3df& pos);
        void SetPitch(Core::Real pitch);
        void SetGain(Core::Real gain);
        void Stop();
        void Update();
        
        Foundation::SoundServiceInterface::SoundState GetState() const { return state_; }

    private:
        //! Create OpenAL source if one does not exist yet
        bool CreateSource();
        //! Delete OpenAL source
        void DeleteSource();
    
        //! OpenAL handle
        ALuint handle_;
        //! Current sound being played
        SoundPtr sound_;
        //! Pitch
        Core::Real pitch_;
        //! Gain
        Core::Real gain_;
        //! Looped
        bool looped_;
        //! Positional
        bool positional_;
        //! Position
        Core::Vector3df position_;
        //! State 
        Foundation::SoundServiceInterface::SoundState state_;
    };
    
    typedef boost::shared_ptr<SoundChannel> SoundChannelPtr;
}

#endif