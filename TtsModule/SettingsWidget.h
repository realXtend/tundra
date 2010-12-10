// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TtsModule_SettingsWidget_h
#define incl_TtsModule_SettingsWidget_h

#include "ui_TTSSettings.h"

namespace Foundation
{
    class Framework;
}

namespace Tts
{
    class Settings
    {
    public:
        virtual void Load();
        virtual void Save();

    public:
        QString testVoice;
        QString ownVoice;
        QString otherDefaultVoice;
        QString notificationVoice;
        bool playOwnChatMessages;
        bool playOtherChatMessages;
        bool playNotificationMessages;
        QString testPhrase;
        static const char SETTINGS_HEADER_[];
    };

    class SettingsWidget : public QWidget, private Ui::TTSSettings
    {
        Q_OBJECT
    public:
        SettingsWidget(Foundation::Framework* framework);
        virtual ~SettingsWidget();

        public slots:
        virtual void SaveSettings();
        virtual void LoadSettings();

    private slots:
        virtual void TestVoice();
        virtual void UpdateVoiceOptions();
    private:
        Settings settings_;
        Foundation::Framework* framework_;
    };
} // Tts

#endif // incl_TtsModule_SettingsWidget_h
