// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SoundSystem.h"
#include "OpenALAudioModule.h"
#include "WavLoader.h"
#include "VorbisDecoder.h"
#include "RexTypes.h"
#include "AssetServiceInterface.h"
#include "AssetEvents.h"
#include "Framework.h"
#include "ConfigurationManager.h"
#include "ThreadTaskManager.h"
#include "ServiceManager.h"
#include "EventManager.h"

#include <boost/thread/mutex.hpp>

namespace OpenALAudio
{
    const uint DEFAULT_SOUND_CACHE_SIZE = 32 * 1024 * 1024;
    const f64 CACHE_CHECK_INTERVAL = 1.0;
    
    SoundSystem::SoundSystem(Foundation::Framework *framework) : 
        framework_(framework),
        initialized_(false),
        master_gain_(1.0f),
        context_(0),
        device_(0),
        capture_device_(0),
        capture_sample_size_(0),
        next_channel_id_(0),
        sound_cache_size_(DEFAULT_SOUND_CACHE_SIZE),
        update_time_(0),
        listener_position_(0.0, 0.0, 0.0)
    {
        sound_cache_size_ = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "sound_cache_size", DEFAULT_SOUND_CACHE_SIZE);
        
        // By default, initialize default playback device
        Initialize();
        
        // Create vorbis decoder thread task and let the framework thread task manager handle it
        VorbisDecoder* decoder = new VorbisDecoder();
        framework_->GetThreadTaskManager()->AddThreadTask(Foundation::ThreadTaskPtr(decoder));
        
