// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSystem.h"
#include "OpenALAudioModule.h"
#include "WavLoader.h"
#include "VorbisDecoder.h"
#include "RexTypes.h"
#include "AssetServiceInterface.h"
#include "AssetEvents.h"

#include <boost/thread/mutex.hpp>

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
        update_time_(0),
        listener_position_(0.0, 0.0, 0.0)
    {
        sound_cache_size_ = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "sound_cache_size", DEFAULT_SOUND_CACHE_SIZE);
        
        Initialize();
        
        // Create vorbis decoder thread task and let the framework thread task manager handle it
        VorbisDecoder* decoder = new VorbisDecoder();
        framework_->GetThreadTaskManager()->AddThreadTask(Foundation::ThreadTaskPtr(decoder));        
    }

    SoundSystem::~SoundSystem()
    {
        Uninitialize();
    }

    void SoundSystem::Initialize()
    {
        if (initialized_)
            return;
            
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

        mutex.initialize();
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

    Foundation::SoundServiceInterface::SoundState SoundSystem::GetSoundState(Core::sound_id_t id) const
    {
        SoundChannelMap::const_iterator i = channels_.find(id);
        if (i == channels_.end())
            return Foundation::SoundServiceInterface::Stopped;
        return i->second->GetState();
    }
    
    std::vector<Core::sound_id_t> SoundSystem::GetActiveSounds() const
    {
        std::vector<Core::sound_id_t> ret;
        
        SoundChannelMap::const_iterator i = channels_.begin();
        while (i != channels_.end())        
        {
            if (i->second->GetState() != Foundation::SoundServiceInterface::Stopped)
                ret.push_back(i->first);
            ++i;
        }
        
        return ret;
    }
    
    const std::string& SoundSystem::GetSoundName(Core::sound_id_t id) const
    {
        static std::string empty;
        
        SoundChannelMap::const_iterator i = channels_.find(id);
        if (i == channels_.end())
            return empty;
        return i->second->GetSoundName();
    }    
    
    void SoundSystem::Update(Core::f64 frametime)
    {   
        mutex.lock();
        std::vector<SoundChannelMap::iterator> channels_to_delete;

        // Update listener position/orientation to sound device
        ALfloat pos[] = {listener_position_.x, listener_position_.y, listener_position_.z};
        alListenerfv(AL_POSITION, pos);
        Core::Vector3df front = listener_orientation_ * Core::Vector3df(0.0f, -1.0f, 0.0f);
        Core::Vector3df up = listener_orientation_ * Core::Vector3df(0.0f, 0.0f, -1.0f); 
        ALfloat orient[] = {front.x, front.y, front.z, up.x, up.y, up.z};
        alListenerfv(AL_ORIENTATION, orient);    
        
        // Update channel attenuations, check which have stopped
        SoundChannelMap::iterator i = channels_.begin();
        while (i != channels_.end())
        {
            i->second->Update(listener_position_);
            if (i->second->GetState() == Foundation::SoundServiceInterface::Stopped)
                channels_to_delete.push_back(i);
            ++i;
        }
        
        // Remove stopped channels
        for (Core::uint j = 0; j < channels_to_delete.size(); ++j)
            channels_.erase(channels_to_delete[j]);   
        
        mutex.unlock();
        
        // Age the sound cache
        UpdateCache(frametime);      
    }
    
    void SoundSystem::SetListener(const Core::Vector3df& position, const Core::Quaternion& orientation)
    {
        if (!initialized_)
            return;
     
        listener_position_ = position;
        listener_orientation_ = orientation;    
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
        i->second->Play(sound);
         
        return i->first;     
    }
    
    Core::sound_id_t SoundSystem::PlaySound3D(const std::string& name, bool local, Core::Vector3df position, Core::sound_id_t channel)
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
        
        i->second->SetPositional(true);        
        i->second->SetPosition(position);
        i->second->Play(sound);            
                
        return i->first;     
    }        

    Core::sound_id_t SoundSystem::PlayAudioData(Core::u8 * buffer, int buffer_size, int sample_rate, int sample_width, bool stereo, Core::sound_id_t channel)
    {
        if (!mutex.try_lock())
        {
            int i = (int)((buffer_size*8000 / sample_rate) / sample_width);
            std::string s_int = boost::lexical_cast<std::string>(i);
            OpenALAudioModule::LogDebug("Dropped " + s_int + " ms");
            return 0;
        }

        SoundPtr sound = SoundPtr(new OpenALAudio::Sound("audiobuffer"));
        if (!sound)
            return 0;

        bool sample_width_16bit = true;
        switch(sample_width)
        {
            case 8: sample_width_16bit = false; break;
            case 16: sample_width_16bit = true; break;
            default:
                return 0; // todo: Write log entry
        }
        sound->LoadFromBuffer(buffer, buffer_size, sample_rate, sample_width_16bit, stereo);

        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            SoundChannel* s = new SoundChannel();
            if (!s)
                return 0;
            i = channels_.insert(std::pair<Core::sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(s))).first;
        }
        
        if (!i->second.get())
            return 0;
        i->second->Play(sound);
        
        Core::sound_id_t this_channel_id =  i->first;
        mutex.unlock();
        return this_channel_id;     
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
    
    void SoundSystem::SetRange(Core::sound_id_t id, Core::Real inner_radius, Core::Real outer_radius, Core::Real rolloff)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;  
            
        i->second->SetRange(inner_radius, outer_radius, rolloff);
    }    
 
    Core::sound_id_t SoundSystem::GetNextSoundChannelID()
    {
        for (;;)
        {
            next_channel_id_++;
            if (!next_channel_id_)
                next_channel_id_ = 1;
            // Must be currently unused
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
        
        if (local)
        {        
            // Loading of local wav sound
            std::string name_lower = name;
            boost::algorithm::to_lower(name_lower);                
            if (name_lower.find(".wav") != std::string::npos)
            {
                SoundPtr new_sound(new Sound(name));
                if (WavLoader::LoadFromFile(new_sound.get(), name))
                {
                    sounds_[name] = new_sound;
                    return new_sound;
                }
            }        
            
            // Loading of local ogg sound
            if (name_lower.find(".ogg") != std::string::npos)
            {
                SoundPtr new_sound(new Sound(name));
                
                // See if the file exists. If it does, read it and post a decode request                       
                if (DecodeLocalOggFile(new_sound.get(), name))
                {
                    // Now the sound exists in cache with no data yet. We'll fill in later
                    sounds_[name] = new_sound;
                    return new_sound;
                }
            }   
        }
        else
        {
            // Loading of sound from assetdata, assumed to be vorbis compressed stream
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
            {
                SoundPtr new_sound(new Sound(name));
                sounds_[name] = new_sound;
                
                // The sound will be filled with data later
                asset_service->RequestAsset(name, RexTypes::ASSETTYPENAME_SOUNDVORBIS);                                             
                return new_sound;
            }
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
            // Don't erase zero size sounds, because they haven't been created yet and are probably waiting for assetdata
            if ((i->second->GetAge() >= oldest_age) && (i->second->GetSize()))
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
    
    bool SoundSystem::DecodeLocalOggFile(Sound* sound, const std::string& name)
    {
        boost::filesystem::path file_path(name);      
        std::ifstream file(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            OpenALAudioModule::LogError("Could not open file: " + name + ".");
            return false;
        }

        VorbisDecodeRequestPtr new_request(new VorbisDecodeRequest());
        new_request->name_ = name;
    
        std::filebuf *pbuf = file.rdbuf();
        size_t size = pbuf->pubseekoff(0, std::ios::end, std::ios::in);
        new_request->buffer_.resize(size);
        pbuf->pubseekpos(0, std::ios::in);
        pbuf->sgetn((char *)&new_request->buffer_[0], size);
        file.close();
                
        framework_->GetThreadTaskManager()->AddRequest("VorbisDecoder", new_request);
        return true;
    }
    
    bool SoundSystem::HandleTaskEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id != Task::Events::REQUEST_COMPLETED)
            return false;
        VorbisDecodeResult* result = dynamic_cast<VorbisDecodeResult*>(data);
        if (!result || result->task_description_ != "VorbisDecoder")
            return false;

        // If we can find the sound from our cache, and the result contains data, stuff the data into the sound
        SoundMap::iterator i = sounds_.find(result->name_);
        if (i == sounds_.end())
            return false;
                   
        if (!result->buffer_.size())
            return true;
                    
        i->second->LoadFromBuffer(&result->buffer_[0], result->buffer_.size(), result->frequency_, true, result->stereo_); 
        return true;
    }
            
    bool SoundSystem::HandleAssetEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id != Asset::Events::ASSET_READY)
            return false;
        
        Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data);
        if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_SOUNDVORBIS)
        {
            if (!event_data->asset_)
                return false;
            // Find the sound from our cache
            SoundMap::iterator i = sounds_.find(event_data->asset_id_);
            if (i == sounds_.end())
                return false;   
            // If sound already has data, do not queue another decode request
            if (i->second->GetSize() != 0)
                return false;
            VorbisDecodeRequestPtr new_request(new VorbisDecodeRequest());
            new_request->name_ = event_data->asset_id_;                
            new_request->buffer_.resize(event_data->asset_->GetSize());
            //! \todo use asset data directly instead of copying to decode request buffer
            memcpy(&new_request->buffer_[0], event_data->asset_->GetData(), event_data->asset_->GetSize());
            framework_->GetThreadTaskManager()->AddRequest("VorbisDecoder", new_request);  
        }
        
        return false;
    }
}
