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
//#include <CommunicationsService.h>
//#include <QMouseEvent>
#include "VoiceUsersWidget.h" /// @todo Separate to VoiceParticipantWidget.h/cpp
//#include "VoiceControl.h"

#include "DebugOperatorNew.h"

namespace CommUI
{

    VoiceControllerWidget::VoiceControllerWidget(Communications::InWorldVoice::SessionInterface* voice_session) :
        voice_controller_(voice_session)
    {
        setupUi(this);

        QStringList options;
        options << "Mute" << "Continuous transmission" << "Push-to-Talk" << "Toggle mode";
        transmissionModeComboBox->addItems(options);   
        QObject::connect(transmissionModeComboBox, SIGNAL(currentIndexChanged(int)), (VoiceControllerWidget*)this, SLOT(ApplyTransmissionModeSelection(int)));

        QObject::connect(showListButton, SIGNAL(clicked()), this, SLOT(OpenParticipantListWidget()));

        QObject::connect(muteAllCheckBox, SIGNAL(stateChanged(int)), this, SLOT(ApplyMuteAllSelection()));

        QObject::connect(voice_controller_.GetSession(), SIGNAL(ParticipantJoined(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateUI()));
        QObject::connect(voice_controller_.GetSession(), SIGNAL(ParticipantLeft(Communications::InWorldVoice::ParticipantInterface*)), this, SLOT(UpdateUI()));
        UpdateUI();

        /// @todo Use Settings class from MumbeVoipModule
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/MumbleVoip");
        int default_voice_mode = settings.value("MumbleVoice/default_voice_mode").toInt();
        transmissionModeComboBox->setCurrentIndex(default_voice_mode);

        showListButton->hide(); // temporaly hide the button because there is an another similiar button.

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

    void VoiceControllerWidget::ApplyTransmissionModeSelection(int selection)
    {
        UpdateUI();
        voice_controller_.SetTransmissionMode(VoiceController::TransmissionMode(selection));
    }

    void VoiceControllerWidget::OpenParticipantListWidget()
    {


    }

    void VoiceControllerWidget::ApplyMuteAllSelection()
    {
        if (muteAllCheckBox->checkState() == Qt::Checked)
        {
            voice_controller_.GetSession()->DisableAudioReceiving();
        }
        else
        {
            voice_controller_.GetSession()->EnableAudioReceiving();
        }
    }

    void VoiceControllerWidget::UpdateUI()
    {
        if (voice_controller_.GetSession()->Participants().length() > 0)
            participantsCountLabel->setText(QString("%1 participants").arg(voice_controller_.GetSession()->Participants().length()));
        else
            participantsCountLabel->setText("No participants");

        switch(transmissionModeComboBox->currentIndex())
        {
        case 0:
            transmissionModeDescriptionLabel->setText("Another participants cannot hear you.");
            break;
        case 1:
            transmissionModeDescriptionLabel->setText("Another participants can here you.");
            break;
        case 2:
            transmissionModeDescriptionLabel->setText("Another participants can here you while you keep pressing middle mouse button (MMB) down.");
            break;
        case 3:
            transmissionModeDescriptionLabel->setText("Another participants can here you when you have enabled transmission. You can toggle the state by pressing middle mouse button (MMB).");
            break;
        }
    }

    void VoiceControllerWidget::SetPushToTalkOn()
    {
        voice_controller_.SetPushToTalkOn();
    }

    void VoiceControllerWidget::SetPushToTalkOff()
    {
        voice_controller_.SetPushToTalkOff();
    }

    void VoiceControllerWidget::Toggle()
    {
        voice_controller_.Toggle();
    }

    void VoiceControllerWidget::UpdateParticipantList()
    {
        Communications::InWorldVoice::SessionInterface* session = voice_controller_.GetSession();
        if (!session)
            return;

        QList<Communications::InWorldVoice::ParticipantInterface*> list = session->Participants();
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
        userListScrollAreaWidgetContents->
        setGeometry(geometry.x(), geometry.y(), geometry.width(), area_height);
    }

} // CommUI
