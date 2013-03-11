// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "AudioFwd.h"
#include "AudioAsset.h"

#include "Math/float3.h"
#include "AssetFwd.h"

/// An OpenAL sound channel (source).
class TUNDRACORE_API SoundChannel : public QObject, public enable_shared_from_this<SoundChannel>
{
    Q_OBJECT
    Q_ENUMS(SoundState)
    Q_ENUMS(SoundType)

    Q_PROPERTY(uint channelId READ ChannelId)
    Q_PROPERTY(SoundState soundState READ State)
    Q_PROPERTY(SoundType soundType READ Type)
    Q_PROPERTY(QString soundName READ SoundName)

    Q_PROPERTY(bool looped READ IsLooped WRITE SetLooped)
    Q_PROPERTY(bool positional READ IsPositional WRITE SetPositional)
    Q_PROPERTY(float3 position READ Position WRITE SetPosition)
    Q_PROPERTY(float pitch READ Pitch WRITE SetPitch)
    Q_PROPERTY(float gain READ Gain WRITE SetGain)
    Q_PROPERTY(float masterGain READ MasterGain WRITE SetMasterGain)

public:
    /// States of sound channels
    enum SoundState
    {
        Stopped = 0,
        Pending,
        Playing
    };
   
    /// Types of sound channels, for adjusting master volume individually
    enum SoundType
    {
        Triggered = 0,
        Ambient,
        Voice
    };

    SoundChannel(sound_id_t channelId, SoundType type);
    ~SoundChannel();
    
public slots:
    /// Start playing sound. Set to pending state if sound is actually not loaded yet
    void Play(AudioAssetPtr audioAsset);

    /// Stop playing sound.
    void Stop();

    /// Add a sound buffer and play.
    /** @note After a sound buffer is added, channel will remain in pending state even if there is no 
        more sound data for the moment. This is to ensure that the sound system will not automatically
        dispose of the channel. */ 
    void AddBuffer(AudioAssetPtr buffer);

    /// Adjusts range parameters of positional sound channel.
    /** Between radiuses, attenuation will be interpolated and raised to power of roll-off
        If outer radius is 0, there will be no attenuation (sound is always played at gain)
        Also, for non-positional channels the range parameters have no effect.
        @param innerRadius Within inner radius, sound will be played at gain
        @param outerRadius Outside outer radius, sound will be silent
        @param rollOff Roll-off power factor. 1.0 = linear, 2.0 = distance squared */
    void SetRange(float innerRadius, float outerRadius, float rollOff);

public:
    /// Per-frame update with new listener position
    void Update(const float3& listenerPos);

    /// Return current state of channel.
    SoundState State() const { return state_; }

    /// Gets type of sound played/pending on channel (triggered/ambient etc.)
    SoundType Type() const { return type_; }

    /// Return name/id of sound that's playing, empty if nothing playing
    QString SoundName() const;

    /// Gets the channel id.
    sound_id_t ChannelId() const { return channelId; }

    /// Adjusts positional status of channel
    /** @param positional Positional status */
    void SetPositional(bool enable);

    /// Get if channel is positional.
    bool IsPositional() const { return positional_; }

    /// Adjusts looping status of channel
    /** @param looped Whether to loop */
    void SetLooped(bool enable);

    /// Get if channel is looped.
    bool IsLooped() const { return looped_; }

    /// Set position
    void SetPosition(const float3& pos);

    /// Get position
    float3 Position() const { return position_; }

    /// Adjusts pitch of channel
    /** @param pitch Pitch relative to sound's original pitch (1.0 = original) */
    void SetPitch(float pitch);

    /// Get sound channel pitch.
    float Pitch() const { return pitch_; }

    /// Adjusts gain of channel.
    /** @param gain New gain value, 1.0 = full volume, 0.0 = silence */
    void SetGain(float gain);

    /// Get gain of channel.
    float Gain() const { return gain_; }

    /// Sets master gain.
    /** Final sound volume = gain * master gain * possible distance attenuation.
        @param gain New master gain value [0.0, 1.0]. */
    void SetMasterGain(float masterGain);

    /// Get master gain.
    float MasterGain() const { return master_gain_; }

private:
    /// Queue buffers and start playing
    void QueueBuffers();
    /// Remove processed buffers
    void UnqueueBuffers();
    /// Create OpenAL source if one does not exist yet
    bool CreateSource();
    /// Delete OpenAL source
    void DeleteSource();
    /// Calculate attenuation from position, listener position & range parameters
    void CalculateAttenuation(const float3 &listener_pos);
    /// Set positionality & position
    void SetPositionAndMode();
    /// Set gain, taking attenuation into account
    void SetAttenuatedGain();
    
    /// Sound type
    SoundType type_;
    /// OpenAL handle
    ALuint handle_;
    /// Sounds buffers pending to be played
    std::list<AudioAssetPtr> pending_sounds_;
    /// Currently playing sound buffers
    std::vector<AudioAssetPtr> playing_sounds_;
    /// Pitch
    float pitch_;
    /// Gain
    float gain_;
    /// Master gain. Final sound volume = gain * master gain * possible distance attenuation
    float master_gain_;
    /// Inner radius
    float inner_radius_;
    /// Outer radius
    float outer_radius_;
    /// Roll-off power factor
    float rolloff_;
    /// Last calculated attenuation factor
    float attenuation_;
    /// Looped flag
    bool looped_;
    /// Positional flag
    bool positional_;
    /// Buffered operation flag. Will never report as stopped, unless explicitly stopped
    bool buffered_mode_;
    /// Position
    float3 position_;
    /// State 
    SoundState state_;
    /// Specifies an unique ID for this sound channel. Note that this ID should not be treated as a "channel index" or anything like that.
    sound_id_t channelId;
};

typedef shared_ptr<SoundChannel> SoundChannelPtr;
