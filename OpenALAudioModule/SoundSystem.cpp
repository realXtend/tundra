// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSystem.h"
#include "OpenALAudioModule.h"

namespace OpenALAudio
{
    const Core::uint DEFAULT_SOUND_CACHE_SIZE = 32 * 1024 * 1024;
    const Core::f64 CACHE_CHECK_INTERVAL = 1.0;
    
    SoundSystem::SoundSystem(Foundation::Framework *framework) : 
        framework_(framework),
        initialized_(false), 
        context_(0), 
        device_(0), 
        next_channel_id_(0),
        sound_cache_size_(DEFAULT_SOUND_CACHE_SIZE),
        update_time_(0)
    {
        sound_cache_size_ = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "sound_cache_size", DEFAULT_SOUND_CACHE_SIZE);
        
        Initialize();
    }

    SoundSystem::~SoundSystem()
    {
        Uninitialize();
    }

    void SoundSystem::Initialize()
    {
        if (initialized_)
            return;
            
        //Initial listener properties
        ALfloat listener_pos[] = {0.0,0.0,0.0};
        ALfloat listener_vel[] = {0.0,0.0,0.0};
        ALfloat listener_orient[] = {0.0,0.0,-1.0, 0.0,1.0,0.0};

        device_ = alcOpenDevice(NULL); 
        if (!device_)
        {
            OpenALAudioModule::LogWarning("Could not open OpenAL sound device");        
            return;
        } 
          
        context_ = alcCreateContext(device_, NULL);
        if (!context_)
        {
            OpenALAudioModule::LogWarning("Could not create OpenAL sound context");   
            return;
        }
           
        alcMakeContextCurrent(context_);
        initialized_ = true;

        //Set initial listener position, velocity and orientation
        alListenerfv(AL_POSITION, listener_pos);
        alListenerfv(AL_VELOCITY, listener_vel);
        alListenerfv(AL_ORIENTATION, listener_orient);
    }

    void SoundSystem::Uninitialize()
    {    
        if (context_)
        {
            alcMakeContextCurrent(0);
            alcDestroyContext(context_);
            context_ = 0;
        }
        if (device_)
        {
            alcCloseDevice(device_);
            device_ = 0;
        }

        initialized_ = false;
    }

    Foundation::SoundServiceInterface::SoundState SoundSystem::GetSoundState(Core::sound_id_t id)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return Foundation::SoundServiceInterface::Stopped;
        return i->second->GetState();
    }
    
    void SoundSystem::Update(Core::f64 frametime)
    {        
        std::vector<SoundChannelMap::iterator> channels_to_delete;
        
        SoundChannelMap::iterator i = channels_.begin();
        while (i != channels_.end())
        {
            i->second->Update();
            if (i->second->GetState() == Foundation::SoundServiceInterface::Stopped)
                channels_to_delete.push_back(i);
            ++i;
        }
        
        // Remove stopped channels
        for (Core::uint j = 0; j < channels_to_delete.size(); ++j)
            channels_.erase(channels_to_delete[j]);   
           
        // Age the sound cache
        UpdateCache(frametime);        
    }
    
    void SoundSystem::SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation)
    {
        if (!initialized_)
            return;
            
        ALfloat listener_pos[] = {position.x, position.y, position.z};
        alListenerfv(AL_POSITION, listener_pos);

        // OpenSim coordinate axes: identity orientation is negative X (front), positive Z (up)
        Core::Vector3df front = orientation * Core::Vector3df(-1.0f, 0.0f, 0.0f);
        Core::Vector3df up = orientation * Core::Vector3df(0.0f, 0.0f, 1.0f); 
        ALfloat listener_orient[] = {front.x, front.y, front.z, up.x, up.y, up.z};
        alListenerfv(AL_ORIENTATION, listener_orient);        
    }
      
    Core::sound_id_t SoundSystem::PlaySound(const std::string& name, bool local, Core::sound_id_t channel)
    {
        SoundPtr sound = GetSound(name, local);
        if (!sound)
            return 0;

        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<Core::sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel()))).first;
        }
        
        i->second->SetPositional(false);
        i->second->Play(sound, false);
         
        return i->first;     
    }
    
    Core::sound_id_t SoundSystem::PlaySound3D(const std::string& name, bool local, Core::Vector3df& position, Core::sound_id_t channel)
    {
        SoundPtr sound = GetSound(name, local);
        if (!sound)
            return 0;
                
        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<Core::sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel()))).first;
        }
        
        i->second->SetPosition(position);
        i->second->Play(sound, false);            
                
        return i->first;     
    }        

    void SoundSystem::StopSound(Core::sound_id_t id)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;    
            
        i->second->Stop();
    }
    
    void SoundSystem::SetPitch(Core::sound_id_t id, Core::Real pitch)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;        
            
        i->second->SetPitch(pitch);  
    }
    
    void SoundSystem::SetGain(Core::sound_id_t id, Core::Real gain)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
            
        i->second->SetGain(gain);
    }
    
    void SoundSystem::SetLooped(Core::sound_id_t id, bool looped)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
            
        i->second->SetLooped(looped);
    }
    
    void SoundSystem::SetPositional(Core::sound_id_t id, bool positional)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
         
        i->second->SetPositional(positional);
    }
    
    void SoundSystem::SetPosition(Core::sound_id_t id, Core::Vector3df position)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
            
        i->second->SetPosition(position);
    }
    
    void SoundSystem::SetRange(Core::sound_id_t id, Core::Real radius, Core::Real rolloff)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
            
        i->second->SetRange(radius, rolloff);
    }    
 
    Core::sound_id_t SoundSystem::GetNextSoundChannelID()
    {
        for (;;)
        {
            next_channel_id_++;
            if (!next_channel_id_)
                next_channel_id_ = 1;
            // Must be previously unused
            if (channels_.find(next_channel_id_) == channels_.end())
                break;
        }
        
        return next_channel_id_;
    }   
    
    SoundPtr SoundSystem::GetSound(const std::string& name, bool local)
    {
        SoundMap::iterator i = sounds_.find(name);
        if (i != sounds_.end())
        {
            i->second->ResetAge();
            return i->second;
        }
        
        return SoundPtr();
    }
    
    void SoundSystem::UpdateCache(Core::f64 frametime)
    {
        update_time_ += frametime;
        if (update_time_ < CACHE_CHECK_INTERVAL)
            return;
            
        Core::uint total_size = 0;
        SoundMap::iterator oldest_sound = sounds_.end();
        Core::f64 oldest_age = 0.0;
        
        SoundMap::iterator i = sounds_.begin();
        while (i != sounds_.end())
        {
            i->second->AddAge(update_time_);   
            total_size += i->second->GetSize();
            if (i->second->GetAge() >= oldest_age)
            {
                oldest_age = i->second->GetAge();
                oldest_sound = i;
            }
            ++i;
        }

        if (total_size > sound_cache_size_)
        {
            if (oldest_sound != sounds_.end())
                sounds_.erase(oldest_sound);
        }
        
        update_time_ = 0.0;
    }
}
