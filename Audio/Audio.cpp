// For conditions of distribution and use, see copyright notice in license.txt

//#include "StableHeaders.h"
#include "Audio.h"
//#include "WavLoader.h"
//#include "VorbisDecoder.h"
#include "CoreTypes.h"
//#include "ConfigurationManager.h"
//#include "ThreadTaskManager.h"
//#include "ServiceManager.h"
//#include "EventManager.h"
#include "AudioAsset.h"
#include "SoundChannel.h"
#include "LoggingFunctions.h"

#include <AL/al.h>
#include <AL/alc.h>

DEFINE_POCO_LOGGING_FUNCTIONS("Audio")

//#include <boost/thread/mutex.hpp>

using namespace std;

struct AudioApiImpl
{
public:
    /// Initialized flag
    bool initialized;
    /// OpenAL context
    ALCcontext *context;
    /// OpenAL device
    ALCdevice *device;
    /// OpenAL capture device
    ALCdevice *captureDevice;
    /// Capture sample size
    uint captureSampleSize;
    /// Active channels
    SoundChannelMap channels;
    /// Currently loaded sounds
//    SoundMap sounds;
    /// Update timer (for cache)
//    f64 updateTime;
    /// Next channel id
    sound_id_t nextChannelId;
    
    /// Listener position
    Vector3df listenerPosition;
    /// Listener orientation
    Quaternion listenerOrientation;
    
    /// Master gain for whole sound system
    float masterGain;
    /// Master gain for individual sound types
    std::map<SoundChannel::SoundType, float> soundMasterGain;

//    boost::mutex mutex;
};

AudioAPI::AudioAPI()
:impl(new AudioApiImpl)
{
    impl->initialized = false;
    impl->masterGain = 1.f;
    impl->context = 0;
    impl->device = 0;
    impl->captureDevice = 0;
    impl->captureSampleSize = 0;
    impl->nextChannelId = 1;
    impl->soundMasterGain[SoundChannel::Triggered] = 1.f;
    impl->soundMasterGain[SoundChannel::Ambient] = 1.f;
    impl->soundMasterGain[SoundChannel::Voice] = 1.f;
    impl->listenerPosition = Vector3df(0.0, 0.0, 0.0);
    
    // By default, initialize default playback device
    Initialize();
    
    // Create vorbis decoder thread task and let the framework thread task manager handle it
//    VorbisDecoder* decoder = new VorbisDecoder();
//    framework_->GetThreadTaskManager()->AddThreadTask(Foundation::ThreadTaskPtr(decoder));
    
    // Set default master gains for sound types
/*
    masterGain = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "masterGain", 1.0f);
    soundMasterGain[SoundChannel::Triggered] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "triggered_sound_gain", 1.0f);
    soundMasterGain[SoundChannel::Ambient] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "ambient_sound_gain", 1.0f);
    soundMasterGain[SoundChannel::Voice] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "voice_sound_gain", 1.0f);
*/
}

AudioAPI::~AudioAPI()
{
    Uninitialize();
/*
    framework_->GetDefaultConfig().SetSetting<float>("SoundSystem", "masterGain", masterGain);
    framework_->GetDefaultConfig().SetSetting<float>("SoundSystem", "triggered_sound_gain", soundMasterGain[SoundChannel::Triggered]);
    framework_->GetDefaultConfig().SetSetting<float>("SoundSystem", "ambient_sound_gain", soundMasterGain[SoundChannel::Ambient]);
    framework_->GetDefaultConfig().SetSetting<float>("SoundSystem", "voice_sound_gain", soundMasterGain[SoundChannel::Voice]);
*/
}

bool AudioAPI::Initialize(const QString &playbackDeviceName)
{
    if (impl && impl->initialized)
        Uninitialize();
    
    if (playbackDeviceName.isEmpty())
        impl->device = alcOpenDevice(NULL); 
    else
        impl->device = alcOpenDevice(playbackDeviceName.toStdString().c_str());
    
    if (!impl->device)
    {
        LogWarning("Could not open OpenAL playback device " + playbackDeviceName.toStdString());
        return false;
    } 
      
    impl->context = alcCreateContext(impl->device, NULL);
    if (!impl->context)
    {
        LogWarning("Could not create OpenAL playback context");
        return false;
    }
       
    alcMakeContextCurrent(impl->context);
    LogInfo("Opened OpenAL playback device " + playbackDeviceName.toStdString());
    impl->initialized = true;
    return true;
}

QStringList AudioAPI::GetPlaybackDevices()
{
    QStringList names;
    
    const char *deviceNames = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    if (deviceNames)
    {
        while (*deviceNames)
        {
            names.push_back(QString(deviceNames));
            deviceNames += strlen(deviceNames) + 1;
        }
    }
    
    return names;
}

