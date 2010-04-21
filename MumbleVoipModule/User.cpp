#include "StableHeaders.h"
#include "User.h"
#include "MumbleVoipModule.h"
#include "stdint.h"
#include "MumbleDefines.h"

#include <QTimer>

#include <mumbleclient/user.h>

namespace MumbleVoip
{
    User::User(const MumbleClient::User& user) : user_(user), speaking_(false), position_known_(false), position_(0,0,0)
    {
    }

    User::~User()
    {
        foreach(PCMAudioFrame* audio, playback_queue_)
        {
            SAFE_DELETE(audio);
        }
        playback_queue_.clear();
    }

    QString User::Name()
    {
        return QString(user_.name.c_str());
    }
    
    QString User::Hash()
    {
        return QString(user_.hash.c_str());
    }
    
    int User::Session()
    {
        return user_.session;
    }
    
    int User::Id()
    {
        return user_.user_id;
    }
    
    Channel* User::Channel()
    {
        return 0; // \todo: implement
    }
    
    QString User::Comment()
    {
        return QString(user_.comment.c_str());
    }

    bool User::IsSpeaking()
    {
        return speaking_;
    }

    void User::OnAudioFrameReceived(PCMAudioFrame* frame)
    {
        if (PlaybackBufferAvailableMs() < PLAYBACK_BUFFER_MS_)
            playback_queue_.push_back(frame);

        if (!speaking_)
            emit StartSpeaking();
        speaking_ = true;

        QTimer::singleShot(SPEAKING_TIMEOUT_MS, this, SLOT(SpeakingTimeout()));
    }

    void User::SpeakingTimeout()
    {
        if (speaking_)
            emit StopSpeaking();
        speaking_ = false;
    }

    void User::UpdatePosition(Vector3df position)
    {
        position_ = position;
    }

    Vector3df User::Position()
    {
        return position_;
    }

    double User::PlaybackBufferAvailableMs()
    {
        return 1000 * playback_queue_.size() * SAMPLES_IN_FRAME / SAMPLE_RATE;
    }
    
    PCMAudioFrame* User::GetAudioFrame()
    {
        if (playback_queue_.size() == 0)
            return 0;

        return playback_queue_.takeFirst();
    }

} // namespace MumbleVoip
