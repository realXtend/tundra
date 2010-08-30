// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    SettingsWidget::SettingsWidget() : QWidget()
    {
        InitializeUI();
        LoadInitialState();
    }

    void SettingsWidget::InitializeUI()
    {
        setupUi(this);
    }

    void SettingsWidget::LoadInitialState()
    {
        connect(this->testMicrophoneButton_2, SIGNAL(clicked()), this, SLOT(OpenMicrophoneAdjustmentWidget));
        QStringList items;
        items.append("Allways off");
        items.append("Allways On");
        items.append("Push-to-talk");
        items.append("Automatic");
        this->defaultVoiceMode_2->addItems(items);
    }

    void SettingsWidget::OpenMicrophoneAdjustmentWidget()
    {

    }

} // MumbleVoip
