// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Settings.h"
#include <QSettings>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    const char Settings::SETTINGS_HEADER_[] = "configuration/MumbleVoip";

    Settings::Settings()
    {
        Load();
    }
    
    Settings::~Settings()
    {
        Save();
    }

    void Settings::Load()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        encode_quality_ = settings.value("MumbleVoice/encode_quality", 0.5).toDouble();
        microphone_level_ = settings.value("MumbleVoice/microphone_level", 0.5).toDouble();
        playback_buffer_size_ms_ = settings.value("MumbleVoice/playback_buffer_size", 200).toInt();
        default_voice_mode_ = VoiceMode(settings.value("MumbleVoice/default_voice_mode", Mute).toInt());
        positional_audio_enabled_ = settings.value("MumbleVoice/positional_audio_enabled", true).toBool();
    }

    void Settings::Save()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        settings.setValue("MumbleVoice/encode_quality",encode_quality_);
        settings.setValue("MumbleVoice/microphone_level", microphone_level_);
        settings.setValue("MumbleVoice/playback_buffer_size", playback_buffer_size_ms_);
        settings.setValue("MumbleVoice/default_voice_mode", static_cast<int>(default_voice_mode_));
        settings.setValue("MumbleVoice/positional_audio_enabled", QVariant(positional_audio_enabled_));
        settings.sync();
    }

} // MumbleVoip
