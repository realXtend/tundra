// For conditions of distribution and use, see copyright notice in license.txt

#include "AudioAsset.h"
#include "LoggingFunctions.h"
#include "WavLoader.h"
#include "OggVorbisLoader.h"

#include <QString>
#include <AL/al.h>
#include <AL/alc.h>

DEFINE_POCO_LOGGING_FUNCTIONS("AudioAsset")

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
    if (handle)
    {
        alDeleteBuffers(1, &handle);
        handle = 0;
    }
}

bool AudioAsset::DeserializeFromData(const u8 *data, size_t numBytes)
{
    if (WavLoader::IdentifyWavFileInMemory(data, numBytes) && this->Name().endsWith(".wav", Qt::CaseInsensitive)) // Detect whether this file is Wav data or not.
        return LoadFromWavFileInMemory(data, numBytes);
   else if (this->Name().endsWith(".ogg", Qt::CaseInsensitive))
        return LoadFromOggVorbisFileInMemory(data, numBytes);
    else
        LogError("Unable to serialize audio asset data. Unknown format!");

    return false;
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

    if (!data || numBytes == 0)
    {
        LogError("Null data passed in AudioAsset::LoadFromWavData!");
        return false;
    }

    if (!CreateBuffer())
        return false;

    ALenum openALFormat;
    if (stereo && is16Bit) openALFormat = AL_FORMAT_STEREO16;
    else if (!stereo && is16Bit) openALFormat = AL_FORMAT_MONO16;
    else if (stereo && !is16Bit) openALFormat = AL_FORMAT_STEREO8;
    else /* (!stereo && !is16Bit)*/ openALFormat = AL_FORMAT_MONO8;

    // Copy the new data over.
    std::vector<u8> tmpData(data, data + numBytes);
    alBufferData(handle, openALFormat, &tmpData[0], tmpData.size(), frequency);
    ALenum error = alGetError();
    if (error != AL_NONE)
    {
        const ALchar unknownError[] = "unknown error";
        const ALchar *errorString = alGetString(error);
        if (!errorString)
            errorString = unknownError;
        LogError("Could not set OpenAL sound buffer data: OpenAL error number " + QString::number(error).toStdString() + ": " + errorString);
        DoUnload();
        return false;
    }
    return true;
}

bool AudioAsset::CreateBuffer()
{
    if (!handle)
        alGenBuffers(1, &handle);
    
    if (!handle)
    {
        LogError("Could not create OpenAL sound buffer");
        return false;
    } 
    
    return true;
}
