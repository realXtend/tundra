// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
#include <QSettings>
#include "MemoryLeakCheck.h"


namespace Tts
{
    SettingsWidget::SettingsWidget():  QWidget()
    {
        setupUi(this);
        LoadSettings();
        connect(testButton, SIGNAL(clicked()), this, SLOT(TestVoice()) );
    }

    SettingsWidget::~SettingsWidget()
    {
    }

    void SettingsWidget::SaveSettings()
    {
        // TODO: IMPLEMENT
    }

    void SettingsWidget::LoadSettings()
    {
        // TODO: IMPLEMENT
    }
        
    void SettingsWidget::TestVoice()
    {
        QString text = testText->displayText();
        // TODO: IMPLEMENT
    }

} // Tts
