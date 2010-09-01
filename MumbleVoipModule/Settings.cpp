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

    }
    
    Settings::~Settings()
    {

    }

    void Settings::Load()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        enabled = settings.value("MumbleVoice/enabled", true).toBool();
        encode_quality = settings.value("MumbleVoice/encode_quality", 0.5).toDouble();
        microphone_level = settings.value("MumbleVoice/microphone_level", 0.5).toDouble();
        playback_buffer_size_ms = settings.value("MumbleVoice/playback_buffer_size", 200).toInt();
        default_voice_mode = VoiceMode(settings.value("MumbleVoice/default_voice_mode", AllwaysOff).toInt());
    }

    void Settings::Save()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        settings.setValue("MumbleVoice/enabled", QVariant(enabled));
        settings.setValue("MumbleVoice/encode_quality",encode_quality);
        settings.setValue("MumbleVoice/microphone_level", microphone_level);
        settings.setValue("MumbleVoice/playback_buffer_size", playback_buffer_size_ms);
        settings.setValue("MumbleVoice/default_voice_mode", static_cast<int>(default_voice_mode));
        settings.sync();
        emit Changed();
    }

} // MumbleVoip
