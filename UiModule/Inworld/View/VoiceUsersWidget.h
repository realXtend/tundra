// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_VoiceUsersWidget_h
#define incl_UiModule_VoiceUsersWidget_h

//#include <QGraphicsProxyWidget>
#include <QWidget>
#include "ui_VoiceUsers.h"
#include "ui_VoiceUser.h"

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
    //! Presents InWorldVoice::Participant object
    class VoiceUserWidget : public QWidget, private Ui::voiceUserWidget
    {
        Q_OBJECT
    public:
        VoiceUserWidget(Communications::InWorldVoice::ParticipantInterface* participant);
        Communications::InWorldVoice::ParticipantInterface* Participant() const;
    private slots:
        void UpdateStyleSheet();
        void ToggleMute();
    private:
        Communications::InWorldVoice::ParticipantInterface* participant_;
    };

    //! VoiceUserListWidget
    class VoiceUsersWidget : public QWidget, private Ui::VoiceUsersWidget
    {
        Q_OBJECT
    public:
        VoiceUsersWidget(QWidget *parent = 0, Qt::WindowFlags wFlags = 0);
        virtual ~VoiceUsersWidget();
        virtual void SetSession(Communications::InWorldVoice::SessionInterface* session);
    public slots:
        void UpdateList();
    private:
        Communications::InWorldVoice::SessionInterface* session_;
        QList<VoiceUserWidget *> user_widgets_;
    };

} // CommUI

#endif // incl_UiModule_VoiceUsersWidget_h
