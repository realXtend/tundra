// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "User.h"
#include "PCMAudioFrame.h"
#include "MumbleVoipModule.h"
#include "stdint.h"
#include "MumbleDefines.h"
#include "PCMAudioFrame.h"
#include <QTimer>
#include "Channel.h"
#include <mumbleclient/user.h>
#include <mumbleclient/channel.h>

#include "MemoryLeakCheck.h"

namespace MumbleLib
{
    User::User(const MumbleClient::User& user, MumbleLib::Channel* channel)
        : user_(user),
          speaking_(false),
          position_known_(false),
          position_(0,0,0),
          left_(false),
          channel_(channel),
          received_voice_packet_count_(0),
          voice_packet_drop_count_(0)
    {
        last_audio_frame_time_.start(); // initialize time state so that restart is possible later
    }

    User::~User()
    {
        foreach(MumbleVoip::PCMAudioFrame* audio, playback_queue_)
            SAFE_DELETE(audio);

        playback_queue_.clear();
    }

    QString User::Name() const
    {
        if (left_)
            return "";
        return QString(user_.name.c_str());
    }
    
    //QString User::Hash()
    //{
    //    return QString(user_.hash.c_str());
    //}
    
    int User::Session() const
    {
        if (left_)
            return 0;
        return user_.session;
    }
    
    int User::Id() const
    {
        if (left_)
            return 0;
        return user_.user_id;
    }
    
    Channel* User::Channel() const
    {
        if (left_)
            return 0;
        return channel_;
    }
    
    QString User::Comment() const
    {
        if (left_)
            return "";
        return QString(user_.comment.c_str());
    }

    bool User::IsSpeaking() const
    {
        return speaking_;
    }

    void User::AddToPlaybackBuffer(MumbleVoip::PCMAudioFrame* frame)
    {
        received_voice_packet_count_++;
        // Buffer overflow handling: We drop the oldest packet in the buffer
        if (PlaybackBufferLengthMs() > PLAYBACK_BUFFER_MAX_LENGTH_MS_)
        {
            MumbleVoip::PCMAudioFrame* frame = GetAudioFrame();
            SAFE_DELETE(frame);
        }

        playback_queue_.push_back(frame);
        last_audio_frame_time_.restart();

        if (!speaking_)
        {
            speaking_ = true;
            emit StartReceivingAudio();
        }
    }

    void User::UpdatePosition(Vector3df position)
    {
        position_known_ = true;
        position_ = position;

        emit PositionUpdated();
    }

    Vector3df User::Position() const
    {
        return position_;
    }

    int User::PlaybackBufferLengthMs() const
    {
        return 1000 * playback_queue_.size() * MumbleVoip::SAMPLES_IN_FRAME / MumbleVoip::SAMPLE_RATE;
    }
    
    MumbleVoip::PCMAudioFrame* User::GetAudioFrame()
    {
        if (playback_queue_.size() == 0)
            return 0;

        return playback_queue_.takeFirst();
    }

    double User::VoicePacketDropRatio() const
    {
        if (received_voice_packet_count_ == 0)
            return 0;
        return static_cast<double>(voice_packet_drop_count_)/received_voice_packet_count_;
    }

    void User::CheckSpeakingState()
    {
        bool was_speaking = speaking_;

        if (speaking_ && last_audio_frame_time_.elapsed() > SPEAKING_TIMEOUT_MS)
        {
            speaking_ = false;
            if (was_speaking && !speaking_)
                emit StopReceivingAudio();
        }
    }

    void User::SetChannel(MumbleLib::Channel* channel)
    {
        if (left_)
            return;
        if (channel_ != channel)
        {
            channel_ = channel;
            emit ChangedChannel(this);
        }
    }

    int User::CurrentChannelID() const
    {
        if (left_)
            return -1;
        boost::shared_ptr<MumbleClient::Channel> mumble_channel = user_.channel.lock();
        if (mumble_channel)
            return user_.channel.lock()->id;
        else
            return -1;
    }

} // namespace MumbleLib