void AudioAPI::Uninitialize()
{
    if (!impl)
        return;

    StopRecording();
    
    impl->channels.clear();
//    sounds.clear();
    
    if (impl->context)
    {
        alcMakeContextCurrent(0);
        alcDestroyContext(impl->context);
        impl->context = 0;
    }
    if (impl->device)
    {
        alcCloseDevice(impl->device);
        impl->device = 0;
    }
    
    impl->initialized = false;
}
/*
SoundChannel::SoundState AudioAPI::GetSoundState(sound_id_t id) const
{
    SoundChannelMap::const_iterator i = channels.find(id);
    if (i == channels.end())
        return SoundChannel::Stopped;
    return i->second->GetState();
}
*/
std::vector<SoundChannelPtr> AudioAPI::GetActiveSounds() const
{
    std::vector<SoundChannelPtr> ret;
    
    SoundChannelMap::const_iterator i = impl->channels.begin();
    while (i != impl->channels.end())
    {
        if (i->second->GetState() != SoundChannel::Stopped)
            ret.push_back(i->second);
        ++i;
    }
    
    return ret;
}
/*
QString AudioAPI::GetSoundName(sound_id_t id) const
{
    static QString empty;
    
    SoundChannelMap::const_iterator i = channels.find(id);
    if (i == channels.end())
        return empty;
    return QString::fromStdString(i->second->GetSoundName());
}    

SoundChannel::SoundType AudioAPI::GetSoundType(sound_id_t id) const
{
    SoundChannelMap::const_iterator i = channels.find(id);
    if (i == channels.end())
        return SoundChannel::Triggered;
    return i->second->GetSoundType();
}    
*/
void AudioAPI::Update(f64 frametime)
{   
    if (!impl || !impl->initialized)
        return;
        
//        mutex.lock();
    std::vector<SoundChannelMap::iterator> channelsToDelete;

    // Update listener position/orientation to sound device
    ALfloat pos[] = {impl->listenerPosition.x, impl->listenerPosition.y, impl->listenerPosition.z};
    alListenerfv(AL_POSITION, pos);
    Vector3df front = impl->listenerOrientation * Vector3df(0.0f, -1.0f, 0.0f);
    Vector3df up = impl->listenerOrientation * Vector3df(0.0f, 0.0f, -1.0f); 
    ALfloat orient[] = {front.x, front.y, front.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orient);
    
    // Update channel attenuations, check which have stopped
    SoundChannelMap::iterator i = impl->channels.begin();
    while (i != impl->channels.end())
    {
        i->second->Update(impl->listenerPosition);
        if (i->second->GetState() == SoundChannel::Stopped)
        {
            channelsToDelete.push_back(i);
        }
        ++i;
    }
    
    // Remove stopped channels
    for (uint j = 0; j < channelsToDelete.size(); ++j)
        impl->channels.erase(channelsToDelete[j]);   
    
 //   mutex.unlock();
}

bool AudioAPI::IsInitialized() const
{ 
    return impl && impl->initialized;
}

void AudioAPI::SetListener(const Vector3df &position, const Quaternion &orientation)
{
    if (!impl || !impl->initialized)
        return;
 
    impl->listenerPosition = position;
    impl->listenerOrientation = orientation;
}
  
SoundChannelPtr AudioAPI::PlaySound(AssetPtr audioAsset, SoundChannel::SoundType type, SoundChannelPtr channel)
{
    if (!impl || !impl->initialized)
        return SoundChannelPtr();

    if (!channel)
    {
        sound_id_t newId = GetNextSoundChannelID();
        channel = SoundChannelPtr(new SoundChannel(newId, type));
        impl->channels.insert(make_pair(newId, channel));
    }

    channel->SetMasterGain(impl->soundMasterGain[type] * impl->masterGain);
    channel->SetPositional(false);
    channel->Play(boost::dynamic_pointer_cast<AudioAsset>(audioAsset));

    return channel;
}

