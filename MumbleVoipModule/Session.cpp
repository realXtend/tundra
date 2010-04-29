#include "StableHeaders.h"
#include "Session.h"
#include "Participant.h"
#include "ConnectionManager.h"
#include "ServerInfo.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Session::Session(Foundation::Framework* framework, const ServerInfo &server_info) : 
            state_(STATE_INITIALIZING),
            description_(""),
            sending_audio_(false),
            receiving_audio_(false),
            audio_sending_enabled_(false),
            audio_receiving_enabled_(false),
            connection_manager_(new ConnectionManager(framework))
        {
            connection_manager_->OpenConnection(server_info);
            connection_manager_->SendAudio(audio_sending_enabled_);
            connect(connection_manager_, SIGNAL(UserJoined(User*)), SLOT(OnUserJoined(User*)) );
        }

        Session::~Session()
        {
            SAFE_DELETE(connection_manager_);
        }

        QString Session::Description() const
        {
            return description_;
        }

        bool Session::IsSendingAudio() const
        {
            return sending_audio_;
        }

        bool Session::IsReceivingAudio() const
        {
            return receiving_audio_;
        }

        void Session::EnableAudioSending()
        {
            audio_sending_enabled_ = true;
            connection_manager_->SendAudio(true);
        }

        void Session::DisableAudioSending()
        {
            audio_sending_enabled_ = false;
            connection_manager_->SendAudio(false);
        }

        bool Session::IsAudioSendingEnabled() const
        {
            return audio_sending_enabled_;
        }

        void Session::EnableAudioReceiving()
        {
            audio_receiving_enabled_ = true;
            connection_manager_->ReceiveAudio(true);
        }

        void Session::DisableAudioReceiving()
        {
            audio_receiving_enabled_ = false;
            connection_manager_->ReceiveAudio(false);
        }

        bool Session::IsAudioReceivingEnabled() const
        {
            return audio_receiving_enabled_;
        }

        QList<Communications::InWorldVoice::ParticipantInterface*> Session::Participants() const
        {
            QList<Communications::InWorldVoice::ParticipantInterface*> list;
            for(ParticipantList::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
            {
                Participant* p = *i;
                list.append(static_cast<Communications::InWorldVoice::ParticipantInterface*>(p));
            }
            return list;
        }

        void Session::Update(f64 frametime)
        {
            if (connection_manager_)
                connection_manager_->Update(frametime);
        }

        void Session::OnUserJoined(User* user)
        {
            Participant* p = new Participant(user);
            participants_.append(p);
            connect(p, SIGNAL(StartSpeaking()), SLOT(OnUserStartSpeaking()) );
            connect(p, SIGNAL(SopSpeaking()), SLOT(OnuserStopSpeaking()) );

//            emit ParticipantJoined(p);
        }

        void Session::OnUserStartSpeaking()
        {
            bool was_receiving_audio = receiving_audio_;
            receiving_audio_ = true;
            //if (!was_receiving_audio)
            //    emit StartReceivingAudio();
        }

        void Session::OnUserStopSpeaking()
        {
            bool was_receiving_audio = receiving_audio_;
            foreach(Participant* p, participants_)
            {
                if (p->IsSpeaking())
                {
                    receiving_audio_ = true;
                    return;
                }
            }
            receiving_audio_ = false;
            //if (was_receiving_audio)
            //    emit StopReceivingAudio();
        }

    } // InWorldVoice

} // MumbleVoip
