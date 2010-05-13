// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VoiceUsersWidget.h"
#include <CommunicationsService.h>

namespace CommUI
{
    VoiceUserWidget::VoiceUserWidget(Communications::InWorldVoice::ParticipantInterface* participant)
        : QWidget(), participant_(participant) 
    {
        setupUi(this);
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
        {
            thumbnailWidget->setStyleSheet("#thumbnailWidget {border: 0px; background-color: rgbs(0,0,0,0); background-image: url('./data/ui/images/comm/user_green.png'); background-position: top left; background-repeat: no-repeat;}");
        }
        else
        {
            thumbnailWidget->setStyleSheet("#thumbnailWidget {border: 0px; background-color: rgbs(0,0,0,0); background-image: url('./data/ui/images/comm/user.png'); background-position: top left; background-repeat: no-repeat;}");
        }
    }

    VoiceUsersWidget::VoiceUsersWidget(QWidget *parent, Qt::WindowFlags wFlags)
        : QWidget(parent, wFlags),
          session_(0)
    {
        setupUi(this);
        connect(hideButton, SIGNAL(clicked()), SLOT(hide()) );
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
                break;
            }
        }
        int area_height = 0;
        int number = user_widgets_.size();
        if (number > PARTICIPANTS_SHOWN_MAX_)
            number = PARTICIPANTS_SHOWN_MAX_;
        if (number == 0)
            area_height = 0;
        else
            area_height = number*user_widgets_[0]->height() + verticalLayoutArea->spacing()*(number-1) + verticalLayoutArea->contentsMargins().bottom() + verticalLayoutArea->contentsMargins().top();

        const QRect& geometry = userListScrollArea->geometry();
        userListScrollArea->setGeometry(geometry.x(), geometry.y(), geometry.width(), area_height);

        titleLabel->setText( QString("Voice Users (%1)").arg(session_->Participants().size()) );
    }

    void VoiceUsersWidget::mouseMoveEvent(QMouseEvent *e)
    {
        QPoint pos = e->globalPos();
        QPoint move = pos - mouse_last_pos_;
        this->move(this->pos() +  move);
        mouse_last_pos_ = pos;
        QWidget::mouseMoveEvent(e);
    }

    void VoiceUsersWidget::mousePressEvent(QMouseEvent *e)
    {
        mouse_last_pos_ = e->globalPos();
        mouse_dragging_ = true;
    }

    void VoiceUsersWidget::mouseReleaseEvent(QMouseEvent *e)
    {
        mouse_dragging_ = false;
    }

} // CommUI
