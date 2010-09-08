// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_SettingsWidget_h
#define incl_MumbleVoipModule_SettingsWidget_h

#include "ui_VoiceSettings.h"

namespace MumbleVoip
{
    class Settings;
    class Provider;

    /// Provide UI for Mumble Voip module settings. The widget is used with Settings service.
    class SettingsWidget : public QWidget, private Ui::VoiceSettings
    {
        Q_OBJECT
    public:
        SettingsWidget(Provider* provider, Settings* settings);
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
        virtual void UpdateMicrophoneLevel();
    private:
        Settings* settings_;
        Provider* provider_;
    };
} // MumbleVoip

#endif // incl_MumbleVoipModule_SettingsWidget_h
