// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Settings_h
#define incl_MumbleVoipModule_Settings_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_VoiceSettings.h"
#include <QObject>
#include <QDebug>

namespace MumbleVoip
{
    /// Mumble voip system settings.
    /// - SettingsWidget is provided to user to change the settings
    /// - Session class will use settings and react on changes at runtime if possible.
    class Settings : public QObject
    {
        Q_OBJECT

        Q_PROPERTY(bool enabled READ GetEnabled WRITE SetEnabled NOTIFY EnabledChanged )
        Q_PROPERTY(double encode_quality READ GetEncodeQuality WRITE SetEncodeQuality NOTIFY EncodeQualityChanged )
        Q_PROPERTY(int playback_buffer_size_ms READ GetPlaybackBufferSizeMs WRITE SetPlaybackBufferSizeMs NOTIFY PlaybackBufferSizeMsChanged )
        Q_PROPERTY(double microphone_level READ GetMicrophoneLevel WRITE SetMicrophoneLevel NOTIFY MicrophoneLevelChanged )
        Q_PROPERTY(VoiceMode default_voice_mode READ GetDefaultVoiceMode WRITE SetDefaultVoiceMode NOTIFY DefaultVoiceModeChanged )

    public:
        enum VoiceMode { AllwaysOff, AllwaysOn, PushToTalk };
        Settings();
        virtual ~Settings();
        virtual void Load();
        virtual void Save();

        bool GetEnabled() { return enabled_; }
        double GetEncodeQuality() { return encode_quality_; }
        int GetPlaybackBufferSizeMs() { return playback_buffer_size_ms_; }
        double GetMicrophoneLevel() { return microphone_level_; }
        VoiceMode GetDefaultVoiceMode() { return default_voice_mode_; }

        void SetEnabled(bool enabled) { enabled_ = enabled; } 
        void SetEncodeQuality(double encode_quality) { encode_quality_ = encode_quality; emit EncodeQualityChanged(encode_quality_); } 
        void SetPlaybackBufferSizeMs(int playback_buffer_size_ms) { playback_buffer_size_ms_ = playback_buffer_size_ms; emit PlaybackBufferSizeMsChanged(playback_buffer_size_ms_); } 
        void SetMicrophoneLevel(double microphone_level) { microphone_level_ = microphone_level; } 
        void SetDefaultVoiceMode(VoiceMode default_voice_mode) { default_voice_mode_ = default_voice_mode; } 

    signals:
        void EnabledChanged(bool);
        void EncodeQualityChanged(double);
        void PlaybackBufferSizeMsChanged(int);
        void MicrophoneLevelChanged(double);
        void DefaultVoiceModeChanged(VoiceMode);
    private:
        bool enabled_;
        double encode_quality_;
        int playback_buffer_size_ms_;
        double microphone_level_;
        VoiceMode default_voice_mode_;

        static const char SETTINGS_HEADER_[]; // = "Mumble Voip";
    };

} // MumbleVoip



#endif // incl_MumbleVoipModule_Settings_h
