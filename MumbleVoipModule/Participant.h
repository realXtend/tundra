// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Participant_h
#define incl_MumbleVoipModule_Participant_h

#include "CommunicationsService.h"

namespace MumbleVoip
{
    class User;

    namespace InWorldVoice
    {
        //! ParticipantInterface implementation using mumble User object
        class Participant : public Communications::InWorldVoice::ParticipantInterface
        {
            Q_OBJECT
        public:
            Participant(QString name, User* user);
            virtual ~Participant();
            virtual QString Name() const;
            virtual QString AvatarUUID() const;
            virtual bool IsSpeaking() const;
            virtual void Mute(bool mute);
            virtual bool IsMuted() const;
            virtual Vector3df Position() const;
            virtual void Add(User* user);
            virtual User* UserPtr() const;

        private:
            bool muted_;
            bool speaking_;
            bool position_known_;
            Vector3df position_;
            User* user_;
            QString name_;
            QString avatar_uuid_;

        private slots:
            void OnStartSpeaking();
            void OnStopSpeaking();
            void OnPositionUpdated();
            void OnUserLeft();
        };
        typedef QList<Participant*> ParticipantList;

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Participant_h
