#include "StableHeaders.h"
#include "Participant.h"
#include "User.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Participant::Participant(QString name, User* user) :
            muted_(false),
            speaking_(false),
            position_known_(false),
            position_(0.0, 0.0, 0.0),
            user_(user),
            name_(name)
        {
            avatar_uuid_ = user_->Name();
            connect(user_, SIGNAL(StartReceivingAudio()), SLOT(OnStartSpeaking()) );
            connect(user_, SIGNAL(StopReceivingAudio()), SLOT(OnStopSpeaking()) );
            connect(user_, SIGNAL(PositionUpdated()), SLOT(OnPositionUpdated()) );
            connect(user_, SIGNAL(Left()), SLOT(OnUserLeft()) );
//            connect(user_, SIGNAL(ChangedChannel()), SLOT((User)) );
        }

        Participant::~Participant()
        {

        }

        QString Participant::Name() const
        {
            return name_;
        }

        QString Participant::AvatarUUID() const
        {
            return avatar_uuid_;
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

        void Participant::Add(User* user)
        {
            user_ = user;
        }

        void Participant::OnStartSpeaking()
        {
            speaking_ = true;
            emit Communications::InWorldVoice::ParticipantInterface::StartSpeaking();
        }

        void Participant::OnStopSpeaking()
        {
            speaking_ = false;
            emit Communications::InWorldVoice::ParticipantInterface::StopSpeaking();
        }

        void Participant::OnPositionUpdated()
        {
            //! @todo ENSURE THAT user_ OBJECT IS NOT DELETED
            if (!user_)
                return;
            
            position_ = user_->Position();
        }

        User* Participant::UserPtr() const
        {
            return user_;
        }

        void Participant::OnUserLeft()
        {
            emit Communications::InWorldVoice::ParticipantInterface::Left();
        }



    } // InWorldVoice

} // MumbleVoip
