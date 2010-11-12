/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceController.cpp
 *  @brief  In-world voice controller user interface
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "VoiceControllerWidget.h"
#include "CommunicationsService.h"
#include "VoiceUsersWidget.h" /// @todo Separate to VoiceParticipantWidget.h/cpp

#include "DebugOperatorNew.h"

#include <QSettings>

namespace CommUI
{

    VoiceControllerWidget::VoiceControllerWidget(Communications::InWorldVoice::SessionInterface* voice_session) :
        voice_session_(voice_session)
    {
        setupUi(this);
        UpdateUI();

        QObject::connect(muteAllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ApplyMuteAllSelection()));

        QObject::connect(voice_session, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateUI()));
        QObject::connect(voice_session, SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateUI()));

        UpdateParticipantList();
        if (voice_session)
        {
            connect(voice_session, SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateParticipantList()) );
            connect(voice_session, SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), SLOT(UpdateParticipantList()) );
            connect(voice_session, SIGNAL(StateChanged(Communications::InWorldVoice::SessionInterface::State)), SLOT(UpdateParticipantList()) );
        }
    }

    VoiceControllerWidget::~VoiceControllerWidget()
    {

    }

    void VoiceControllerWidget::ApplyMuteAllSelection()
    {
        if (muteAllCheckBox->checkState() == Qt::Checked)
            voice_session_->DisableAudioReceiving();
        else
            voice_session_->EnableAudioReceiving();
    }

    void VoiceControllerWidget::UpdateUI()
    {
        if (voice_session_->Participants().length() > 0)
            participantsCountLabel->setText(QString("%1 participants").arg(voice_session_->Participants().length()));
        else
            participantsCountLabel->setText("No participants");
    }

    void VoiceControllerWidget::UpdateParticipantList()
    {
        if (!voice_session_)
            return;

        QList<Communications::InWorldVoice::ParticipantInterface*> list = voice_session_->Participants();
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
        if (number == 0)
            area_height = 0;
        else
            area_height = number*user_widgets_[0]->height() + userListLayout->spacing()*(number-1) + userListLayout->contentsMargins().bottom() + userListLayout->contentsMargins().top();

        const QRect& geometry = userListScrollAreaWidgetContents->geometry();
        userListScrollAreaWidgetContents->setGeometry(geometry.x(), geometry.y(), geometry.width(), area_height);
    }

} // CommUI
