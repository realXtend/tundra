// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenALAudioModule.h"
#include "SoundChannel.h"

namespace OpenALAudio
{
    SoundChannel::SoundChannel() :
        handle_(0),
        pitch_(1.0f),
        gain_(1.0f),
        position_(0.0f, 0.0f, 0.0f),    
        radius_(10.0f),
        rolloff_(1.0f),    
        positional_(false),
        looped_(false),
        state_(Foundation::SoundServiceInterface::Stopped)
    { 
    }
    
    SoundChannel::~SoundChannel()
    {       
        DeleteSource();
    }
    
    void SoundChannel::Update()
    {
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
            // If pending sound has now valid handle, start playback
            if (sound_->GetHandle())
                StartPlaying();
            break;                    
        }
    }
    
    void SoundChannel::Play(SoundPtr sound, bool positional)
    {        
        Stop();
     
        SetPositional(positional);
               
        sound_ = sound;        
        if (!sound_)
            return;
            
        if (sound_->GetHandle())
            StartPlaying();
        else
            // Sound asset not yet loaded, set pending state
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
        
        alSourcef(handle_, AL_GAIN, gain_);
        alSourcef(handle_, AL_PITCH, pitch_);
        alSourcei(handle_, AL_LOOPING, looped_ ? AL_TRUE : AL_FALSE);
        
        SetPositionAndMode();
                    
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
    
    void SoundChannel::SetPosition(const Core::Vector3df& position)
    {
        positional_ = true;
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
    
    void SoundChannel::SetPitch(Core::Real pitch)
    {
        pitch_ = pitch;
        if (handle_)
            alSourcef(handle_, AL_PITCH, pitch_);        
    }
    
    void SoundChannel::SetGain(Core::Real gain)
    {
        gain_ = gain;
        if (handle_)
            alSourcef(handle_, AL_GAIN, gain_);               
    }
    
    void SoundChannel::SetRange(Core::Real radius, Core::Real rolloff)
    {
        radius_ = radius;
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