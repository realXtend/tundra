// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Session_h
#define incl_MumbleVoipModule_Session_h

#include "CommunicationsService.h"


namespace MumbleVoip
{
    namespace InWorldVoice
    {
        class Participant;
        typedef QList<Participant*> ParticipantList;

        class Session : public Communications::InWorldVoice::SessionInterface
        {
            Q_OBJECT
        public:
            Session();
            virtual ~Session();

            virtual QString Description();
            virtual bool IsSendingAudio();
            virtual bool IsReceivingAudio();

            virtual void EnableAudioSending();
            virtual void DisableAudioSending();
            virtual bool IsAudioSendingEnabled();
            virtual void EnableAudioReceiving();
            virtual void DisableAudioReceiving();
            virtual bool IsAudioReceivingEnabled();

            virtual QList<Communications::InWorldVoice::ParticipantInterface*> Participants() const;
        private:
            QString description_;
            bool sending_audio_;
            bool receiving_audio_;
            bool audio_sending_enabled_;
            bool audio_receiving_enabled_;
            ParticipantList participants_;
        };

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Session_h
