// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Settings.h"
#include <QSettings>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    const char Settings::SETTINGS_HEADER_[] = "Mumble Voip";

    Settings::Settings()
    {

    }
    
    Settings::~Settings()
    {

    }

    void Settings::Load()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        enabled = settings.value("enabled", true).toBool();
        encode_quality = settings.value("encode_quality", 0.5).toDouble();
        microphone_level = settings.value("microphone_level", 0.5).toDouble();
        playback_buffer_size_ms = settings.value("playback buffer size", 200).toInt();
        default_voice_mode = VoiceMode(settings.value("default voice mode", AllwaysOff).toInt());
    }

    void Settings::Save()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        settings.setValue("enabled", QVariant(enabled));
        settings.setValue("encode_quality",encode_quality);
        settings.setValue("microphone_level", microphone_level);
        settings.setValue("playback_buffer_size", playback_buffer_size_ms);
        settings.setValue("defaultvoice_mode", static_cast<int>(default_voice_mode));
        emit Changed();
    }

} // MumbleVoip