        // Set default master gains for sound types
        master_gain_ = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "master_gain", 1.0f);
        sound_master_gain_[Foundation::SoundServiceInterface::Triggered] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "triggered_sound_gain", 1.0f);
        sound_master_gain_[Foundation::SoundServiceInterface::Ambient] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "ambient_sound_gain", 1.0f);
        sound_master_gain_[Foundation::SoundServiceInterface::Voice] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "voice_sound_gain", 1.0f);
    }

    SoundSystem::~SoundSystem()
    {
        Uninitialize();

        framework_->GetDefaultConfig().SetSetting<Real>("SoundSystem", "master_gain", master_gain_);
        framework_->GetDefaultConfig().SetSetting<Real>("SoundSystem", "triggered_sound_gain", sound_master_gain_[Foundation::SoundServiceInterface::Triggered]);
        framework_->GetDefaultConfig().SetSetting<Real>("SoundSystem", "ambient_sound_gain", sound_master_gain_[Foundation::SoundServiceInterface::Ambient]);
        framework_->GetDefaultConfig().SetSetting<Real>("SoundSystem", "voice_sound_gain", sound_master_gain_[Foundation::SoundServiceInterface::Voice]);
    }

    StringVector SoundSystem::GetPlaybackDevices()
    {
        StringVector names;
        
        const char* device_names = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
        if (device_names)
        {
            while (*device_names)
            {
                names.push_back(std::string(device_names));
                device_names += strlen(device_names) + 1;
            }
        }
        
        return names;
    }
    
    bool SoundSystem::Initialize(const std::string& name)
    {
        if (initialized_)
            Uninitialize();
        
        if (name.empty())
            device_ = alcOpenDevice(NULL); 
        else
            device_ = alcOpenDevice(name.c_str());
        
        if (!device_)
        {
            OpenALAudioModule::LogWarning("Could not open OpenAL playback device " + name);
            return false;
        } 
          
        context_ = alcCreateContext(device_, NULL);
        if (!context_)
        {
            OpenALAudioModule::LogWarning("Could not create OpenAL playback context");
            return false;
        }
           
        alcMakeContextCurrent(context_);
        OpenALAudioModule::LogInfo("Opened OpenAL playback device " + name);
        initialized_ = true;
        return true;
    }

    void SoundSystem::Uninitialize()
    {
        StopRecording();
        
        channels_.clear();
        sounds_.clear();
        
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

    Foundation::SoundServiceInterface::SoundState SoundSystem::GetSoundState(sound_id_t id) const
    {
        SoundChannelMap::const_iterator i = channels_.find(id);
        if (i == channels_.end())
            return Foundation::SoundServiceInterface::Stopped;
        return i->second->GetState();
    }
    
    std::vector<sound_id_t> SoundSystem::GetActiveSounds() const
    {
        std::vector<sound_id_t> ret;
        
        SoundChannelMap::const_iterator i = channels_.begin();
        while (i != channels_.end())
        {
            if (i->second->GetState() != Foundation::SoundServiceInterface::Stopped)
                ret.push_back(i->first);
            ++i;
        }
        
        return ret;
    }
    
    const std::string& SoundSystem::GetSoundName(sound_id_t id) const
    {
        static std::string empty;
        
        SoundChannelMap::const_iterator i = channels_.find(id);
        if (i == channels_.end())
            return empty;
        return i->second->GetSoundName();
    }    

    Foundation::SoundServiceInterface::SoundType SoundSystem::GetSoundType(sound_id_t id) const
    {
        SoundChannelMap::const_iterator i = channels_.find(id);
        if (i == channels_.end())
            return Foundation::SoundServiceInterface::Triggered;
        return i->second->GetSoundType();
    }    
    
    void SoundSystem::Update(f64 frametime)
    {   
        if (!initialized_)
            return;
            
//        mutex.lock();
        std::vector<SoundChannelMap::iterator> channels_to_delete;

        // Update listener position/orientation to sound device
        ALfloat pos[] = {listener_position_.x, listener_position_.y, listener_position_.z};
        alListenerfv(AL_POSITION, pos);
        Vector3df front = listener_orientation_ * Vector3df(0.0f, -1.0f, 0.0f);
        Vector3df up = listener_orientation_ * Vector3df(0.0f, 0.0f, -1.0f); 
        ALfloat orient[] = {front.x, front.y, front.z, up.x, up.y, up.z};
        alListenerfv(AL_ORIENTATION, orient);    
        
        // Update channel attenuations, check which have stopped
        SoundChannelMap::iterator i = channels_.begin();
        while (i != channels_.end())
        {
            i->second->Update(listener_position_);
            if (i->second->GetState() == Foundation::SoundServiceInterface::Stopped)
            {
                channels_to_delete.push_back(i);
            }
            ++i;
        }
        
        // Remove stopped channels
        for (uint j = 0; j < channels_to_delete.size(); ++j)
            channels_.erase(channels_to_delete[j]);   
        
     //   mutex.unlock();
        
        // Age the sound cache
        UpdateCache(frametime);
    }
    
    void SoundSystem::SetListener(const Vector3df& position, const Quaternion& orientation)
    {
        if (!initialized_)
            return;
     
        listener_position_ = position;
        listener_orientation_ = orientation;
    }
      
    sound_id_t SoundSystem::PlaySound(const std::string& name, Foundation::SoundServiceInterface::SoundType type, bool local, sound_id_t channel)
    {
        if (!initialized_)
            return 0;
            
        SoundPtr sound = GetSound(name, local);
        if (!sound)
            return 0;

        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
        }
        
        i->second->SetMasterGain(sound_master_gain_[type] * master_gain_);
        i->second->SetPositional(false);
        i->second->Play(sound);
         
        return i->first;
    }
    
    sound_id_t SoundSystem::PlaySound3D(const std::string& name, Foundation::SoundServiceInterface::SoundType type, bool local, Vector3df position, sound_id_t channel)
    {
        if (!initialized_)
            return 0;
            
        SoundPtr sound = GetSound(name, local);
        if (!sound)
            return 0;
                
        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
        }
       
        i->second->SetMasterGain(sound_master_gain_[type] * master_gain_);
        i->second->SetPositional(true);
        i->second->SetPosition(position);
        i->second->Play(sound);
        
        return i->first;
    }

    sound_id_t SoundSystem::PlaySoundBuffer(const Foundation::SoundServiceInterface::SoundBuffer& buffer, Foundation::SoundServiceInterface::SoundType type, sound_id_t channel)
    {
        if (!initialized_)
            return 0;
            
        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
        }
        
        i->second->SetMasterGain(sound_master_gain_[type] * master_gain_);
        i->second->SetPositional(false);
        i->second->AddBuffer(buffer);
        
        return i->first;
    }
    
    sound_id_t SoundSystem::PlaySoundBuffer3D(const Foundation::SoundServiceInterface::SoundBuffer& buffer, Foundation::SoundServiceInterface::SoundType type, Vector3df position, sound_id_t channel)
    {
        if (!initialized_)
            return 0;
            
        SoundChannelMap::iterator i = channels_.find(channel);
        if (i == channels_.end())
        {
            i = channels_.insert(
                std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
        }
        
        i->second->SetMasterGain(sound_master_gain_[type] * master_gain_);
        i->second->SetPositional(true);
        i->second->SetPosition(position);
        i->second->AddBuffer(buffer);
        
        return i->first;
    }

    void SoundSystem::StopSound(sound_id_t id)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;    
            
        i->second->Stop();
    }
    
    void SoundSystem::SetPitch(sound_id_t id, Real pitch)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
            
        i->second->SetPitch(pitch);
    }
    
    void SoundSystem::SetGain(sound_id_t id, Real gain)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
        
        i->second->SetGain(gain);
    }
    
    void SoundSystem::SetLooped(sound_id_t id, bool looped)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
            
        i->second->SetLooped(looped);
    }
    
    void SoundSystem::SetPositional(sound_id_t id, bool positional)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
         
        i->second->SetPositional(positional);
    }
    
    void SoundSystem::SetPosition(sound_id_t id, Vector3df position)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
        
        i->second->SetPosition(position);
    }
    
    void SoundSystem::SetRange(sound_id_t id, Real inner_radius, Real outer_radius, Real rolloff)
    {
        SoundChannelMap::iterator i = channels_.find(id);
        if (i == channels_.end())
            return;
        
        i->second->SetRange(inner_radius, outer_radius, rolloff);
    }

    sound_id_t SoundSystem::GetNextSoundChannelID()
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
        if (!initialized_)
            return SoundPtr();
            
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
    
    void SoundSystem::UpdateCache(f64 frametime)
    {
        if (!initialized_)
            return;
            
        update_time_ += frametime;
        if (update_time_ < CACHE_CHECK_INTERVAL)
            return;
        
        uint total_size = 0;
        SoundMap::iterator oldest_sound = sounds_.end();
        f64 oldest_age = 0.0;
        
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
    
    bool SoundSystem::HandleTaskEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id != Task::Events::REQUEST_COMPLETED)
            return false;
        VorbisDecodeResult* result = dynamic_cast<VorbisDecodeResult*>(data);
        if (!result || result->task_description_ != "VorbisDecoder")
            return false;

        // Check if this was for a resource request, if so, stuff the data
        for (;;)
        {
            request_tag_t tag = 0;

            std::map<request_tag_t, std::string>::iterator i = sound_resource_requests_.begin();
            while (i != sound_resource_requests_.end())
            {
                if (i->second == result->name_)
                {
                    tag = i->first;
                    sound_resource_requests_.erase(i);
                    break;
                }
                ++i;
            }
            
            if (tag)
            {
                // Note: the decoded sound resource is not stored anywhere, just sent in the event wrapped to a smart pointer.
                // It's up to the caller to do whatever wanted with it.
                Foundation::SoundServiceInterface::SoundBuffer res_buffer;
                res_buffer.data_ = new unsigned char[result->buffer_.size()];
                res_buffer.size_ = result->buffer_.size();
                memcpy(res_buffer.data_, &result->buffer_[0], res_buffer.size_);
                res_buffer.frequency_ = result->frequency_;
                res_buffer.sixteenbit_ = true;
                res_buffer.stereo_ = result->stereo_;
                Foundation::SoundResource* res = new Foundation::SoundResource(result->name_, res_buffer);
                Foundation::ResourcePtr res_ptr(res);
                
                Resource::Events::ResourceReady event_data(result->name_, res_ptr, tag);
                Foundation::EventManagerPtr event_mgr = framework_->GetEventManager();
                event_mgr->SendEvent(event_mgr->QueryEventCategory("Resource"), Resource::Events::RESOURCE_READY, &event_data);
            }
            else
                break;
        }
        
        // If we can find the sound from our cache, and the result contains data, stuff the data into the sound
        SoundMap::iterator i = sounds_.find(result->name_);
        if (i == sounds_.end())
            return false;
        // If sound already has data, do not stuff again
        if (i->second->GetSize() != 0)
            return true;
        if (!result->buffer_.size())
            return true;
        
        Foundation::SoundServiceInterface::SoundBuffer vorbis_buffer;
        vorbis_buffer.data_ = &result->buffer_[0];
        vorbis_buffer.size_ = result->buffer_.size();
        vorbis_buffer.frequency_ = result->frequency_;
        vorbis_buffer.sixteenbit_ = true;
        vorbis_buffer.stereo_ = result->stereo_;
        
        i->second->LoadFromBuffer(vorbis_buffer); 
        return true;
    }
    
    bool SoundSystem::HandleAssetEvent(event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id != Asset::Events::ASSET_READY)
            return false;
        
        Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data);
        if (event_data->asset_type_ == RexTypes::ASSETTYPENAME_SOUNDVORBIS)
        {
            bool resource_request = false;
            
            if (!event_data->asset_)
                return false;
            
            // Check if this is for a sound resource request; in that case we allow redecode to get the raw sound data
            if (sound_resource_requests_.find(event_data->tag_) != sound_resource_requests_.end())
                resource_request = true;
            
            // Find the sound from our cache to see if it was already decoded
            if (!resource_request)
            {
                SoundMap::iterator i = sounds_.find(event_data->asset_id_);
                if (i == sounds_.end())
                    return false;
                // If sound already has data, do not queue another decode request
                if (i->second->GetSize() != 0)
                    return false;
            }
            
            VorbisDecodeRequestPtr new_request(new VorbisDecodeRequest());
            new_request->name_ = event_data->asset_id_;
            new_request->buffer_.resize(event_data->asset_->GetSize());
            //! \todo use asset data directly instead of copying to decode request buffer
            memcpy(&new_request->buffer_[0], event_data->asset_->GetData(), event_data->asset_->GetSize());
            framework_->GetThreadTaskManager()->AddRequest("VorbisDecoder", new_request);
        }
        
        return false;
    }
    
    void SoundSystem::SetMasterGain(Real master_gain)
    {
        master_gain_ = master_gain;
        ApplyMasterGain();
    }
    
    Real SoundSystem::GetMasterGain()
    {
        return master_gain_;
    }
    
    void SoundSystem::SetSoundMasterGain(Foundation::SoundServiceInterface::SoundType type, Real master_gain)
    {
        sound_master_gain_[type] = master_gain;
        ApplyMasterGain();
    }
    
    Real SoundSystem::GetSoundMasterGain(Foundation::SoundServiceInterface::SoundType type)
    {
        return sound_master_gain_[type];
    }

    void SoundSystem::ApplyMasterGain()
    {
        SoundChannelMap::iterator i = channels_.begin();
        while (i != channels_.end())
        {
            i->second->SetMasterGain(master_gain_ * sound_master_gain_[i->second->GetSoundType()]);
            ++i;
        }
    }
    
    StringVector SoundSystem::GetRecordingDevices()
    {
        StringVector names;
        
        const char* capture_device_names = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
        if (capture_device_names)
        {
            while (*capture_device_names)
            {
                names.push_back(std::string(capture_device_names));
                capture_device_names += strlen(capture_device_names) + 1;
            }
        }
        
        return names;
    }
    
    bool SoundSystem::StartRecording(const std::string& name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size)
    {
        if (!initialized_)
            return false;
        
        // Stop old recording if any
        StopRecording();
        
        ALenum openal_format;
        capture_sample_size_ = 1;
        if (stereo) 
            capture_sample_size_ <<= 1;
        if (sixteenbit) 
            capture_sample_size_ <<= 1;
        
        if (!stereo)
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_MONO8;
            else
                openal_format = AL_FORMAT_MONO16;
        }
        else
        {
            if (!sixteenbit)
                openal_format = AL_FORMAT_STEREO8;
            else
                openal_format = AL_FORMAT_STEREO16;
        }
        
        if (name.empty())
            capture_device_ = alcCaptureOpenDevice(NULL, frequency, openal_format, buffer_size / capture_sample_size_);
        else
            capture_device_ = alcCaptureOpenDevice(name.c_str(), frequency, openal_format, buffer_size / capture_sample_size_);
        
        if (!capture_device_)
        {
            OpenALAudioModule::LogError("Could not open OpenAL recording device " + name);
            return false;
        }
        
        alcCaptureStart(capture_device_);
        
        OpenALAudioModule::LogInfo("Opened OpenAL recording device " + name);
        return true;
    }
    
    void SoundSystem::StopRecording()
    {
        if (capture_device_)
        {
            alcCaptureStop(capture_device_);
            alcCaptureCloseDevice(capture_device_);
            capture_device_ = 0;
        }
    }
    
    uint SoundSystem::GetRecordedSoundSize()
    {
        if (!capture_device_)
            return 0;
        
        ALCint samples;
        alcGetIntegerv(capture_device_, ALC_CAPTURE_SAMPLES, 1, &samples);
        return samples * capture_sample_size_;
    }
    
    uint SoundSystem::GetRecordedSoundData(void* buffer, uint size)
    {
        if (!capture_device_)
            return 0;
        
        ALCint samples = size / capture_sample_size_;
        ALCint max_samples = 0;
        alcGetIntegerv(capture_device_, ALC_CAPTURE_SAMPLES, 1, &max_samples);
        if (samples > max_samples)
            samples = max_samples;
        
        alcCaptureSamples(capture_device_, buffer, samples);
        return samples * capture_sample_size_;
    }
    
    request_tag_t SoundSystem::RequestSoundResource(const std::string& assetid)
    {
        // Loading of sound from assetdata, assumed to be vorbis compressed stream
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
        if (asset_service)
        {
            request_tag_t tag = asset_service->RequestAsset(assetid, RexTypes::ASSETTYPENAME_SOUNDVORBIS);
            if (tag)
                sound_resource_requests_[tag] = assetid;
            
            return tag;
        }
        
        return 0;
    }
}
