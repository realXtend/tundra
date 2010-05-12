// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VoiceUsersWidget.h"
#include <CommunicationsService.h>

namespace CommUI
{
    VoiceUserWidget::VoiceUserWidget(Communications::InWorldVoice::ParticipantInterface* participant)
        : QWidget(), participant_(participant) 
    {
//        setObjectName("VoiceUserWidget");
        setupUi(this);
		setWindowTitle("Voice Users");
        avatarNameLabel->setText(participant->Name());
        connect(participant_, SIGNAL(StartSpeaking()), SLOT(UpdateStyleSheet()));
        connect(participant_, SIGNAL(StopSpeaking()), SLOT(UpdateStyleSheet()));
        connect(muteCheckBox, SIGNAL(clicked()), SLOT(ToggleMute()) );
        UpdateStyleSheet();
    }

    void VoiceUserWidget::ToggleMute()
    {
        if (participant_->IsMuted())
            participant_->Mute(false);
        else
            participant_->Mute(true);
    }

    Communications::InWorldVoice::ParticipantInterface* VoiceUserWidget::Participant() const
    {
        return participant_;
    }

    void VoiceUserWidget::UpdateStyleSheet()
    {
		QString t = styleSheet();
        if (participant_->IsSpeaking())
            avatarNameLabel->setStyleSheet("QLabel#avatarNameLabel { color: rgb(255,0,0);}");
        else
            avatarNameLabel->setStyleSheet("QLabel#avatarNameLabel { color: rgb(255,255,255);}");
    }


    VoiceUsersWidget::VoiceUsersWidget(QWidget *parent, Qt::WindowFlags wFlags)
        : QWidget(parent, wFlags),
          session_(0)
    {
        setupUi(this);
//        setObjectName("Voice users");

//        userListLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }

    VoiceUsersWidget::~VoiceUsersWidget()
    {
        foreach(VoiceUserWidget* widget, user_widgets_)
        {
            userListLayout->removeWidget(widget);
            SAFE_DELETE(widget);
        }
        user_widgets_.clear();
    }

    void VoiceUsersWidget::SetSession(Communications::InWorldVoice::SessionInterface* session)
    {
        session_ = session;
        if (session_)
        {
            connect(session_, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateList()) );
            connect(session_, SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateList()) );
            connect(session_, SIGNAL(StateChanged(Communications::InWorldVoice::SessionInterface::State)), SLOT(UpdateList()) );
        }
    }

    void VoiceUsersWidget::UpdateList()
    {
        if (!session_)
            return;

        QList<Communications::InWorldVoice::ParticipantInterface*> list = session_->Participants();
        foreach(Communications::InWorldVoice::ParticipantInterface* p, list)
        {
            bool widget_exist = false;
            foreach(VoiceUserWidget* w, user_widgets_)
            {
                if (w->Participant() == p)
                {
                    widget_exist = true;
                    break;
                }
            }
            if (!widget_exist)
            {
                VoiceUserWidget* w = new VoiceUserWidget(p);
                user_widgets_.append(w);
                userListLayout->insertWidget(0,w);
            }
        }
        //! @todo remove widgets...
        foreach(VoiceUserWidget* widget, user_widgets_)
        {
            bool participant_exist = false;
            foreach(Communications::InWorldVoice::ParticipantInterface* p, list)
            {
                if (widget->Participant() == p)
                {
                    participant_exist = true;
                    break;
                }
            }
            if (!participant_exist)
            {
                user_widgets_.removeOne(widget);
                userListLayout->removeWidget(widget);
                SAFE_DELETE(widget);
                return;
            }
        }
    }

} // CommUI
