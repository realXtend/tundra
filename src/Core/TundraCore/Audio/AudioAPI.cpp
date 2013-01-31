// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Win.h"
#include "AudioAPI.h"
#include "AudioAsset.h"
#include "SoundChannel.h"

#include "CoreDefines.h"
#include "CoreTypes.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "LoggingFunctions.h"
#include "Framework.h"
#include "Profiler.h"
#include "Math/float3.h"
#include "ConfigAPI.h"

#ifndef TUNDRA_NO_AUDIO
#ifndef Q_WS_MAC
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif
#else
struct ALCcontext;
struct ALCdevice;
#endif

#include "MemoryLeakCheck.h"

using namespace std;

struct AudioAPI::AudioApiImpl
{
public:
    AudioApiImpl() :
        listenerPosition(0,0,0),
        listenerOrientation(Quat::identity),
        initialized(false),
        context(0),
        device(0),
        captureDevice(0),
        captureSampleSize(0),
        nextChannelId(0),
        masterGain(0.0f)
    {
    }

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
    float3 listenerPosition;
    /// Listener orientation
    Quat listenerOrientation;
    
    /// Master gain for whole sound system
    float masterGain;
    /// Master gain for individual sound types
    std::map<SoundChannel::SoundType, float> soundMasterGain;
};

AudioAPI::AudioAPI(Framework *fw, AssetAPI *assetAPI_)
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
    impl->listenerPosition = float3(0.0, 0.0, 0.0);
    
    QStringList audioDevice = fw->CommandLineParameters("--audiodevice");
    QString device = "";
    if (audioDevice.size() >= 1)
        device = audioDevice.back();
    if (audioDevice.size() > 1)
        LogWarning("Specified multiple --audiodevice parameters. Using \"" + device + "\".");
    Initialize(device);

    // Load sound settings. If we have "master_gain" in config we very likely have all the other settings as well.
    if (fw->Config()->HasValue(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_SOUND, "master_gain"))
        LoadSoundSettingsFromConfig();

    QStringList audioTypeExtensions;
    audioTypeExtensions << ".wav" << ".ogg" << ".mp3";
    
    if (!fw->IsHeadless())
        assetAPI->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<AudioAsset>("Audio", audioTypeExtensions)));
    else
        assetAPI->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Audio", audioTypeExtensions)));
}

AudioAPI::~AudioAPI()
{
}

void AudioAPI::Reset()
{
    Uninitialize();
    SAFE_DELETE(impl);
}

bool AudioAPI::Initialize(const QString &playbackDeviceName)
{
    if (impl && impl->initialized)
        Uninitialize();
    
#ifndef TUNDRA_NO_AUDIO
    if (playbackDeviceName.isEmpty())
        impl->device = alcOpenDevice(NULL);
    else
        impl->device = alcOpenDevice(playbackDeviceName.toStdString().c_str());

    if (!impl->device)
    {
        LogWarning("Could not open OpenAL playback device " + playbackDeviceName);
        return false;
    }

    impl->context = alcCreateContext(impl->device, NULL);
    if (!impl->context)
    {
        LogWarning("Could not create OpenAL playback context");
        return false;
    }

    alcMakeContextCurrent(impl->context);
    if (playbackDeviceName.isEmpty())
        LogInfo("Opened default OpenAL playback device.");
    else
        LogInfo("Opened OpenAL playback device '" + playbackDeviceName + "'.");

    impl->initialized = true;
    
#endif
    return true;
}

QStringList AudioAPI::GetPlaybackDevices() const
{
    QStringList names;

#ifndef TUNDRA_NO_AUDIO
    const char *deviceNames = alcGetString(NULL, ALC_DEVICE_SPECIFIER);
    if (deviceNames)
    {
        while(*deviceNames)
        {
            names.push_back(QString(deviceNames));
            deviceNames += strlen(deviceNames) + 1;
        }
    }
#endif
    
    return names;
}

AudioAssetPtr AudioAPI::CreateAudioAssetFromSoundBuffer(const SoundBuffer &buffer) const
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

#ifndef TUNDRA_NO_AUDIO
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
#endif    
    
    impl->initialized = false;
}

std::vector<SoundChannelPtr> AudioAPI::GetActiveSounds() const
{
    std::vector<SoundChannelPtr> ret;
    
    SoundChannelMap::const_iterator i = impl->channels.begin();
    while(i != impl->channels.end())
    {
        if (i->second->State() != SoundChannel::Stopped)
            ret.push_back(i->second);
        ++i;
    }
    
    return ret;
}

void AudioAPI::Update(f64 /*frametime*/)
{
    if (!impl || !impl->initialized)
        return;
    
#ifndef TUNDRA_NO_AUDIO
    PROFILE(AudioAPI_Update);

//        mutex.lock();
    std::vector<SoundChannelMap::iterator> channelsToDelete;

    // Update listener position/orientation to sound device
    ALfloat pos[] = {impl->listenerPosition.x, impl->listenerPosition.y, impl->listenerPosition.z};
    alListenerfv(AL_POSITION, pos);
    float3 front = impl->listenerOrientation * float3(0.0f, -1.0f, 0.0f);
    float3 up = impl->listenerOrientation * float3(0.0f, 0.0f, -1.0f);
    ALfloat orient[] = {front.x, front.y, front.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, orient);

    // Update channel attenuations, check which have stopped
    SoundChannelMap::iterator i = impl->channels.begin();
    while(i != impl->channels.end())
    {
        i->second->Update(impl->listenerPosition);
        if (i->second->State() == SoundChannel::Stopped)
        {
            channelsToDelete.push_back(i);
        }
        ++i;
    }

    // Remove stopped channels
    for(uint j = 0; j < channelsToDelete.size(); ++j)
        impl->channels.erase(channelsToDelete[j]);

 //   mutex.unlock();
#endif
}

