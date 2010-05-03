// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Session_h
#define incl_MumbleVoipModule_Session_h

#include "CommunicationsService.h"

namespace Foundation
{
    class Framework;
}

namespace MumbleVoip
{
    class ConnectionManager;
    class ServerInfo;
    class User;
    class PCMAudioFrame;

    namespace InWorldVoice
    {
        class Participant;
        typedef QList<Participant*> ParticipantList;

        class Session : public Communications::InWorldVoice::SessionInterface
        {
            Q_OBJECT
        public:
            Session(Foundation::Framework* framework, const ServerInfo &server_info);
            virtual ~Session();

            virtual QString Description() const;
            virtual bool IsSendingAudio() const;
            virtual bool IsReceivingAudio() const;

            virtual void EnableAudioSending();
            virtual void DisableAudioSending();
            virtual bool IsAudioSendingEnabled() const;
            virtual void EnableAudioReceiving();
            virtual void DisableAudioReceiving();
            virtual bool IsAudioReceivingEnabled() const;

            virtual QList<Communications::InWorldVoice::ParticipantInterface*> Participants() const;

            virtual void Update(f64 frametime);

        private:
            bool GetOwnAvatarPosition(Vector3df& position, Vector3df& direction);

            Foundation::Framework* framework_;
            State state_;
            QString description_;
            bool sending_audio_;
            bool receiving_audio_;
            bool audio_sending_enabled_;
            bool audio_receiving_enabled_;
            ParticipantList participants_;
            ConnectionManager* connection_manager_;  // In future session could have multiple connections

        private slots:
            void OnUserJoined(User*);
            void OnUserLeft(User*);
            void OnUserStartSpeaking(); // rename to: UpdateReceivingAudioStatus
            void OnUserStopSpeaking(); // rename to: UpdateReceivingAudioStatus
            void OnAudioPacketSent(PCMAudioFrame*);
        };

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Session_h
