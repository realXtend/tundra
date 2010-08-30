// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_SettingsWidget_h
#define incl_MumbleVoipModule_SettingsWidget_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_VoiceSettings.h"


namespace MumbleVoip
{
    class Settings
    {
    public:
        bool enabled;
        double voice_quality;
        double microphone_level;
        int playback_buffer_size_ms;
        QString default_voice_mode;
    };

    class SettingsWidget : public QWidget, private Ui::VoiceSettings
    {
        Q_OBJECT
    public:
        SettingsWidget();
        virtual ~SettingsWidget() {};

    private:
        virtual void InitializeUI();
        virtual void LoadInitialState();
    private slots:
        virtual void OpenMicrophoneAdjustmentWidget();

    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_SettingsWidget_h
