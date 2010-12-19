// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Audio_Audio_h
#define incl_Audio_Audio_h

#include "AudioFwd.h"
#include "AudioAsset.h"
#include "SoundChannel.h"
#include "AudioApiExports.h"
#include "Vector3D.h"
#include "Quaternion.h"
  
/// Sound service implementation. Owned by OpenALAudioModule.
class AUDIO_API AudioAPI : public QObject
{
    Q_OBJECT;

public:
    /// The Audio API constructor initializes OpenAL audio using default device.
    AudioAPI();
    
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
        \param existingChannel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
        \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later) */
    SoundChannelPtr PlaySound(AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Plays positional sound. Returns sound id to adjust parameters
    /** \param name Sound file name or asset id
        \param local If true, name is interpreted as filename. Otherwise asset id
        \param position Position of sound
        \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
        \return nonzero channel id, if successful (in case of loading from asset, actual sound may start later) */
    SoundChannelPtr PlaySound3D(const Vector3df &position, AssetPtr audioAsset, SoundChannel::SoundType type = SoundChannel::Triggered, SoundChannelPtr existingChannel = SoundChannelPtr());

    /// Buffers sound data into a non-positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        \param buffer Sound buffer structure
        \param type Sound channel type, decides which mastervolume to use for the channel 
        \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
        \return nonzero channel id, if successful */
//    SoundChannelPtr PlaySoundBuffer(const SoundBuffer& buffer, SoundChannel::SoundType type = Triggered, sound_id_t channel = 0);
    
    /// Buffers sound data into a positional channel
    /** Note: use the returned channel id for continuing to feed the sound stream.
        Call StopSound() with channel id to free the channel, when done.
        \param buffer Sound buffer structure
        \param type Sound channel type, decides which mastervolume to use for the channel 
        \param position Position of sound
        \param channel Channel id. If non-zero, and is a valid channel, will use that channel instead of making new
        \return nonzero channel id, if successful */
//    SoundChannelPtr PlaySoundBuffer3D(const SoundBuffer& buffer, SoundChannel::SoundType type = Triggered, Vector3df position = Vector3df(0.0f, 0.0f, 0.0f), sound_id_t channel = 0);

    /// Gets state of channel
    /** \param id Channel id
        \return Current state (stopped, pending & loading sound asset, playing) */
//    SoundState GetSoundState(sound_id_t id) const;
    
    /// Gets all non-stopped channels id's
    std::vector<SoundChannelPtr> GetActiveSounds() const;
    
    /// Gets name of sound played/pending on channel
    /** \param id Channel id
        \return Sound name, or empty if no sound */
//    QString GetSoundName(sound_id_t id) const;
 
    /// Gets type of sound played/pending on channel (triggered/ambient etc.)
    /** \param id Channel id
        \return Sound type */
//    SoundChannel::SoundType GetSoundType(sound_id_t id) const;
    
    /// Stops sound that's playing & destroys the channel
    /** \param id Channel id */
    void Stop(SoundChannelPtr channel);
    
    /// Adjusts pitch of channel
    /** \param id Channel id
        \param pitch Pitch relative to sound's original pitch (1.0 = original) */
//    void SetPitch(sound_id_t id, float pitch);

    /// Get sound channel pitch.
    /** \param id Channel id
        \return Channel's pitch value. */
//    float GetPitch(sound_id_t id) const;
    
    /// Adjusts gain of channel
    /** \param id Channel id
        \param gain New gain value, 1.0 = full volume, 0.0 = silence */
//    void SetGain(sound_id_t id, float gain);
    
    /// Get gain of channel. If channel wasn't found return -1.
    /** \param id Channel id
     *  \return Channel's gain. */
//    float GetGain(sound_id_t id) const; 
    
    /// Adjusts looping status of channel
    /** \param id Channel id
        \param looped Whether to loop */
//    void SetLooped(sound_id_t id, bool looped);
    
    /// Adjusts positional status of channel
    /** \param id Channel id
        \param positional Positional status */
//    void SetPositional(sound_id_t id, bool positional);
    
    /// Sets position of channel
    /** \param id Channel id
        \param position New position */
//    void SetPosition(sound_id_t id, Vector3df position);
    
    /// Adjusts range parameters of positional sound channel.
    /** \param id Channel id
        \param inner_radius Within inner radius, sound will be played at gain
        \param outer_radius Outside outer radius, sound will be silent
        \param rolloff Rolloff power factor. 1.0 = linear, 2.0 = distance squared 
        Between radiuses, attenuation will be interpolated and raised to power of rolloff
        If outer_radius is 0, there will be no attenuation (sound is always played at gain)
        Also, for non-positional channels the range parameters have no effect. */
//    void SetRange(sound_id_t id, float inner_radius, float outer_radius, float rolloff);

    /// Get recording device names
    QStringList GetRecordingDevices();

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
    
    /// Get sound
    /** Creates new if necessary. Initiates asset decode/download as necessary. */
//    SoundPtr GetSound(const std::string& name, bool local);
    /// Posts request for local decode of ogg file
    /* \return true if file could be found & decode initiated. This does not yet tell if the data is valid, though */
 //   bool DecodeLocalOggFile(Sound* sound, const std::string& name);
    
    AudioApiImpl *impl;
};

#endif
