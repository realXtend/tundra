/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   VoiceController.cpp
 *  @brief  In-world voice controller user interface
 *          
 */
 
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <QSettings>
#include "VoiceController.h"
#include <CommunicationsService.h>
#include <QMouseEvent>

#include "DebugOperatorNew.h"

namespace CommUI
{
    VoiceController::VoiceController(Communications::InWorldVoice::SessionInterface* voice_session) :
        transmission_mode_(Mute),
        in_world_voice_session_(voice_session),
        voice_activity_timeout_ms_(1000),
        transmitting_audio_(false),
        push_to_talk_on_(false),
        toggle_mode_on_(false)
    {
    }

    VoiceController::~VoiceController()
    {
    }

    void VoiceController::SetTransmissionMode(TransmissionMode mode)
    {
        TransmissionMode old_mode = transmission_mode_;
        transmission_mode_ = mode;
        push_to_talk_on_ = false;
        toggle_mode_on_ = false;
        SetTransmissionState();
        if (old_mode != transmission_mode_)
            emit TransmissionModeChanged(transmission_mode_);
    }

    void VoiceController::SetPushToTalkOn()
    {
        push_to_talk_on_ = true;
        SetTransmissionState();
    }

    void VoiceController::SetPushToTalkOff()
    {
        push_to_talk_on_ = false;
        SetTransmissionState();
    }
    
    void VoiceController::Toggle()
    {
        toggle_mode_on_ = !toggle_mode_on_;
        SetTransmissionState();
    }

    void VoiceController::SetTransmissionState()
    {
        bool was_transmitting_audio = transmitting_audio_;
        switch (transmission_mode_)
        {
        case Mute:
            transmitting_audio_ = false;
            break;

        case ContinuousTransmission:
            transmitting_audio_ = true;
            break;

        case PushToTalk:
            if (push_to_talk_on_)
                transmitting_audio_ = true;
            else
                transmitting_audio_ = false;
            break;

        case ToggleMode:
            if (toggle_mode_on_)
                transmitting_audio_ = true;
            else
                transmitting_audio_ = false;
            break;

        case VoiceActivity:
            transmitting_audio_ = false;
            // @todo IMPLEMENT
            break;
        }

        if (transmitting_audio_)
            in_world_voice_session_->EnableAudioSending();
        else
            in_world_voice_session_->DisableAudioSending();

        if (transmitting_audio_ != was_transmitting_audio)
            if (transmitting_audio_)
                emit TransmittingAudioStarted();
            else
                emit TransmittingAudioStopped();
    }


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

} // CommUI
