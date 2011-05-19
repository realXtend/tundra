// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Audio_Audio_h
#define incl_Audio_Audio_h

#include "AudioFwd.h"
#include "AssetFwd.h"
#include "AudioAsset.h"
#include "SoundChannel.h"
#include "AudioApiExports.h"
#include "Vector3D.h"
#include "Quaternion.h"

class Framework;

// Remove <Windows.h> PlaySound defines.
#ifdef PlaySound
#undef PlaySound
#endif

class AUDIO_API AudioAPI : public QObject
{
    Q_OBJECT;

public:
    /// The Audio API constructor initializes OpenAL audio using default device.
    /// @param assetAPI The Audio API depends on the asset API to be able to programmatically create audio record buffer assets. Pass in the audio API here.
    AudioAPI(Framework *fw, AssetAPI *assetAPI);
    
    ~AudioAPI();
    
    /// (Re)initializes playback with specified device. Empty name uses default device.
    /** \param playbackDeviceName Playback device name
        \return true if successful */
    bool Initialize(const QString &playbackDeviceName = "");
    
public slots:
    /// Gets playback device names
    QStringList GetPlaybackDevices();
    
    /// Sets listener position & orientation
    /** \param position Position
        \param orientation Orientation as quaternion */
    void SetListener(const Vector3df &position, const Quaternion &orientation);
    
    /// Sets master gain of whole sound system
    /** \param master_gain New master gain, in range 0.0 - 1.0 */
    void SetMasterGain(float master_gain);
    
    /// Sets master gain of certain sound types
    /** \param type Sound channel type to adjust
        \param master_gain New master gain, in range 0.0 - 1.0 */
    void SetSoundMasterGain(SoundChannel::SoundType type, float master_gain);
    
    /// Gets master gain of whole sound system
    float GetMasterGain();
    
    /// Sets master gain of certain sound types
    float GetSoundMasterGain(SoundChannel::SoundType type);        
    
    /// Plays non-positional sound
    /** \param name Sound file name or asset id
        \param local If true, name is interpreted as filename. Otherwise asset id
        \param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later) */
    SoundChannelPtr PlaySound(AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Plays positional sound. Returns sound id to adjust parameters
    /** \param name Sound file name or asset id
        \param local If true, name is interpreted as filename. Otherwise asset id
        \param position Position of sound
        \param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later) */
    SoundChannelPtr PlaySound3D(const Vector3df &position, AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Buffers sound data into a non-positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        \param buffer Sound buffer structure
        \param type Sound channel type, decides which mastervolume to use for the channel 
        \param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        \return nonzero channel id, if successful */
    SoundChannelPtr PlaySoundBuffer(const SoundBuffer& buffer, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());
    
    /// Buffers sound data into a positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        \param buffer Sound buffer structure
        \param type Sound channel type, decides which mastervolume to use for the channel 
        \param position Position of sound
        \param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making a new one.
        \return nonzero channel id, if successful */
    SoundChannelPtr PlaySoundBuffer3D(const SoundBuffer& buffer, SoundChannel::SoundType type = SoundChannel::Triggered, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Gets all non-stopped channels id's
    std::vector<SoundChannelPtr> GetActiveSounds() const;
    
    /// Stops sound that's playing & destroys the channel
    /** \param id Channel id */
    void Stop(SoundChannelPtr channel);
    
    /// Get recording device names
    QStringList GetRecordingDevices();

    AudioAssetPtr CreateAudioAssetFromSoundBuffer(const SoundBuffer &buffer);

public:
    
    /// Open sound recording device & start recording
    /** \param name Device name, empty for default
        \param frequency Sound frequency
        \param sixteenbit Whether to use sixteen bit audio
        \param stereo Whether to use stereo
        \param buffer_size Buffer size in bytes. Should be multiple of sample size.
        \return true if successful */
    bool StartRecording(const QString &name, uint frequency, bool sixteenbit, bool stereo, uint buffer_size);
    
    /// Stop recording & close sound recording device
    void StopRecording();
    
    /// Get amount of sound currently in recording buffer, in bytes
    uint GetRecordedSoundSize();
    
    /// Get sound data from recording buffer
    /** \param buffer Buffer to receive data
        \param size How many bytes to receive
        \return Amount of bytes returned */
    uint GetRecordedSoundData(void* buffer, uint size);
    
    /// Update. Cleans up channels not playing anymore, and checks sound cache. This function is called from the core Framework. You should not call this manually.
    void Update(f64 frametime);
    
    /// Returns initialized status
    bool IsInitialized() const;

private:
    /// Uninitialize OpenAL sound
    void Uninitialize();
    
    /// Return next sound channel ID
    sound_id_t GetNextSoundChannelID();
    
    /// Reapply master gain to all existing channels
    void ApplyMasterGain();
    
    AssetAPI *assetAPI;

    AudioApiImpl *impl;
};

#endif