SoundChannelPtr AudioAPI::PlaySound3D(const Vector3df &position, AssetPtr audioAsset, SoundChannel::SoundType type, SoundChannelPtr channel)
{
    if (!impl || !impl->initialized)
        return SoundChannelPtr();

    if (!channel)
    {
        sound_id_t newId = GetNextSoundChannelID();
        channel = SoundChannelPtr(new SoundChannel(newId, type));
        impl->channels.insert(make_pair(newId, channel));
    }

    channel->SetMasterGain(impl->soundMasterGain[type] * impl->masterGain);
    channel->SetPositional(true);
    channel->SetPosition(position);
    channel->Play(boost::dynamic_pointer_cast<AudioAsset>(audioAsset));

    return channel;
}
/*
sound_id_t AudioAPI::PlaySoundBuffer(const SoundChannel::SoundBuffer& buffer, SoundChannel::SoundType type, sound_id_t channel)
{
    if (!initialized)
        return 0;
        
    SoundChannelMap::iterator i = channels.find(channel);
    if (i == channels.end())
    {
        i = channels.insert(
            std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
    }
    
    i->second->SetMasterGain(soundMasterGain[type] * masterGain);
    i->second->SetPositional(false);
    i->second->AddBuffer(buffer);
    
    return i->first;
}

sound_id_t AudioAPI::PlaySoundBuffer3D(const SoundChannel::SoundBuffer& buffer, SoundChannel::SoundType type, Vector3df position, sound_id_t channel)
{
    if (!initialized)
        return 0;
        
    SoundChannelMap::iterator i = channels.find(channel);
    if (i == channels.end())
    {
        i = channels.insert(
            std::pair<sound_id_t, SoundChannelPtr>(GetNextSoundChannelID(), SoundChannelPtr(new SoundChannel(type)))).first;
    }
    
    i->second->SetMasterGain(soundMasterGain[type] * masterGain);
    i->second->SetPositional(true);
    i->second->SetPosition(position);
    i->second->AddBuffer(buffer);
    
    return i->first;
}
*/
void AudioAPI::Stop(SoundChannelPtr channel)
{
    if (channel)
        channel->Stop();
/*
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;    
        
    i->second->Stop();
*/
}
/*
void AudioAPI::SetPitch(sound_id_t id, float pitch)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
        
    i->second->SetPitch(pitch);
}

float AudioAPI::GetPitch(sound_id_t id) const
{
    SoundChannelMap::const_iterator i = channels.find(id);
    if (i == channels.end())
    {
        LogWarning("Fail to find sound channel for id:" + id);
        return -1.0f;
    }

    return i->second->GetPitch();
}

void AudioAPI::SetGain(sound_id_t id, float gain)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
    
    i->second->SetGain(gain);
}

float AudioAPI::GetGain(sound_id_t id) const
{
    SoundChannelMap::const_iterator i = channels.find(id);
    if (i == channels.end())
    {
        LogWarning("Fail to find sound channel for id:" + id);
        return -1.0f;
    }

    return i->second->GetGain();
}

void AudioAPI::SetLooped(sound_id_t id, bool looped)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
        
    i->second->SetLooped(looped);
}

void AudioAPI::SetPositional(sound_id_t id, bool positional)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
     
    i->second->SetPositional(positional);
}

void AudioAPI::SetPosition(sound_id_t id, Vector3df position)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
    
    i->second->SetPosition(position);
}

void AudioAPI::SetRange(sound_id_t id, float inner_radius, float outer_radius, float rolloff)
{
    SoundChannelMap::iterator i = channels.find(id);
    if (i == channels.end())
        return;
    
    i->second->SetRange(inner_radius, outer_radius, rolloff);
}
*/

sound_id_t AudioAPI::GetNextSoundChannelID()
{
    assert(impl);
    if (!impl)
        return 0;

    for(;;)
    {
        impl->nextChannelId++;
        if (!impl->nextChannelId)
            impl->nextChannelId = 1;
        // Must be currently unused
        if (impl->channels.find(impl->nextChannelId) == impl->channels.end())
            break;
    }
    
    return impl->nextChannelId;
}

#if 0
SoundPtr AudioAPI::GetSound(const std::string& name, bool local)
{
    if (!initialized)
        return SoundPtr();
        
    SoundMap::iterator i = sounds.find(name);
    if (i != sounds.end())
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
                sounds[name] = new_sound;
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
                sounds[name] = new_sound;
                return new_sound;
            }
        }
    }
    else
    {
                ///\todo Regression. Reimplement using Asset API. -jj.
/*
        // Loading of sound from assetdata, assumed to be vorbis compressed stream
        boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Service::ST_Asset).lock();
        if (asset_service)
        {
            SoundPtr new_sound(new Sound(name));
            sounds[name] = new_sound;
            
            // The sound will be filled with data later
            asset_service->RequestAsset(name, "SoundVorbis");
            return new_sound;
        }
        */
    }
    
    return SoundPtr();
}
#endif
/*
bool AudioAPI::DecodeLocalOggFile(Sound* sound, const std::string& name)
{
    boost::filesystem::path file_path(name);
    std::ifstream file(file_path.native_directory_string().c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        LogError("Could not open file: " + name + ".");
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
*/

