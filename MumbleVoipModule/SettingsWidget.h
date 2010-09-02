// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_SettingsWidget_h
#define incl_MumbleVoipModule_SettingsWidget_h

//#include "CommunicationsService.h"
//#include <QGraphicsProxyWidget>
#include "ui_VoiceSettings.h"

namespace MumbleVoip
{
    class Settings;

    /// Provide UI for Mumble Voip module settings. The widget is used with Settings service.
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
        virtual void ApplyEncodeQuality();
        virtual void ApplyPlaybackBufferSize();
        virtual void ApplyMicrophoneLevel();
        virtual void ApplyChanges();
        virtual void UpdateUI();
    private:
        Settings* settings_;

    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_SettingsWidget_h
