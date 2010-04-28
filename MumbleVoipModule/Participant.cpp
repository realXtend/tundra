#include "StableHeaders.h"
#include "Participant.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Participant::Participant() :
            muted_(false),
            speaking_(false),
            position_known_(false),
            position_(0.0, 0.0, 0.0)
        {

        }

            Participant::~Participant()
        {

        }

        bool Participant::IsSpeaking() const
        {
            return speaking_;
        }

        void Participant::Mute(bool mute)
        {
            muted_ = mute;
        }

        bool Participant::IsMuted() const
        {
            return muted_;
        }

        Vector3df Participant::Position() const
        {
            return position_;
        }

    } // InWorldVoice

} // MumbleVoip
