// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Settings_h
#define incl_MumbleVoipModule_Settings_h

#include "ui_VoiceSettings.h"
#include <QObject>
#include <QDebug>

namespace MumbleVoip
{
    /// Mumble voip system settings.
    /// - SettingsWidget class is provided to user to change the settings
    class Settings : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(double encode_quality READ GetEncodeQuality WRITE SetEncodeQuality NOTIFY EncodeQualityChanged )
        Q_PROPERTY(int playback_buffer_size_ms READ GetPlaybackBufferSizeMs WRITE SetPlaybackBufferSizeMs NOTIFY PlaybackBufferSizeMsChanged )
        Q_PROPERTY(double microphone_level READ GetMicrophoneLevel WRITE SetMicrophoneLevel NOTIFY MicrophoneLevelChanged )
        Q_PROPERTY(VoiceMode default_voice_mode READ GetDefaultVoiceMode WRITE SetDefaultVoiceMode NOTIFY DefaultVoiceModeChanged )
        Q_PROPERTY(bool positional_audio_enabled READ GetPositionalAudioEnabled WRITE SetPositionalAudioEnabled NOTIFY PositionalAudioEnabledChanged )

    public:
        enum VoiceMode { Mute, ContinuousTransmission, PushToTalk, ToggleMode };
        Settings();
        virtual ~Settings();
        virtual void Load();
        virtual void Save();

        double GetEncodeQuality() { return encode_quality_; }
        int GetPlaybackBufferSizeMs() { return playback_buffer_size_ms_; }
        double GetMicrophoneLevel() { return microphone_level_; }
        VoiceMode GetDefaultVoiceMode() { return default_voice_mode_; }
        bool GetPositionalAudioEnabled() { return positional_audio_enabled_; }

        void SetEncodeQuality(double encode_quality) { encode_quality_ = encode_quality; emit EncodeQualityChanged(encode_quality_); } 
        void SetPlaybackBufferSizeMs(int playback_buffer_size_ms) { playback_buffer_size_ms_ = playback_buffer_size_ms; emit PlaybackBufferSizeMsChanged(playback_buffer_size_ms_); } 
        void SetMicrophoneLevel(double microphone_level) { microphone_level_ = microphone_level; emit MicrophoneLevelChanged(microphone_level_); } 
        void SetDefaultVoiceMode(VoiceMode default_voice_mode) { default_voice_mode_ = default_voice_mode; } 
        void SetPositionalAudioEnabled(bool positional_audio_enabled) { positional_audio_enabled_ = positional_audio_enabled; emit PositionalAudioEnabledChanged(positional_audio_enabled_); } 

    signals:
        void EncodeQualityChanged(double);
        void PlaybackBufferSizeMsChanged(int);
        void MicrophoneLevelChanged(double);
        void DefaultVoiceModeChanged(VoiceMode);
        void PositionalAudioEnabledChanged(bool);

    private:
        double encode_quality_;
        int playback_buffer_size_ms_;
        double microphone_level_;
        VoiceMode default_voice_mode_;
        bool positional_audio_enabled_;

        static const char SETTINGS_HEADER_[];
    };

} // MumbleVoip



#endif // incl_MumbleVoipModule_Settings_h
