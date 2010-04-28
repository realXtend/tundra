// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Participant_h
#define incl_MumbleVoipModule_Participant_h

#include "CommunicationsService.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        class Participant : public Communications::InWorldVoice::ParticipantInterface
        {
            Q_OBJECT
        public:
            Participant();
            virtual ~Participant();
            virtual bool IsSpeaking() const = 0;
            virtual void Mute(bool mute) = 0;
            virtual bool IsMuted() const = 0;
            virtual Vector3df Position() const = 0;
        private:
            bool muted_;
            bool speaking_;
            bool position_known_;
            Vector3df position_;
        };
        typedef QList<Participant*> ParticipantList;

    } // InWorldVoice

} // MumbleVoip

#endif // incl_MumbleVoipModule_Participant_h
