// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TtsModule_SettingsWidget_h
#define incl_TtsModule_SettingsWidget_h

#include "ui_TTSSettings.h"

namespace Tts
{
    class SettingsWidget : public QWidget, private Ui::TTSSettings
    {
        Q_OBJECT
    public:
        SettingsWidget();
        virtual ~SettingsWidget();

        public slots:
        virtual void SaveSettings();
        virtual void LoadSettings();

    private slots:
        virtual void TestVoice();
    };
} // Tts

#endif // incl_TtsModule_SettingsWidget_h
