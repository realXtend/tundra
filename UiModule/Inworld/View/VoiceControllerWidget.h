// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_VoiceControllerWidget_h
#define incl_UiModule_VoiceControllerWidget_h

#include <QObject>
#include <QWidget>
#include "ui_VoiceControl.h"
#include "VoiceController.h"

namespace Communications
{
    namespace InWorldVoice
    {
        class SessionInterface;
    }
}

namespace CommUI
{
    class VoiceControllerWidget : public QWidget, private Ui::VoiceControl
    {
        Q_OBJECT
    public:
        VoiceControllerWidget(Communications::InWorldVoice::SessionInterface* voice_session);
        virtual ~VoiceControllerWidget();

    public slots:
        virtual void SetPushToTalkOn();
        virtual void SetPushToTalkOff();
        virtual void Toggle();

    private slots:
        void ApplyTransmissionModeSelection(int selection);
        void OpenParticipantListWidget();
        void ApplyMuteAllSelection();
        void UpdateUI();
        void UpdateParticipantList();

    private:
        QGraphicsProxyWidget* voice_users_proxy_widget_;
        VoiceController voice_controller_;
        QList<VoiceUserWidget *> user_widgets_;
    };
} // namespace CommUI

#endif // incl_UiModule_VoiceControllerWidget_h
