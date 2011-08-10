// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"

#include "Session.h"
#include "ServerInfo.h"
#include "PCMAudioFrame.h"

#include "SceneAPI.h"
#include "Scene.h"

#include "MumbleVoipModule.h"
#include "User.h"
#include "Channel.h"
#include "Connection.h"
#include "Participant.h"
#include "MumbleLibrary.h"
#include "Settings.h"

#include <QTimer>

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    const double Session::DEFAULT_AUDIO_QUALITY_ = 0.5; // 0 .. 1.0

    Session::Session(Framework* framework, Settings* settings) : 
        state_(STATE_CLOSED),
        reason_(""),
        framework_(framework),
        description_(""),
        sending_audio_(false),
        receiving_audio_(false),
        audio_sending_enabled_(false),
        audio_receiving_enabled_(true),
        speaker_voice_activity_(0),
        connection_(0),
        settings_(settings),
        local_echo_mode_(false),
        reconnect_timeout_(300),
        server_address_("")
    {
        connect(settings_, SIGNAL(PlaybackBufferSizeMsChanged(int)), this, SLOT(SetPlaybackBufferSizeMs(int)));
        connect(settings_, SIGNAL(EncodeQualityChanged(double)), this, SLOT(SetEncodeQuality(double)));
    }

    Session::~Session()
    {
        Close();
    }

    void Session::OpenConnection(ServerInfo server_info)
    {
        state_ = STATE_INITIALIZING;
        emit StateChanged(state_);
        /// @todo Check that closed connection can be reopened
        SAFE_DELETE(connection_);

        connection_ = new MumbleLib::Connection(server_info, 200);
        if (connection_->GetState() == MumbleLib::Connection::STATE_ERROR)
        {
            state_ = STATE_ERROR;
            reason_ = connection_->GetReason();
            return;
        }
        server_address_ = server_info.server;

        connect(connection_, SIGNAL(UserJoinedToServer(MumbleLib::User*)), SLOT(CreateNewParticipant(MumbleLib::User*)), Qt::UniqueConnection);
        connect(connection_, SIGNAL(UserLeftFromServer(MumbleLib::User*)), SLOT(UpdateParticipantList()), Qt::UniqueConnection);
        connect(connection_, SIGNAL(StateChanged(MumbleLib::Connection::State)), SLOT(CheckConnectionState()), Qt::UniqueConnection);

        connection_->Join(server_info.channel_name);
        connection_->SetEncodingQuality(DEFAULT_AUDIO_QUALITY_);
        connection_->SendPosition(settings_->GetPositionalAudioEnabled());
        connection_->SendAudio(audio_sending_enabled_);
        connection_->ReceiveAudio(audio_receiving_enabled_);
        
        // Set the current settings as the mumble config/settings widget might not be used.
        // Was used in OS Naali.
        if (sending_audio_)
            EnableAudioReceiving();
        else
            DisableAudioReceiving();
        if (audio_sending_enabled_)
            EnableAudioSending();
        else
            DisableAudioSending();

        MumbleLib::MumbleLibrary::Start();
        state_ = STATE_OPEN;
        emit StateChanged(state_);
    }

    void Session::Close()
    {
        if (state_ != STATE_CLOSED && state_ != STATE_ERROR)
        {
            State old_state = state_;
            state_ = STATE_CLOSED;

            if(connection_)
                SAFE_DELETE(connection_);

            current_mumble_channel_ = "";
            emit ActiceChannelChanged(current_mumble_channel_);

            if (old_state != state_)
                emit StateChanged(state_);
        }
        foreach(Participant* p, participants_)
        {
            IParticipant *iPart = dynamic_cast<IParticipant*>(p);
            if (iPart)
                emit ParticipantLeft(iPart);
            else
                LogInfo("MumbleVoidp:Session::Close(): IParticipant *iPart = dynamic_cast<IParticipant*>(p) FAILED!");
            SAFE_DELETE(p);
        }
        participants_.clear();
        other_channel_users_.clear();
    }

    ISession::State Session::GetState() const
    {
        return state_;
    }

    QString Session::Reason() const
    {
        return reason_;
    }

    QString Session::Description() const
    {
        return description_;
    }

    bool Session::IsSendingAudio() const
    {
        return sending_audio_;
    }

    bool Session::IsReceivingAudio() const
    {
        return receiving_audio_;
    }

    void Session::EnableAudioSending()
    {
        if (!framework_->Audio())
            return;

        if (connection_)
            connection_->SendAudio(true);
        bool audio_sending_was_enabled = audio_sending_enabled_;
        audio_sending_enabled_ = true;
        if (!audio_sending_was_enabled)
        {
            int frequency = SAMPLE_RATE;
            bool sixteenbit = true;
            bool stereo = false;
            int buffer_size = SAMPLE_WIDTH/8*frequency*AUDIO_RECORDING_BUFFER_MS/1000;
            framework_->Audio()->StartRecording(QString::fromStdString(recording_device_), frequency, sixteenbit, stereo, buffer_size);

            emit StartSendingAudio();
        }
    }

    void Session::DisableAudioSending()
    {
        if (!framework_->Audio())
            return;

        if (connection_)
            connection_->SendAudio(false);
        bool audio_sending_was_enabled = audio_sending_enabled_;
        audio_sending_enabled_ = false;
        if (audio_sending_was_enabled)
        {
            framework_->Audio()->StopRecording();

            emit StopSendingAudio();
        }
    }

    bool Session::IsAudioSendingEnabled() const
    {
        return audio_sending_enabled_;
    }

    void Session::EnableAudioReceiving()
    {
        if (connection_)
            connection_->ReceiveAudio(true);
        audio_receiving_enabled_ = true;
    }

    void Session::DisableAudioReceiving()
    {
        if (connection_)
            connection_->ReceiveAudio(false);
        audio_receiving_enabled_ = false;
    }

    bool Session::IsAudioReceivingEnabled() const
    {
        return audio_receiving_enabled_;
    }

    double Session::SpeakerVoiceActivity() const
    {
        return speaker_voice_activity_;
    }

    void Session::SetPosition(float3 position)
    {
        user_position_ = position;
    }

    void Session::EnablePositionalAudio(bool enable)
    {
        settings_->SetPositionalAudioEnabled(enable);
        if(connection_)
            connection_->SendPosition(enable);
    }

    bool Session::GetPositionalAudioEnabled() const
    {
        return settings_->GetPositionalAudioEnabled();
    }

    ParticipantList Session::Participants() const
    {
        ParticipantList list;
        for (QList<Participant*>::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            Participant* p = *i;
            list.append(static_cast<IParticipant*>(p));
        }
        return list;
    }

    QStringList Session::GetParticipantsNames() const
    {
        QStringList names;
        for (QList<Participant*>::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            Participant* p = *i;
            names << p->Name();
        }
        return names;
    }

    void Session::MuteParticipantByName(QString name, bool mute) const
    {
        for (QList<Participant*>::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            Participant* p = *i;
            if (p->Name() == name)
            {
                p->Mute(mute);
            }
        }
    }

    void Session::Update(f64 frametime)
    {
        PlaybackReceivedAudio();
        SendRecordedAudio();
        UpdateParticipantList();
    }

    void Session::CreateNewParticipant(MumbleLib::User* user)
    {
        foreach(Participant* p, participants_)
        {
            if (p->UserPtr() == user)
                return;
        }
        
        disconnect(user, SIGNAL(ChangedChannel(MumbleLib::User*)),this, SLOT(CheckChannel(MumbleLib::User*)));    
        connect(user, SIGNAL(ChangedChannel(MumbleLib::User*)), SLOT(CheckChannel(MumbleLib::User*)));

        if (user->GetChannel()->FullName() != current_mumble_channel_)
        {
            other_channel_users_.append(user);
            return; 
        }

        Participant* p = new Participant(user->Name(), user);
        participants_.append(p);

        connect(p, SIGNAL(StartSpeaking()), SLOT(OnUserStartSpeaking()));
        connect(p, SIGNAL(StopSpeaking()), SLOT(OnUserStopSpeaking()));
        connect(p, SIGNAL(Left()), SLOT(UpdateParticipantList()));

        IParticipant *iParticipant = dynamic_cast<IParticipant*>(p);
        if (iParticipant)
            emit ParticipantJoined(p);
        else
            LogInfo("MumbleVoidp:Session::CreateNewParticipant(): IParticipant *iPart = dynamic_cast<IParticipant*>(p) FAILED!");
    }

    void Session::CheckChannel(MumbleLib::User* user)
    {
        if (user->IsLeft())
            return;

        if (user->GetChannel()->FullName() == current_mumble_channel_)
        {
            foreach(MumbleLib::User* u, other_channel_users_)
            {
                if (u == user)
                    other_channel_users_.removeAll(u);
            }
            CreateNewParticipant(user);
            return;
        }
        else
        {
            foreach(Participant* p, participants_)
            {
                if (p->UserPtr() == user)
                {
                    participants_.removeOne(p);
                    other_channel_users_.push_back(p->UserPtr()); /// \note CHECKME
                    emit ParticipantLeft(p);
                }
            }
        }
    }

    void Session::UpdateParticipantList()
    {
        foreach(Participant* p, participants_)
        {
            if (p->UserPtr()->IsLeft())
            {
                participants_.removeOne(p);
                other_channel_users_.append(p->UserPtr());
                emit ParticipantLeft(p);
                continue;
            }

            /// @todo Update spaces away? Really needed?
            QString avatar_name = p->UserPtr()->Name();
            avatar_name.replace('_', ' ');        
            p->SetName(avatar_name);
        }
    }

    void Session::OnUserStartSpeaking()
    {
        bool was_receiving_audio = receiving_audio_;
        receiving_audio_ = true;
        if (!was_receiving_audio)
            emit StartReceivingAudio();
    }

    void Session::OnUserStopSpeaking()
    {
        bool was_receiving_audio = receiving_audio_;
        bool someone_speaking = false;
        foreach(Participant* p, participants_)
        {
            if (p->IsSpeaking())
            {
                someone_speaking = true;
                return;
            }
        }
        if (was_receiving_audio && !someone_speaking)
        {
            receiving_audio_ = false;
            emit StopReceivingAudio();
        }
    }

    void Session::UpdateSpeakerActivity(PCMAudioFrame* frame)
    {
        static int counter = 0;
        counter++;
        counter = counter % 10; // 10 ms audio frames -> 10 fps speaker activity updates
        if (counter != 0)
            return;

        short top = 0;
        const short max = 100; //! \todo Use more proper treshold value
        for(int i = 0; i < frame->SampleCount(); ++i)
        {
            int sample = abs(frame->SampleAt(i));
            if (sample > top)
                top = sample;
        }

        double activity = top / max;
        if (activity > 1.0)
            activity = 1.0;

        double old_activity = speaker_voice_activity_;
        speaker_voice_activity_ = activity;
        if (old_activity != activity)
            emit SpeakerVoiceActivityChanged(activity);
    }

    void Session::SendRecordedAudio()
    {
        if (!framework_->Audio())
            return;

        if (!connection_)
            return;

        while (framework_->Audio()->GetRecordedSoundSize() > SAMPLES_IN_FRAME*SAMPLE_WIDTH/8)
        {
            int bytes_to_read = SAMPLES_IN_FRAME*SAMPLE_WIDTH/8;
            PCMAudioFrame* frame = new PCMAudioFrame(SAMPLE_RATE, SAMPLE_WIDTH, NUMBER_OF_CHANNELS, bytes_to_read );
            int bytes = framework_->Audio()->GetRecordedSoundData(frame->DataPtr(), bytes_to_read);
            UNREFERENCED_PARAM(bytes);
            ApplyMicrophoneLevel(frame);
            UpdateSpeakerActivity(frame);
            assert(bytes_to_read == bytes);

            //if (voice_indicator_)
            //{
            //    voice_indicator_->AnalyzeAudioFrame(frame);
            //    if (!voice_indicator_->IsSpeaking())
            //    {
            //        delete frame;
            //        continue;
            //    }
            //}
            if (audio_sending_enabled_)
                connection_->SendAudioFrame(frame, user_position_);
            else
                delete frame;
        }
    }

    void Session::PlaybackReceivedAudio()
    {
        if (!connection_)
            return;

        if (!audio_receiving_enabled_)
            return;

        for(;;) // until we have 'em all
        {
            MumbleLib::AudioPacket packet = connection_->GetAudioPacket();
            if (packet.second == 0)
                break; // there was nothing to play

            bool source_muted = false;
            foreach(Participant* participant, participants_)
            {
                if (participant->UserPtr() == packet.first && participant->IsMuted())
                {
                    source_muted = true;
                    break;
                }
            }
            if (!source_muted)
                PlaybackAudioFrame(packet.first, packet.second);
            else
                delete packet.second;
        }
    }

    void Session::PlaybackAudioFrame(MumbleLib::User* user, PCMAudioFrame* frame)
    {
        if (!framework_->Audio())
            return;

        SoundBuffer sound_buffer;
        
        sound_buffer.data.resize(frame->DataSize());
        memcpy(&sound_buffer.data[0], frame->DataPtr(), frame->DataSize());

        sound_buffer.frequency = frame->SampleRate();
        if (frame->SampleWidth() == 16)
            sound_buffer.is16Bit = true;
        else
            sound_buffer.is16Bit = false;
        
        if (frame->Channels() == 2)
            sound_buffer.stereo = true;
        else
            sound_buffer.stereo = false;

        if (!user)
        {
            const int source_id = 0;
            if (audio_playback_channels_.contains(source_id))
                framework_->Audio()->PlaySoundBuffer(sound_buffer, SoundChannel::Voice, audio_playback_channels_[0]);
            else
                audio_playback_channels_[0] = framework_->Audio()->PlaySoundBuffer(sound_buffer, SoundChannel::Voice);
        }
        else
        {
            QMutexLocker user_locker(user);
            if (audio_playback_channels_.contains(user->Session()))
                if (user->PositionKnown() && settings_->GetPositionalAudioEnabled())
                    framework_->Audio()->PlaySoundBuffer3D(sound_buffer, SoundChannel::Voice, user->Position(), audio_playback_channels_[user->Session()]);
                else
                    framework_->Audio()->PlaySoundBuffer(sound_buffer,  SoundChannel::Voice, audio_playback_channels_[user->Session()]);
            else
                if (user->PositionKnown() && settings_->GetPositionalAudioEnabled())
                    audio_playback_channels_[user->Session()] = framework_->Audio()->PlaySoundBuffer3D(sound_buffer, SoundChannel::Voice, user->Position());
                else
                    audio_playback_channels_[user->Session()] = framework_->Audio()->PlaySoundBuffer(sound_buffer,  SoundChannel::Voice);
        }

        delete frame;
    }

    QList<QString> Session::Statistics()
    {
        QList<QString> lines;
        QString line = QString("  Total %1 participants:").arg(participants_.size());
        lines.append(line);
        foreach(Participant* p, participants_)
        {
            MumbleLib::User* user = p->UserPtr();
            if (!user)
                continue;
            int buffer_len = user->PlaybackBufferLengthMs();
            int drop = static_cast<int>( 100*user->VoicePacketDropRatio() );
            QString line = QString("    participant %1:   audio buffer=%2 ms   frame loss=%3 %").arg(p->Name()).arg(buffer_len).arg(drop);
            lines.append(line);
        }
        return lines;
    }

    void Session::CheckConnectionState()
    {
        switch(connection_->GetState())
        {
        case STATE_ERROR:
            if (state_ == STATE_OPEN) // Reconnect
            {
                Reconnect();
            }
            break;
        }
    }

    void Session::Reconnect()
    {
        LogInfo("MumbleVoidp: Connection to server lost. Reconnecting..");
        Close();
        ServerInfo server_info = channels_[current_mumble_channel_];
        OpenConnection(server_info);

        if(state_ == STATE_ERROR)
        {
            LogInfo("MumbleVoidp: Reconnection failed, trying again in " + ToString(reconnect_timeout_) + " seconds..");
            QTimer::singleShot(reconnect_timeout_, this, SLOT(Reconnect()));
        }
        else
            PopulateParticipantList();
    }

    void Session::SetPlaybackBufferSizeMs(int size)
    {
        connection_->SetPlaybackBufferMaxLengthMs(size);
    }

    void Session::SetEncodeQuality(double quality)
    {
        connection_->SetEncodingQuality(quality);
    }

    void Session::ApplyMicrophoneLevel(PCMAudioFrame* frame)
    {
        frame->DataPtr();
        for(int i = 0; i < frame->SampleCount(); ++i)
        {
            int sample = static_cast<int>((frame->SampleAt(i))*settings_->GetMicrophoneLevel());
            frame->SetSampleAt(i, sample);
        }
    }

    int Session::GetAverageBandwithIn() const
    {
        if (connection_)
            return connection_->GetAverageBandwithIn();
        else
            return 0;
    }

    int Session::GetAverageBandwithOut() const
    {
        if (connection_)
            return connection_->GetAverageBandwithOut();
        else
            return 0;
    }

    QString Session::GetServerInfo() const
    {
        return server_address_;
    }

    QString Session::GetActiveChannel() const
    {
        return current_mumble_channel_;
    }

    void Session::ClearParticipantList()
    {
        foreach(Participant* p, participants_)
        {
            participants_.removeAll(p);
            other_channel_users_.push_back(p->UserPtr());
            emit ParticipantLeft(p);
        }
        participants_.clear();
    }

    void Session::PopulateParticipantList()
    {
        ClearParticipantList();

        foreach(MumbleLib::User* user, other_channel_users_)
        {
            if(user->GetChannel()->FullName() == current_mumble_channel_)
            {
                CreateNewParticipant(user);
            }
        }
    }

    void Session::SetActiveChannel(QString channel_name) 
    {
        if (current_mumble_channel_ == channel_name)
            return;

        if (!channels_.contains(channel_name))
        {
            LogInfo("MumbleVoidp: Channel \"" + channel_name.toStdString() + "\" not found!");
            return;
        }

        ServerInfo server_info = channels_[channel_name];

        if(connection_ && QString::compare(server_info.server, connection_->GetCurrentServer(), Qt::CaseInsensitive) == 0 && GetState() != STATE_CLOSED && GetState() != STATE_ERROR)
        {
            connection_->Join(channel_name);
        }
        else
        {
            if (GetState() != STATE_CLOSED)
                Close();
            OpenConnection(server_info);
        }

        current_mumble_channel_ = channel_name;
        LogInfo(QString("MumbleVoidp: Active voice channel changed to: %1").arg(current_mumble_channel_).toStdString());
        PopulateParticipantList();
        emit ActiceChannelChanged(channel_name);
    }

    QStringList Session::GetChannels()
    {
        return QStringList(channels_.keys());
    }

    void Session::AddChannel(QString name, const ServerInfo &server_info)
    {
        channels_[name] = server_info;
        emit ChannelListChanged(GetChannels());
    }

    void Session::AddChannel(QString name, QString username, QString server, QString port, QString password, QString version, QString channelIdBase)
    {
        ServerInfo server_info;
        server_info.version = version;
        server_info.user_name = username;
        server_info.server = server;
        server_info.port = port;
        server_info.password = password;
        server_info.channel_name = name;
        server_info.channel_id = channelIdBase + name;
        AddChannel(name, server_info);
    }

    void Session::RemoveChannel(QString name)
    {
        if (active_channel_ == name)
        {
            SetActiveChannel("");
            emit ChannelListChanged(GetChannels());
        }

        if (channels_.contains(name))
        {
            channels_.remove(name);
            emit ChannelListChanged(GetChannels());
        }
    }

}
