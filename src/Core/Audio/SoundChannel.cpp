// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include "SoundChannel.h"
#include "LoggingFunctions.h"

#ifndef Q_WS_MAC
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

#include "MemoryLeakCheck.h"

static const float MINIMUM_ROLLOFF = 0.1f;
static const float DEFAULT_ROLLOFF = 2.0f;
static const float DEFAULT_INNER_RADIUS = 1.0f;
static const float DEFAULT_OUTER_RADIUS = 50.0f;

SoundChannel::SoundChannel(sound_id_t channelId_, SoundType type) :
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
    buffered_mode_(false),
    state_(Stopped),
    channelId(channelId_)
{ 
}

SoundChannel::~SoundChannel()
{
    DeleteSource();
}

void SoundChannel::Update(const float3& listener_pos)
{   
    CalculateAttenuation(listener_pos);
    SetAttenuatedGain();
    QueueBuffers();
    UnqueueBuffers();
    
    if (state_ == Playing)
    {
        if (handle_)
        {
            ALint playing;
            alGetSourcei(handle_, AL_SOURCE_STATE, &playing);
            if (playing != AL_PLAYING)
            {
                // Stopped state may trigger removal of audio channel, so don't
                // do that in buffered mode
                if (buffered_mode_)
                {
                    state_ = Pending;
                }
                else
                    state_ = Stopped;
            }
        }
    }
}

void SoundChannel::Play(AudioAssetPtr audioAsset)
{
    // Stop any previously buffered sound
    Stop();
    
    if (!audioAsset)
        return;
    
    pending_sounds_.push_back(audioAsset);
    
    // Start actual playback on next update
    state_ = Pending;
    buffered_mode_ = false;
}

void SoundChannel::AddBuffer(AudioAssetPtr buffer)
{
    pending_sounds_.push_back(buffer);
    
    // Buffered mode should not loop
    SetLooped(false);
    
    // Start actual playback on next update
    if (state_ == Stopped)
        state_ = Pending;
    buffered_mode_ = true;
}

bool SoundChannel::CreateSource()
{
    if (!handle_)
        alGenSources(1, &handle_);
    
    if (!handle_)
    {
        LogError("Could not create OpenAL sound source");
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
    Stop();
    
    if (handle_)
    {
        alDeleteSources(1, &handle_);
        handle_ = 0;
    }
}

void SoundChannel::Stop()
{
    if (handle_)
    {
        alSourceStop(handle_);
        // Set null buffer to be sure we cleared the buffer queue
        alSourcei(handle_, AL_BUFFER, 0);
    }
    
    pending_sounds_.clear();
    playing_sounds_.clear();
    
    state_ = Stopped;
}

QString SoundChannel::GetSoundName() const
{   
    AudioAssetPtr asset = playing_sounds_.size() > 0 ? playing_sounds_.front() : AudioAssetPtr();
    if (asset)
        return asset->Name();
    asset = pending_sounds_.size() > 0 ? pending_sounds_.front() : AudioAssetPtr();
    if (asset)
        return asset->Name();
    
    return "";
}

void SoundChannel::SetPosition(const float3 &position)
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
    // Can not set looping in buffered mode
    if (buffered_mode_)
        enable = false;
    
    looped_ = enable;
    if (handle_)
        alSourcei(handle_, AL_LOOPING, looped_ ? AL_TRUE : AL_FALSE);
}

void SoundChannel::SetPitch(float pitch)
{
    pitch_ = pitch;
    if (handle_)
        alSourcef(handle_, AL_PITCH, pitch_);
}

void SoundChannel::SetGain(float gain)
{
    if (gain < 0.0f) 
        gain = 0.0f;
    if (gain > 1.0f)
        gain = 1.0f;
        
    gain_ = gain;
}

void SoundChannel::SetMasterGain(float master_gain)
{
    if (master_gain < 0.0f) 
        master_gain = 0.0f;
    if (master_gain > 1.0f)
        master_gain = 1.0f;
        
    master_gain_ = master_gain;
}    

void SoundChannel::SetRange(float inner_radius, float outer_radius, float rolloff)
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

void SoundChannel::CalculateAttenuation(const float3& listener_pos)
{
    if ((outer_radius_ == 0.0f) || (outer_radius_ <= inner_radius_))
    {
        attenuation_ = 1.0f;
        return;
    }
      
    float distance = (position_ - listener_pos).Length();
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

void SoundChannel::QueueBuffers()
{
    // See that we do have waiting sounds and they're ready to play
    AudioAssetPtr pending = pending_sounds_.size() > 0 ? pending_sounds_.front() : AudioAssetPtr();

    if (!pending)
        return;
    
    // Create source now if did not exist already
    if (!CreateSource())
    {
        state_ = Stopped;
        pending_sounds_.clear();
        return;
    }
    
    bool queued = false;
    
    // Buffer pending sounds, move them to playing vector
    while(pending_sounds_.size() > 0)
    {
        AudioAssetPtr sound = pending_sounds_.front();
        if (!sound)
        {
            pending_sounds_.pop_front();
            continue;
        }
        ALuint buffer = sound->GetHandle();
        // If no valid handle yet, cannot play this one, break out
        if (!buffer)
            return;
        
        alGetError();
        alSourceQueueBuffers(handle_, 1, &buffer);
        ALenum error = alGetError();
        
        if (error != AL_NONE)
        {
            // If queuing fails, we may have changed sound format. Stop, flush queue & retry
            alSourceStop(handle_);
            alSourcei(handle_, AL_BUFFER, 0);
            alSourceQueueBuffers(handle_, 1, &buffer);
            ALenum error = alGetError();
            if (error != AL_NONE)
                LogError("Could not queue OpenAL sound buffer: " + QString::number(error));
            else
            {
                playing_sounds_.push_back(sound);
                queued = true;
            }
        }
        else
        {
            playing_sounds_.push_back(sound);
            queued = true;
        }
        
        pending_sounds_.pop_front();
    }
    
    // If at least one sound queued, start playback if not already playing
    if (queued)
    {
        ALint playing;
        alGetSourcei(handle_, AL_SOURCE_STATE, &playing);
        if (playing != AL_PLAYING)
            alSourcePlay(handle_);
        state_ = Playing;
    }
}

void SoundChannel::UnqueueBuffers()
{
    if (handle_)
    {
        int processed = 0;
        alGetSourcei(handle_, AL_BUFFERS_PROCESSED, &processed);
        while(processed--)
        {
            ALuint buffer = 0;
            alSourceUnqueueBuffers(handle_, 1, &buffer);
            if (buffer)
            {
                // See if we find matching buffer from the sounds vector.
                // If found, erase so that the sound may be freed if not used elsewhere
                for(uint i = 0; i < playing_sounds_.size(); ++i)
                {
                    if (playing_sounds_[i]->GetHandle() == buffer)
                    {
                        playing_sounds_.erase(playing_sounds_.begin() + i);
                        break;
                    }
                }
            }
        }
    }
}
