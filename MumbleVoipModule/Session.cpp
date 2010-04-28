#include "StableHeaders.h"
#include "Session.h"
#include "Participant.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Session::Session() : 
            description_(""),
            sending_audio_(false),
            receiving_audio_(false),
            audio_sending_enabled_(false),
            audio_receiving_enabled_(false)
        {

        }

        Session::~Session()
        {

        }

        QString Session::Description()
        {
            return description_;
        }

        bool Session::IsSendingAudio()
        {
            return sending_audio_;
        }


        bool Session::IsReceivingAudio()
        {
            return receiving_audio_;
        }

        void Session::EnableAudioSending()
        {
            audio_sending_enabled_ = true;
        }

        void Session::DisableAudioSending()
        {
            audio_sending_enabled_ = false;
        }

        bool Session::IsAudioSendingEnabled()
        {
            return audio_sending_enabled_;
        }

        void Session::EnableAudioReceiving()
        {
            audio_receiving_enabled_ = true;
        }

        void Session::DisableAudioReceiving()
        {
            audio_receiving_enabled_ = false;
        }

        bool Session::IsAudioReceivingEnabled()
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
            //! \todo IMPLEMENT
            return list;
        }

    } // InWorldVoice

} // MumbleVoip
