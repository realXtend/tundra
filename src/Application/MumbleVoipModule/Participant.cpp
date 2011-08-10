// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Participant.h"
#include "User.h"

namespace MumbleVoip
{
    Participant::Participant(QString name, MumbleLib::User* user) :
        muted_(false),
        speaking_(false),
        position_known_(false),
        position_(0.0, 0.0, 0.0),
        user_(user),
        name_(name),
        voice_activity_(0)
    {
        connect(user_, SIGNAL(StartReceivingAudio()), SLOT(OnStartSpeaking()) );
        connect(user_, SIGNAL(StopReceivingAudio()), SLOT(OnStopSpeaking()) );
        connect(user_, SIGNAL(PositionUpdated()), SLOT(OnPositionUpdated()) );
        connect(user_, SIGNAL(Left()), SLOT(OnUserLeft()) );
        connect(user_, SIGNAL(destroyed()), SLOT(UserObjectDestroyed()));
    }

    Participant::~Participant()
    {
    }

    QString Participant::Name() const
    {
        return name_;
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

    float3 Participant::Position() const
    {
        return position_;
    }

    void Participant::Add(MumbleLib::User* user)
    {
        user_ = user;
    }

    void Participant::OnStartSpeaking()
    {
        speaking_ = true;
        emit StartSpeaking();
    }

    void Participant::OnStopSpeaking()
    {
        speaking_ = false;
        emit StopSpeaking();
    }

    void Participant::OnPositionUpdated()
    {
        //! @todo ENSURE THAT user_ OBJECT IS NOT DELETED
        if (!user_)
            return;
        
        position_ = user_->Position();
    }

    MumbleLib::User* Participant::UserPtr() const
    {
        return user_;
    }

    void Participant::OnUserLeft()
    {
        emit Left();
    }

    void Participant::SetName(QString name)
    {
        name_ = name;
        emit StateChanged();
    }

    double Participant::VoiceActivity() const
    {
         // todo: implement
        return voice_activity_;
    }

    void Participant::UserObjectDestroyed()
    {
        user_ = 0;
    }

} // MumbleVoip
