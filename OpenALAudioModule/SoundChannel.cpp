// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenALAudioModule.h"
#include "SoundChannel.h"

namespace OpenALAudio
{
    static const Real MINIMUM_ROLLOFF = 0.1f;
    static const Real DEFAULT_ROLLOFF = 2.0f;
    static const Real DEFAULT_INNER_RADIUS = 1.0f;
    static const Real DEFAULT_OUTER_RADIUS = 50.0f;
    
    SoundChannel::SoundChannel(Foundation::SoundServiceInterface::SoundType type) :
        type_(type),
        handle_(0),
        pitch_(1.0f),
        gain_(1.0f),
        master_gain_(1.0f),
        position_(0.0, 0.0, 0.0), 
        inner_radius_(DEFAULT_INNER_RADIUS),
        outer_radius_(DEFAULT_OUTER_RADIUS),
        rolloff_(DEFAULT_ROLLOFF),
        attenuation_(1.0f),
        positional_(false),
        looped_(false),
        state_(Foundation::SoundServiceInterface::Stopped)
    { 
    }
    
    SoundChannel::~SoundChannel()
    {       
        DeleteSource();
    }
    
    void SoundChannel::Update(const Vector3df& listener_pos)
    {   
        CalculateAttenuation(listener_pos);         
        SetAttenuatedGain();
        
        switch (state_)
        {
            case Foundation::SoundServiceInterface::Playing:
            if (handle_)
            {                
                ALint playing;
                alGetSourcei(handle_, AL_SOURCE_STATE, &playing);
                if (playing != AL_PLAYING)
                {
                    state_ = Foundation::SoundServiceInterface::Stopped;
                    sound_.reset();
                }
            }
            break;
            
            case Foundation::SoundServiceInterface::Loading:
            // If pending sound has valid handle, start playback now
            if (sound_->GetHandle())
                StartPlaying();
            break;                    
        }
    }
    
    void SoundChannel::Play(SoundPtr sound)
    {   
        Stop();
        
        sound_ = sound;        
        if (!sound_)
            return;
            
        // Start actual playback on next update
        state_ = Foundation::SoundServiceInterface::Loading;
    }
        
    
    bool SoundChannel::CreateSource()
    {
        if (!handle_)
            alGenSources(1, &handle_);
        
        if (!handle_)
        {
            OpenALAudioModule::LogError("Could not create OpenAL sound source");
            return false;
        }   
        
        alSourcef(handle_, AL_PITCH, pitch_);
        alSourcei(handle_, AL_LOOPING, looped_ ? AL_TRUE : AL_FALSE);
        // No matter whether sound is positional or not, we use own attenuation, so OpenAL rolloff is 0
        alSourcef(handle_, AL_ROLLOFF_FACTOR, 0.0);       
        
        SetPositionAndMode();
        SetAttenuatedGain();   
                    
        return true;
    }
    
    void SoundChannel::DeleteSource()
    {
        if (handle_)
        {
            alDeleteSources(1, &handle_);
            handle_ = 0;
        }    
    }
    
    void SoundChannel::Stop()
    {
        if (handle_)
            alSourceStop(handle_);

        state_ = Foundation::SoundServiceInterface::Stopped;
        sound_.reset();
    }
    
    const std::string& SoundChannel::GetSoundName() const
    {   
        static std::string empty;
        
        if (!sound_)
            return empty;
        else
            return sound_->GetName();
    }
    
    void SoundChannel::SetPosition(const Vector3df& position)
    {
        position_ = position;
        
        SetPositionAndMode();
    }
    
    void SoundChannel::SetPositional(bool enable)
    {
        positional_ = enable;
       
        SetPositionAndMode();        
    }
    
    void SoundChannel::SetLooped(bool enable)
    {
        looped_ = enable;
        if (handle_)
            alSourcei(handle_, AL_LOOPING, looped_ ? AL_TRUE : AL_FALSE);
    }
    
    void SoundChannel::SetPitch(Real pitch)
    {
        pitch_ = pitch;
        if (handle_)
            alSourcef(handle_, AL_PITCH, pitch_);        
    }
    
    void SoundChannel::SetGain(Real gain)
    {
        if (gain < 0.0f) 
            gain = 0.0f;
        if (gain > 1.0f)
            gain = 1.0f;
            
        gain_ = gain;          
    }
    
    void SoundChannel::SetMasterGain(Real master_gain)
    {
        if (master_gain < 0.0f) 
            master_gain = 0.0f;
        if (master_gain > 1.0f)
            master_gain = 1.0f;
            
        master_gain_ = master_gain;          
    }    
    
    void SoundChannel::SetRange(Real inner_radius, Real outer_radius, Real rolloff)
    {
        if (rolloff < MINIMUM_ROLLOFF) 
            rolloff = MINIMUM_ROLLOFF;
        if (inner_radius < 0.0f)
            inner_radius = 0.0f;
        if (outer_radius < 0.0f)
            outer_radius = 0.0f;

        inner_radius_ = inner_radius;
        outer_radius_ = outer_radius;
        rolloff_ = rolloff;
    }
    
    void SoundChannel::SetPositionAndMode()
    {
        if (handle_)
        {
            if (positional_)
            {
                alSourcei(handle_, AL_SOURCE_RELATIVE, AL_FALSE);
                ALfloat sound_pos[] = {position_.x, position_.y, position_.z};
                alSourcefv(handle_, AL_POSITION, sound_pos);

            }
            else
            {
                alSourcei(handle_, AL_SOURCE_RELATIVE, AL_TRUE);
                ALfloat sound_pos[] = {0.0, 0.0, 0.0};
                alSourcefv(handle_, AL_POSITION, sound_pos);
            }
        }
    }
    
    void SoundChannel::CalculateAttenuation(const Vector3df& listener_pos)
    {
        if ((outer_radius_ == 0.0f) || (outer_radius_ <= inner_radius_))
        {
            attenuation_ = 1.0f;
            return;
        }
          
        Real distance = (position_ - listener_pos).getLength();
        if (distance <= inner_radius_)
        {
            attenuation_ = 1.0f;
            return;
        }
        if (distance >= outer_radius_)
        {
            attenuation_ = 0.0f;
            return;
        }
                            
        attenuation_ = pow(1.0f - (distance - inner_radius_) / (outer_radius_ - inner_radius_), rolloff_);
    }  
    
    void SoundChannel::SetAttenuatedGain()
    {
        if (handle_)
        {            
            if (positional_)
                alSourcef(handle_, AL_GAIN, master_gain_ * gain_ * attenuation_);          
            else  
                alSourcef(handle_, AL_GAIN, master_gain_ * gain_);     
        }
    }
    
    void SoundChannel::StartPlaying()
    {
        // Create source now if did not exist already
        if (!CreateSource())
        {
            state_ = Foundation::SoundServiceInterface::Stopped;
            sound_.reset();
            return;
        }
        
        alSourcei(handle_, AL_BUFFER, sound_->GetHandle());
        alSourcePlay(handle_);
        state_ = Foundation::SoundServiceInterface::Playing; 
    }      
}