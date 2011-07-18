// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Framework.h"
#include "ConfigAPI.h"

#include "Settings.h"
#include <QSettings>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Settings::Settings(Framework *framework) :
        framework_(framework)
    {
        Load();
    }
    
    Settings::~Settings()
    {
        Save();
    }

    void Settings::Load()
    {
        if (!framework_)
            return;

        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, "voip");
        encode_quality_ = framework_->Config()->Get(configData, "encode quality", 0.5).toDouble();
        microphone_level_ = framework_->Config()->Get(configData, "microphone level", 0.5).toDouble();
        playback_buffer_size_ms_ = framework_->Config()->Get(configData, "playback buffer size", 200).toInt();
        default_voice_mode_ = (VoiceMode)framework_->Config()->Get(configData, "default voice mode", (int)Mute).toInt();
        positional_audio_enabled_ = framework_->Config()->Get(configData, "positional audio enabled", false).toBool();
    }

    void Settings::Save()
    {
        if (!framework_)
            return;

        ConfigData configData(ConfigAPI::FILE_FRAMEWORK, "voip");
        framework_->Config()->Set(configData, "encode quality", encode_quality_);
        framework_->Config()->Set(configData, "microphone level", microphone_level_);
        framework_->Config()->Set(configData, "playback buffer size", playback_buffer_size_ms_);
        framework_->Config()->Set(configData, "default voice mode", (int)default_voice_mode_);
        framework_->Config()->Set(configData, "positional audio enabled", positional_audio_enabled_);
    }

} // MumbleVoip
