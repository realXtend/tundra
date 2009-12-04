// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSystem.h"
#include "OpenALAudioModule.h"

namespace OpenALAudio
{
    SoundSystem::SoundSystem(Foundation::Framework *framework) : initialized_(false), context_(0), device_(0)
    {
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
            return;
            
        context_ = alcCreateContext(device_, NULL);
        if (!context_)
            return;
            
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
    
    void SoundSystem::Update()
    {
        SoundChannelMap::iterator i = channels_.begin();
        while (i != channels_.end())
        {
            i->second->Update();
            ++i;
        }
    }
    
    void SoundSystem::SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation)
    {
        if (!initialized_)
            return;
            
        ALfloat listener_pos[] = {position.x, position.y, position.z};
        alListenerfv(AL_POSITION, listener_pos);

        // Todo: set orientation    
    }
    
         
    Core::sound_id_t SoundSystem::PlaySound(const std::string& name, bool local, Core::sound_id_t channel)
    {
        return 0;
    }
    
    Core::sound_id_t SoundSystem::PlaySound3D(const std::string& name, bool local, Core::Vector3df& position, Core::sound_id_t channel)
    {
        return 0;
    }        

    void SoundSystem::StopSound(Core::sound_id_t id)
    {
    }
    
    void SoundSystem::SetPitch(Core::sound_id_t id, Core::Real pitch)
    {
    }
    
    void SoundSystem::SetGain(Core::sound_id_t id, Core::Real gain)
    {
    }
    
    void SoundSystem::SetLooped(Core::sound_id_t id, bool looped)
    {
    }
    
    void SoundSystem::SetPositional(Core::sound_id_t id, bool positional)
    {
    }
    
    void SoundSystem::SetPosition(Core::sound_id_t id, Core::Vector3df position)
    {
    }
    
}
