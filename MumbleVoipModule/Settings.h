// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Settings_h
#define incl_MumbleVoipModule_Settings_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_VoiceSettings.h"
#include <QObject>

namespace MumbleVoip
{
    class Settings : public QObject
    {
        Q_OBJECT
        Q_PROPERTY(bool enabled READ GetEnabled() WRITE SetEnabled)
    public:
        enum VoiceMode { AllwaysOff, AllwaysOn, PushToTalk };
        Settings();
        virtual ~Settings();
        virtual void Load();
        virtual void Save();
        bool enabled;
        double encode_quality;
        double microphone_level;
        int playback_buffer_size_ms;
        VoiceMode default_voice_mode;
    signals:
        void Changed();
    private:
        bool GetEnabled() { return enabled; }
        void SetEnabled(bool value) { enabled = value; } 
        static const char SETTINGS_HEADER_[]; // = "Mumble Voip";
    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_Settings_h
