// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"
#include <boost/algorithm/string.hpp>
#include <QList>
#include "MemoryLeakCheck.h"
#include "AudioAPI.h"
#include "CoreTypes.h"
#include "AssetAPI.h"
#include "AudioAsset.h"
#include "GenericAssetFactory.h"
#include "SoundChannel.h"
#include "LoggingFunctions.h"

#ifndef Q_WS_MAC
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

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

AudioAPI::AudioAPI(AssetAPI *assetAPI_)
:impl(new AudioApiImpl),
assetAPI(assetAPI_)
{
    assert(assetAPI);
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
        
    // Set default master gains for sound types
    /*
    masterGain = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "masterGain", 1.0f);
    soundMasterGain[SoundChannel::Triggered] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "triggered_sound_gain", 1.0f);
    soundMasterGain[SoundChannel::Ambient] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "ambient_sound_gain", 1.0f);
    soundMasterGain[SoundChannel::Voice] = framework_->GetDefaultConfig().DeclareSetting("SoundSystem", "voice_sound_gain", 1.0f);
    */

    /// \todo Is mp3 really supported? Copied over from AssetAPI where file extensions were resolved to a type.
    QStringList supportedAudioFormats;
    supportedAudioFormats << "wav" << "ogg" << "mp3";
    assetAPI->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AudioAsset>("Audio", supportedAudioFormats))); 
}

AudioAPI::~AudioAPI()
{
    Uninitialize();
    delete impl;
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
        while(*deviceNames)
        {
            names.push_back(QString(deviceNames));
            deviceNames += strlen(deviceNames) + 1;
        }
    }
    
    return names;
}

AudioAssetPtr AudioAPI::CreateAudioAssetFromSoundBuffer(const SoundBuffer &buffer)
{
    // Construct a sound from the buffer
    AudioAssetPtr new_sound(new AudioAsset(assetAPI, "Audio", "buffer"));
    new_sound->LoadFromSoundBuffer(buffer);

    // If failed for some reason (out of memory?), bail out
    if (!new_sound->GetHandle())
        return AudioAssetPtr();

    return new_sound;
}

void AudioAPI::Uninitialize()
{
    if (!impl)
        return;

    StopRecording();
    
    impl->channels.clear();
    
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

std::vector<SoundChannelPtr> AudioAPI::GetActiveSounds() const
{
    std::vector<SoundChannelPtr> ret;
    
    SoundChannelMap::const_iterator i = impl->channels.begin();
    while(i != impl->channels.end())
    {
        if (i->second->GetState() != SoundChannel::Stopped)
            ret.push_back(i->second);
        ++i;
    }
    
    return ret;
}

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
    while(i != impl->channels.end())
    {
        i->second->Update(impl->listenerPosition);
        if (i->second->GetState() == SoundChannel::Stopped)
        {
            channelsToDelete.push_back(i);
        }
        ++i;
    }
    
    // Remove stopped channels
    for(uint j = 0; j < channelsToDelete.size(); ++j)
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

SoundChannelPtr AudioAPI::PlaySoundBuffer(const SoundBuffer &buffer, SoundChannel::SoundType type, SoundChannelPtr channel)
{
    if (!impl->initialized)
        return SoundChannelPtr();
        
    if (!channel)
    {
        sound_id_t newId = GetNextSoundChannelID();
        channel = SoundChannelPtr(new SoundChannel(newId, type));
        impl->channels.insert(make_pair(newId, channel));
    }

    AudioAssetPtr audioAsset = CreateAudioAssetFromSoundBuffer(buffer);

    channel->SetMasterGain(impl->soundMasterGain[type] * impl->masterGain);
    channel->SetPositional(false);
    channel->AddBuffer(audioAsset);
    
    return channel;
}

SoundChannelPtr AudioAPI::PlaySoundBuffer3D(const SoundBuffer &buffer, SoundChannel::SoundType type, Vector3df position, SoundChannelPtr channel)
{
    if (!impl->initialized)
        return SoundChannelPtr();
        
    if (!channel)
    {
        sound_id_t newId = GetNextSoundChannelID();
        channel = SoundChannelPtr(new SoundChannel(newId, type));
        impl->channels.insert(make_pair(newId, channel));
    }

    AudioAssetPtr audioAsset = CreateAudioAssetFromSoundBuffer(buffer);

    channel->SetMasterGain(impl->soundMasterGain[type] * impl->masterGain);
    channel->SetPositional(true);
    channel->SetPosition(position);
    channel->AddBuffer(audioAsset);
    
    return channel;
}

void AudioAPI::Stop(SoundChannelPtr channel)
{
    if (channel)
        channel->Stop();
}

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
    while(i != impl->channels.end())
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
        while(*capture_device_names)
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
