/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceTransmissionModeWidget.cpp
 *  @brief  Widget for in-wolrd voice transmission mode selection
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "VoiceTransmissionModeWidget.h"
#include "CommunicationsService.h"
#include "QSettings.h"

#include "DebugOperatorNew.h"

namespace CommUI
{
    VoiceTransmissionModeWidget::VoiceTransmissionModeWidget(int initial_mode)
    {
        setupUi(this);
        switch(initial_mode)
        {
        case 0:
            offButton->setChecked(true);
            break;
        case 1:
            continousTransmissionButton->setChecked(true);
            break;
        case 2:
            pushToTalkButton->setChecked(true);
            break;
        case 3:
            toggleModeButton->setChecked(true);
            break;
        }
        connect(offButton, SIGNAL(clicked()), this, SLOT(DefineTransmissionMode()));
        connect(continousTransmissionButton, SIGNAL(clicked()), this, SLOT(DefineTransmissionMode()));
        connect(pushToTalkButton, SIGNAL(clicked()), this, SLOT(DefineTransmissionMode()));
        connect(toggleModeButton, SIGNAL(clicked()), this, SLOT(DefineTransmissionMode()));
    }

    void VoiceTransmissionModeWidget::DefineTransmissionMode()
    {
        if (offButton->isChecked())
            emit TransmissionModeSelected(0);

        if (continousTransmissionButton->isChecked())
            emit TransmissionModeSelected(1);

        if (pushToTalkButton->isChecked())
            emit TransmissionModeSelected(2);

        if (toggleModeButton->isChecked())
            emit TransmissionModeSelected(3);
        hide();
    }

} // CommUI