#if 0
bool AudioAPI::HandleTaskEvent(event_id_t event_id, IEventData* data)
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
/*                     ///\todo Regression. Reimplement using Asset API. -jj.

        if (tag)
        {
            // Note: the decoded sound resource is not stored anywhere, just sent in the event wrapped to a smart pointer.
            // It's up to the caller to do whatever wanted with it.
            SoundResource* res = new SoundResource(result->name_, result->buffer_);
            Foundation::ResourcePtr res_ptr(res);
            
            Resource::Events::ResourceReady event_data(result->name_, res_ptr, tag);
            EventManagerPtr event_mgr = framework_->GetEventManager();
            event_mgr->SendEvent(event_mgr->QueryEventCategory("Resource"), Resource::Events::RESOURCE_READY, &event_data);
        }
        else
            break;
*/
    }
    
    // If we can find the sound from our cache, and the result contains data, stuff the data into the sound
    SoundMap::iterator i = sounds.find(result->name_);
    if (i == sounds.end())
        return false;
    // If sound already has data, do not stuff again
    if (i->second->GetSize() != 0)
        return true;
    if (!result->buffer_.data_.size())
        return true;
    
    i->second->LoadFromBuffer(result->buffer_);
    return true;
}

bool AudioAPI::HandleAssetEvent(event_id_t event_id, IEventData* data)
{
    ///\todo Regression. Reimplement using the Asset API. -jj.
    return false;
/*
    if (event_id != Asset::Events::ASSET_READY)
        return false;
    
    Asset::Events::AssetReady *event_data = checked_static_cast<Asset::Events::AssetReady*>(data);
    if (event_data->asset_type_ == "SoundVorbis")
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
            SoundMap::iterator i = sounds.find(event_data->asset_id_);
            if (i == sounds.end())
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
*/
}
#endif

void AudioAPI::SetMasterGain(float masterGain)
{
    impl->masterGain = masterGain;
    ApplyMasterGain();
}

float AudioAPI::GetMasterGain()
{
    return impl ? impl->masterGain : 0.f;
}

void AudioAPI::SetSoundMasterGain(SoundChannel::SoundType type, float masterGain)
{
    impl->soundMasterGain[type] = masterGain;
    ApplyMasterGain();
}

float AudioAPI::GetSoundMasterGain(SoundChannel::SoundType type)
{
    return impl ? impl->soundMasterGain[type] : 0.f;
}

void AudioAPI::ApplyMasterGain()
{
    SoundChannelMap::iterator i = impl->channels.begin();
    while (i != impl->channels.end())
    {
        i->second->SetMasterGain(impl->masterGain * impl->soundMasterGain[i->second->GetSoundType()]);
        ++i;
    }
}

QStringList AudioAPI::GetRecordingDevices()
{
    QStringList names;
    
    const char *capture_device_names = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (capture_device_names)
    {
        while (*capture_device_names)
        {
            names.push_back(QString(capture_device_names));
            capture_device_names += strlen(capture_device_names) + 1;
        }
    }
    
    return names;
}

bool AudioAPI::StartRecording(const QString &name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size)
{
    if (!impl || !impl->initialized)
        return false;
    
    // Stop old recording if any
    StopRecording();
    
    ALenum openal_format;
    impl->captureSampleSize = 1;
    if (stereo) 
        impl->captureSampleSize <<= 1;
    if (sixteenbit) 
        impl->captureSampleSize <<= 1;
    
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
    
    if (name.isEmpty())
        impl->captureDevice = alcCaptureOpenDevice(NULL, frequency, openal_format, buffer_size / impl->captureSampleSize);
    else
        impl->captureDevice = alcCaptureOpenDevice(name.toStdString().c_str(), frequency, openal_format, buffer_size / impl->captureSampleSize);
    
    if (!impl->captureDevice)
    {
        LogError("Could not open OpenAL recording device " + name.toStdString());
        return false;
    }
    
    alcCaptureStart(impl->captureDevice);
    
    LogInfo("Opened OpenAL recording device " + name.toStdString());
    return true;
}

void AudioAPI::StopRecording()
{
    if (impl && impl->captureDevice)
    {
        alcCaptureStop(impl->captureDevice);
        alcCaptureCloseDevice(impl->captureDevice);
        impl->captureDevice = 0;
    }
}

uint AudioAPI::GetRecordedSoundSize()
{
    if (!impl || !impl->captureDevice)
        return 0;
    
    ALCint samples;
    alcGetIntegerv(impl->captureDevice, ALC_CAPTURE_SAMPLES, 1, &samples);
    return samples * impl->captureSampleSize;
}

uint AudioAPI::GetRecordedSoundData(void* buffer, uint size)
{
    if (!impl || !impl->captureDevice)
        return 0;
    
    ALCint samples = size / impl->captureSampleSize;
    ALCint max_samples = 0;
    alcGetIntegerv(impl->captureDevice, ALC_CAPTURE_SAMPLES, 1, &max_samples);
    if (samples > max_samples)
        samples = max_samples;
    
    alcCaptureSamples(impl->captureDevice, buffer, samples);
    return samples * impl->captureSampleSize;
}