bool AudioAPI::IsInitialized() const
{
    return impl && impl->initialized;
}

void AudioAPI::SaveSoundSettingsToConfig()
{
    if (IsInitialized())
    {
        ConfigAPI &cfg = *assetAPI->GetFramework()->Config();
        ConfigData sound(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_SOUND);
        cfg.Set(sound, "master_gain", (double)GetMasterGain());
        cfg.Set(sound, "triggered_sound_gain", (double)GetSoundMasterGain(SoundChannel::Triggered));
        cfg.Set(sound, "ambient_sound_gain", (double)GetSoundMasterGain(SoundChannel::Ambient));
        cfg.Set(sound, "voice_sound_gain", (double)GetSoundMasterGain(SoundChannel::Voice));
    }
}

void AudioAPI::LoadSoundSettingsFromConfig()
{
    if (IsInitialized())
    {
        ConfigAPI &cfg = *assetAPI->GetFramework()->Config();
        ConfigData sound(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_SOUND);
        bool ok;
        float val = cfg.Get(sound, "master_gain").toFloat(&ok);
        if (ok)
            SetMasterGain(val);
        val = cfg.Get(sound, "triggered_sound_gain").toFloat(&ok);
        if (ok)
            SetSoundMasterGain(SoundChannel::Triggered, val);
        val = cfg.Get(sound, "ambient_sound_gain").toFloat(&ok);
        if (ok)
            SetSoundMasterGain(SoundChannel::Ambient, val);
        val = cfg.Get(sound, "voice_sound_gain").toFloat(&ok);
        if (ok)
            SetSoundMasterGain(SoundChannel::Voice, val);
    }
}

void AudioAPI::SetListener(const float3 &position, const Quat &orientation)
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
    channel->Play(dynamic_pointer_cast<AudioAsset>(audioAsset));

    return channel;
}

SoundChannelPtr AudioAPI::PlaySound3D(const float3 &position, AssetPtr audioAsset, SoundChannel::SoundType type, SoundChannelPtr channel)
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
    channel->Play(dynamic_pointer_cast<AudioAsset>(audioAsset));

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

SoundChannelPtr AudioAPI::PlaySoundBuffer3D(const SoundBuffer &buffer, SoundChannel::SoundType type, const float3 &position, SoundChannelPtr channel)
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

void AudioAPI::Stop(SoundChannelPtr channel) const
{
    if (channel)
        channel->Stop();
}

sound_id_t AudioAPI::GetNextSoundChannelID() const
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

float AudioAPI::GetMasterGain() const
{
    return impl ? impl->masterGain : 0.f;
}

void AudioAPI::SetSoundMasterGain(SoundChannel::SoundType type, float masterGain)
{
    impl->soundMasterGain[type] = masterGain;
    ApplyMasterGain();
}

float AudioAPI::GetSoundMasterGain(SoundChannel::SoundType type) const
{
    return impl ? impl->soundMasterGain[type] : 0.f;
}

void AudioAPI::ApplyMasterGain()
{
    SoundChannelMap::iterator i = impl->channels.begin();
    while(i != impl->channels.end())
    {
        i->second->SetMasterGain(impl->masterGain * impl->soundMasterGain[i->second->Type()]);
        ++i;
    }
}

QStringList AudioAPI::GetRecordingDevices() const
{
    QStringList names;

#ifndef TUNDRA_NO_AUDIO
    const char *capture_device_names = alcGetString(NULL, ALC_CAPTURE_DEVICE_SPECIFIER);
    if (capture_device_names)
    {
        while(*capture_device_names)
        {
            names.push_back(QString(capture_device_names));
            capture_device_names += strlen(capture_device_names) + 1;
        }
    }
#endif
    
    return names;
}

bool AudioAPI::StartRecording(const QString &name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size)
{
    if (!impl || !impl->initialized)
        return false;
    
#ifndef TUNDRA_NO_AUDIO
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
        LogError("Could not open OpenAL recording device " + name);
        return false;
    }

    alcCaptureStart(impl->captureDevice);

    LogInfo("Opened OpenAL recording device " + name);
    return true;
#endif
}

void AudioAPI::StopRecording()
{
#ifndef TUNDRA_NO_AUDIO
    if (impl && impl->captureDevice)
    {
        alcCaptureStop(impl->captureDevice);
        alcCaptureCloseDevice(impl->captureDevice);
        impl->captureDevice = 0;
    }
#endif
}

uint AudioAPI::GetRecordedSoundSize() const
{
    if (!impl || !impl->captureDevice)
        return 0;

#ifndef TUNDRA_NO_AUDIO
    ALCint samples;
    alcGetIntegerv(impl->captureDevice, ALC_CAPTURE_SAMPLES, 1, &samples);
    return samples * impl->captureSampleSize;
#endif
}

uint AudioAPI::GetRecordedSoundData(void* buffer, uint size)
{
    if (!impl || !impl->captureDevice)
        return 0;
    
#ifndef TUNDRA_NO_AUDIO
    ALCint samples = size / impl->captureSampleSize;
    ALCint max_samples = 0;
    alcGetIntegerv(impl->captureDevice, ALC_CAPTURE_SAMPLES, 1, &max_samples);
    if (samples > max_samples)
        samples = max_samples;
    
    alcCaptureSamples(impl->captureDevice, buffer, samples);
    return samples * impl->captureSampleSize;
#endif
}
