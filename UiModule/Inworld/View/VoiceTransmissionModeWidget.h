/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceUsersInfoWidget.h
 *  @brief  Widget for voice user list information
 *          
 */

#ifndef incl_UiModule_VoiceTransmissionModeWidget_h
#define incl_UiModule_VoiceTransmissionModeWidget_h

#include <QWidget>
#include "ui_VoiceTransmissionMode.h"

namespace Foundation
{
    class Framework;
}

namespace Communications
{
    namespace InWorldVoice
    {
        class SessionInterface;
        class ParticipantInterface;
    }
}

namespace CommUI
{
    //! user can select transmission mode for in-world voice
    class VoiceTransmissionModeWidget : public QWidget, private Ui::TransmissionMode
    {
        Q_OBJECT
    public:
        VoiceTransmissionModeWidget(int initial_mode);

    signals:
        void TransmissionModeSelected(int mode);

    private slots:
        void DefineTransmissionMode();
    //    void OnOffButtonPressed(bool selected);
    //    void OnContinousTransmissionButtonPressed(bool selected);
    //    void OnPushToTalkButtonPressed(bool selected);
    //    void OnToggleModeButtonPressed(bool selected);

    //    Communications::InWorldVoice::SessionInterface* voice_session_;
    };

} // namespace CommUI

#endif // incl_UiModule_VoiceTransmissionModeWidget_h
