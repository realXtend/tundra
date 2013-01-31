// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IAsset.h"
#include "TundraCoreApi.h"
#include "AudioFwd.h"
#include "SoundBuffer.h"

/// Stores raw decoded audio data ready for playback.
class TUNDRACORE_API AudioAsset : public IAsset
{
    Q_OBJECT

public:
    AudioAsset(AssetAPI *owner, const QString &type_, const QString &name_);

    ~AudioAsset();

    virtual void DoUnload();

    virtual bool DeserializeFromData(const u8 *data, size_t numBytes, bool allowAsynchronous);

    /// Loads this audio asset from the given .wav file in memory.
    bool LoadFromWavFileInMemory(const u8 *data, size_t numBytes);

    /// Loads this audio asset from the given .ogg file in memory.
    bool LoadFromOggVorbisFileInMemory(const u8 *data, size_t numBytes);

    /// Loads this audio asset from the given raw PCM WAV data.
    /// @param data Contains the source data. This data is copied to internal AudioAsset memory, and does not need
    ///    to be stored in memory afterwards.
    /// @param numBytes The size of data, in bytes.
    /// @param stereo If true, data is treated as being stereo Wav data. Otherwise, it is treated as mono.
    /// @param is16Bit If true, data is treated as being 16-bits per sample. Otherwise, 8-bits per sample is assumed.
    /// @param frequency Specifies the number of samples per second in the input data buffer, e.g. 44100, or 48000.
    bool LoadFromRawPCMWavData(const u8 *data, size_t numBytes, bool stereo, bool is16Bit, int frequency);

    /// A convenience function to access LoadFromRawPCMWavData.
    bool LoadFromSoundBuffer(const SoundBuffer &buffer);

    /// Recreates the internal OpenAL audio buffer object, if it doesn't exist.
    /// Returns true on success, false otherwise.
    bool CreateBuffer();

    ALuint GetHandle() const { return handle; }

    bool IsLoaded() const;

private:
    /// The actual sound data is stored in an OpenAL internal audio buffer. This handle specifies the buffer.
    /// If == 0, then this AudioAsset is unloaded.
    ALuint handle;
};

