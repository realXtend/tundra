// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_OpenALAudio_SoundChannel_h
#define incl_OpenALAudio_SoundChannel_h

#include "SoundServiceInterface.h"
#include "Sound.h"

namespace OpenALAudio
{
    //! An OpenAL sound channel (source)
    class SoundChannel
    {
    public:
        //! Constructor.
        SoundChannel();
        //! Destructor.
        ~SoundChannel();
        
        //! Start playing sound. Set to pending state if sound is actually not loaded yet
        void Play(SoundPtr sound);       
        //! Set positional state
        void SetPositional(bool enable);
        //! Set looped state
        void SetLooped(bool enable);
        //! Set position
        void SetPosition(const Core::Vector3df& pos);        
        //! Set pitch.
        void SetPitch(Core::Real pitch);
        //! Set gain.
        void SetGain(Core::Real gain);
        //! Set range parameters
        void SetRange(Core::Real inner_radius, Core::Real outer_radius, Core::Real rolloff);        
        //! Stop.
        void Stop();        
        //! Per-frame update with new listener position
        void Update(const Core::Vector3df& listener_pos);
        //! Return current state of channel.
        Foundation::SoundServiceInterface::SoundState GetState() const { return state_; }
        //! Return name/id of sound that's playing, empty if nothing playing
        const std::string& GetSoundName() const;
        
    private:
        //! Create OpenAL source if one does not exist yet
        bool CreateSource();      
        //! Delete OpenAL source
        void DeleteSource();       
        //! Calculate attenuation from position, listener position & range parameters
        void CalculateAttenuation(const Core::Vector3df& listener_pos);
        //! Set positionality & position
        void SetPositionAndMode();
        //! Set gain, taking attenuation into account
        void SetAttenuatedGain();
        //! Start playback
        void StartPlaying();
    
        //! OpenAL handle
        ALuint handle_;
        //! Current sound being played
        SoundPtr sound_;
        //! Pitch
        Core::Real pitch_;
        //! Gain
        Core::Real gain_;
        //! Inner radius
        Core::Real inner_radius_;
        //! Outer radius
        Core::Real outer_radius_;
        //! Rolloff power factor
        Core::Real rolloff_;
        //! Last calculated attenuation factor
        Core::Real attenuation_;
        //! Looped flag
        bool looped_;
        //! Positional flag
        bool positional_;
        //! Position
        Core::Vector3df position_;
        //! State 
        Foundation::SoundServiceInterface::SoundState state_;
    };
    
    typedef boost::shared_ptr<SoundChannel> SoundChannelPtr;
}

#endif
