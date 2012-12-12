// For conditions of distribution and use, see copyright notice in LICENSE
#pragma once

#include "TundraCoreApi.h"
#include "AudioFwd.h"
#include "AssetFwd.h"
#include "AudioAsset.h"
#include "SoundChannel.h"
#include "Math/float3.h"
#include "Math/Quat.h"

class Framework;

/// Enables audio playback functionality.
class TUNDRACORE_API AudioAPI : public QObject
{
    Q_OBJECT

public:
    /// The Audio API constructor initializes OpenAL audio using default device.
    /** @param fw Owner framework.
        @param assetAPI The Audio API depends on the asset API to be able to programmatically create audio record buffer assets. Pass in the audio API here. */
    AudioAPI(Framework *fw, AssetAPI *assetAPI);
    
    ~AudioAPI();
    
    /// (Re)initializes playback with specified device. Empty name uses default device.
    /** @param playbackDeviceName Playback device name
        @return true if successful */
    bool Initialize(const QString &playbackDeviceName = "");
    
    /// Uninitialize the Audio API
    void Reset();
    
    /// Open sound recording device & start recording
    /** @param name Device name, empty for default
        @param frequency Sound frequency
        @param sixteenBit Whether to use sixteen bit audio
        @param stereo Whether to use stereo
        @param bufferSize Buffer size in bytes. Should be multiple of sample size.
        @return true if successful */
    bool StartRecording(const QString &name, uint frequency, bool sixteenBit, bool stereo, uint bufferSize);
    
    /// Stop recording & close sound recording device
    void StopRecording();
    
    /// Get amount of sound currently in recording buffer, in bytes
    uint GetRecordedSoundSize() const;
    
    /// Get sound data from recording buffer
    /** @param buffer Buffer to receive data
        @param size How many bytes to receive
        @return Amount of bytes returned */
    uint GetRecordedSoundData(void* buffer, uint size);
    
    /// Update.
    /** Cleans up channels not playing anymore, and checks sound cache.
        This function is called from the core Framework. You should not call this manually. */
    void Update(f64 frametime);
    
    /// Returns initialized status
    bool IsInitialized() const;

    /// Saves sound settings to config.
    void SaveSoundSettingsToConfig();

    /// Loads and applies sound settings from config.
    void LoadSoundSettingsFromConfig();

public slots:
    /// Gets playback device names
    QStringList GetPlaybackDevices() const;
    
    /// Sets listener position & orientation
    /** @param position Position
        @param orientation Orientation as quaternion */
    void SetListener(const float3 &position, const Quat &orientation);

    /// Sets master gain of whole sound system
    /** @param masterGain New master gain, in range 0.0 - 1.0 */
    void SetMasterGain(float masterGain);

    /// Gets master gain of whole sound system
    float GetMasterGain() const;

    /// Sets master gain of certain sound types
    /** @param type Sound channel type to adjust
        @param masterGain New master gain, in range 0.0 - 1.0 */
    void SetSoundMasterGain(SoundChannel::SoundType type, float masterGain);
    
    /// Sets master gain of certain sound types
    float GetSoundMasterGain(SoundChannel::SoundType type) const;
    
    /// Plays non-positional sound
    /** @param audioAsset Asset pointer that has to be AudioAsset type.
        @param type Sound channel type, decides which master volume to use for the channel 
        @param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        @return Valid SoundChannel pointer, if successful (in case of loading from asset, actual sound may start later). Null SoundChannel pointer on failed play attempt. */
    SoundChannelPtr PlaySound(AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Plays positional sound. Returns sound id to adjust parameters
    /** @param position Position of sound
        @param audioAsset Asset pointer that has to be AudioAsset type.
        @param type Sound channel type, decides which master volume to use for the channel 
        @param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        @return Valid SoundChannel pointer, if successful (in case of loading from asset, actual sound may start later). Null SoundChannel pointer on failed play attempt. */
    SoundChannelPtr PlaySound3D(const float3 &position, AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Buffers sound data into a non-positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        @param buffer Sound buffer structure
        @param type Sound channel type, decides which master volume to use for the channel 
        @param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        @return Valid SoundChannel pointer if successful, otherwise null. */
    SoundChannelPtr PlaySoundBuffer(const SoundBuffer& buffer, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());
    
    /// Buffers sound data into a positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        @param buffer Sound buffer structure
        @param type Sound channel type, decides which master volume to use for the channel 
        @param position Position of sound
        @param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        @return Valid SoundChannel pointer if successful, otherwise null. */
    SoundChannelPtr PlaySoundBuffer3D(const SoundBuffer& buffer, SoundChannel::SoundType type = SoundChannel::Triggered, const float3 &position = float3::zero, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Gets all non-stopped channels id's
    std::vector<SoundChannelPtr> GetActiveSounds() const;

    /// Stops sound that's playing & destroys the channel
    /** @param id Channel id */
    void Stop(SoundChannelPtr channel) const;

    /// Get recording device names
    QStringList GetRecordingDevices() const;

    /// Create new audio asset directly from sound buffer.
    AudioAssetPtr CreateAudioAssetFromSoundBuffer(const SoundBuffer &buffer) const;

private:
    /// Uninitialize OpenAL sound
    void Uninitialize();

    /// Return next sound channel ID
    sound_id_t GetNextSoundChannelID() const;

    /// Reapply master gain to all existing channels
    void ApplyMasterGain();

    AssetAPI *assetAPI;

    struct AudioApiImpl;
    AudioApiImpl *impl;
};
