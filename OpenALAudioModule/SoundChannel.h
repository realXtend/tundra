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
        SoundChannel(Foundation::SoundServiceInterface::SoundType type);
        //! Destructor.
        ~SoundChannel();
        
        //! Start playing sound. Set to pending state if sound is actually not loaded yet
        void Play(SoundPtr sound);       
        //! Set positional state
        void SetPositional(bool enable);
        //! Set looped state
        void SetLooped(bool enable);
        //! Set position
        void SetPosition(const Vector3df& pos);        
        //! Set pitch.
        void SetPitch(Real pitch);
        //! Set gain.
        void SetGain(Real gain);
        //! Set master gain.
        void SetMasterGain(Real master_gain);        
        //! Set range parameters
        void SetRange(Real inner_radius, Real outer_radius, Real rolloff);        
        //! Stop.
        void Stop();        
        //! Per-frame update with new listener position
        void Update(const Vector3df& listener_pos);
        //! Return current state of channel.
        Foundation::SoundServiceInterface::SoundState GetState() const { return state_; }
        //! Return name/id of sound that's playing, empty if nothing playing
        const std::string& GetSoundName() const;
        //! Return sound type
        Foundation::SoundServiceInterface::SoundType GetSoundType() const { return type_; }
        
    private:
        //! Create OpenAL source if one does not exist yet
        bool CreateSource();      
        //! Delete OpenAL source
        void DeleteSource();       
        //! Calculate attenuation from position, listener position & range parameters
        void CalculateAttenuation(const Vector3df& listener_pos);
        //! Set positionality & position
        void SetPositionAndMode();
        //! Set gain, taking attenuation into account
        void SetAttenuatedGain();
        //! Start playback
        void StartPlaying();
    
        //! Sound type
        Foundation::SoundServiceInterface::SoundType type_;
        //! OpenAL handle
        ALuint handle_;
        //! Current sound being played
        SoundPtr sound_;
        //! Pitch
        Real pitch_;
        //! Gain
        Real gain_;
        //! Master gain. Final sound volume = gain * master gain * possible distance attenuation
        Real master_gain_;
        //! Inner radius
        Real inner_radius_;
        //! Outer radius
        Real outer_radius_;
        //! Rolloff power factor
        Real rolloff_;
        //! Last calculated attenuation factor
        Real attenuation_;
        //! Looped flag
        bool looped_;
        //! Positional flag
        bool positional_;
        //! Position
        Vector3df position_;
        //! State 
        Foundation::SoundServiceInterface::SoundState state_;
    };
    
    typedef boost::shared_ptr<SoundChannel> SoundChannelPtr;
}

#endif
