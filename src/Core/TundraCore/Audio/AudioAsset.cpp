// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AudioAsset.h"
#include "AssetAPI.h"
#include "LoggingFunctions.h"
#include "WavLoader.h"
#include "OggVorbisLoader.h"

#include <QString>

#ifndef TUNDRA_NO_AUDIO
#ifndef Q_WS_MAC
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif
#endif

#include "MemoryLeakCheck.h"

AudioAsset::AudioAsset(AssetAPI *owner, const QString &type_, const QString &name_)
:IAsset(owner, type_, name_), handle(0)
{
}

AudioAsset::~AudioAsset()
{
    Unload();
}

void AudioAsset::DoUnload()
{
#ifndef TUNDRA_NO_AUDIO
    if (handle)
    {
        alDeleteBuffers(1, &handle);
        handle = 0;
    }
#endif
}

bool AudioAsset::DeserializeFromData(const u8 *data, size_t numBytes, bool /*allowAsynchronous*/)
{
    bool loadResult = false;

#ifndef TUNDRA_NO_AUDIO
    if (WavLoader::IdentifyWavFileInMemory(data, numBytes) && this->Name().endsWith(".wav", Qt::CaseInsensitive)) // Detect whether this file is Wav data or not.
    {
        loadResult = LoadFromWavFileInMemory(data, numBytes);
        if (loadResult)
            assetAPI->AssetLoadCompleted(Name());
    }
    else if (this->Name().endsWith(".ogg", Qt::CaseInsensitive))
    {
        loadResult = LoadFromOggVorbisFileInMemory(data, numBytes);
        if (loadResult)
            assetAPI->AssetLoadCompleted(Name());
    }
    else
        LogError("Unable to serialize audio asset data. Unknown format!");
#endif

    return loadResult;
}

bool AudioAsset::LoadFromWavFileInMemory(const u8 *data, size_t numBytes)
{
    SoundBuffer buf;
    bool success = WavLoader::LoadWavFileToSoundBuffer(data, numBytes, buf);
    if (!success || buf.data.size() == 0)
        return false;

    return LoadFromRawPCMWavData(&buf.data[0], buf.data.size(), buf.stereo, buf.is16Bit, buf.frequency);
}

bool AudioAsset::LoadFromOggVorbisFileInMemory(const u8 *data, size_t numBytes)
{
    SoundBuffer buf;
    bool success = OggVorbisLoader::LoadOggVorbisFileToSoundBuffer(data, numBytes, buf);
    if (!success || buf.data.size() == 0)
        return false;

    return LoadFromRawPCMWavData(&buf.data[0], buf.data.size(), buf.stereo, buf.is16Bit, buf.frequency);
}

bool AudioAsset::LoadFromRawPCMWavData(const u8 *data, size_t numBytes, bool stereo, bool is16Bit, int frequency)
{
    // Clean up the previous OpenAL audio buffer handle, if old data existed.
    DoUnload();

#ifndef TUNDRA_NO_AUDIO
    if (!data || numBytes == 0)
    {
        LogError("AudioAsset::LoadFromRawPCMWavData: Null data passed in!");
        return false;
    }

    if (!CreateBuffer())
        return false;

    ALenum openALFormat;
    if (stereo && is16Bit) openALFormat = AL_FORMAT_STEREO16;
    else if (!stereo && is16Bit) openALFormat = AL_FORMAT_MONO16;
    else if (stereo && !is16Bit) openALFormat = AL_FORMAT_STEREO8;
    else /* (!stereo && !is16Bit)*/ openALFormat = AL_FORMAT_MONO8;

    alBufferData(handle, openALFormat, (const ALvoid*)data, (ALsizei)numBytes, frequency);
    ALenum error = alGetError();
    if (error != AL_NONE)
    {
        const ALchar unknownError[] = "unknown error";
        const ALchar *errorString = alGetString(error);
        if (!errorString)
            errorString = unknownError;
        LogError("Could not set OpenAL sound buffer data: OpenAL error number " + QString::number(error) + ": " + errorString);
        DoUnload();
        return false;
    }
    return true;
#else
    return false;
#endif
}

bool AudioAsset::LoadFromSoundBuffer(const SoundBuffer &buffer)
{
    if (buffer.data.size() == 0)
    {
        LogError("AudioAsset::LoadFromSoundBuffer: Null data passed in!");
        return false;
    }

    return LoadFromRawPCMWavData(&buffer.data[0], buffer.data.size(), buffer.stereo, buffer.is16Bit, buffer.frequency);
}

bool AudioAsset::CreateBuffer()
{
#ifndef TUNDRA_NO_AUDIO
    if (!handle)
        alGenBuffers(1, &handle);

    if (!handle)
    {
        LogError("Could not create OpenAL sound buffer");
        return false;
    }

    return true;
#else
    return false;
#endif    
}

bool AudioAsset::IsLoaded() const
{
    return handle != 0;
}
