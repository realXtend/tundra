// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_SettingsWidget_h
#define incl_MumbleVoipModule_SettingsWidget_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_VoiceSettings.h"

namespace MumbleVoip
{
    class Settings;

    class SettingsWidget : public QWidget, private Ui::VoiceSettings
    {
        Q_OBJECT
    public:
        SettingsWidget(Settings* settings);
        virtual ~SettingsWidget();

    private:
        virtual void InitializeUI();
        virtual void LoadInitialState();
    private slots:
        virtual void OpenMicrophoneAdjustmentWidget();
        virtual void SaveSettings();
        virtual void UpdateUI();
    private:
        Settings* settings_;

    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_SettingsWidget_h
