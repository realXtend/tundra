// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Session.h"
#include "ServerInfo.h"
#include "PCMAudioFrame.h"
#include "Vector3D.h"
#include "EC_Placeable.h" // for avatar position
#ifdef ENABLE_TAIGA_SUPPORT
#include "EC_OpenSimPresence.h" // for avatar position
#include "WorldLogicInterface.h" // for avatar position
#endif
#include "ModuleManager.h"    // for avatar info
#include "Entity.h" // for avatar position
#include "SceneAPI.h"
#include "SceneManager.h"
#include "User.h"
#include "Channel.h"
#include "Connection.h"
#include "Participant.h"
#include "MumbleLibrary.h"
#include "MumbleVoipModule.h"
#include "Settings.h"

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    const double Session::DEFAULT_AUDIO_QUALITY_ = 0.5; // 0 .. 1.0
    Session::Session(Foundation::Framework* framework, Settings* settings) : 
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
        current_mumble_channel_ = server_info.channel_id;
        server_address_ = server_info.server;

        connect(connection_, SIGNAL(UserJoinedToServer(MumbleLib::User*)), SLOT(CreateNewParticipant(MumbleLib::User*)) );
        connect(connection_, SIGNAL(UserLeftFromServer(MumbleLib::User*)), SLOT(UpdateParticipantList()) );
        connect(connection_, SIGNAL(StateChanged(MumbleLib::Connection::State)), SLOT(CheckConnectionState()));

        connection_->Join(server_info.channel_id);
        connection_->SendAudio(sending_audio_);
        connection_->SetEncodingQuality(DEFAULT_AUDIO_QUALITY_);
        connection_->SendPosition(true); 
        connection_->SendAudio(audio_sending_enabled_);
        connection_->ReceiveAudio(audio_receiving_enabled_);
        
        EnableAudioReceiving();
        if (settings_->GetDefaultVoiceMode() == Settings::ContinuousTransmission)
            EnableAudioSending();
        else
            DisableAudioSending();

        MumbleLib::MumbleLibrary::Start();
        state_ = STATE_OPEN;
        emit StateChanged(state_);
    }

    void Session::Close()
    {
        if (connection_)
            connection_->Close();

        if (state_ != STATE_CLOSED && state_ != STATE_ERROR)
        {
            State old_state = state_;
            state_ = STATE_CLOSED;
            if (old_state != state_)
                emit StateChanged(state_);
        }
        foreach(Participant* p, participants_)
        {
            SAFE_DELETE(p);
        }
        participants_.clear();
        foreach(Participant* p, left_participants_)
        {
            SAFE_DELETE(p);
        }
        left_participants_.clear();
    }

    Communications::InWorldVoice::SessionInterface::State Session::GetState() const
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

    QList<Communications::InWorldVoice::ParticipantInterface*> Session::Participants() const
    {
        QList<Communications::InWorldVoice::ParticipantInterface*> list;
        for(ParticipantList::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            Participant* p = *i;
            list.append(static_cast<Communications::InWorldVoice::ParticipantInterface*>(p));
        }
        return list;
    }

    QStringList Session::GetParticipantsNames() const
    {
        QStringList names;
        QList<Communications::InWorldVoice::ParticipantInterface*> list;
        for(ParticipantList::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            Participant* p = *i;
            names << p->Name();
        }
        return names;
    }

    void Session::MuteParticipantByName(QString name, bool mute) const
    {
        QList<Communications::InWorldVoice::ParticipantInterface*> list;
        for(ParticipantList::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
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

        bool avatar_found = false;
        QString uuid = user->Comment();
#ifdef ENABLE_TAIGA_SUPPORT
        QString avatar_name = GetAvatarFullName(uuid);
#else
        QString avatar_name = ""; ///\todo Reimplement. This logic has never actually worked in Tundra. -jj.
#endif

        if (avatar_name.size() > 0)
            avatar_found = true;

        if (avatar_found)
            avatar_name = user->Name();
        else
            avatar_name = QString("%0 (no avatar)").arg(user->Name());
        avatar_name.replace('_', ' ');        

        Participant* p = new Participant(avatar_name, user);
        if (avatar_found)
            p->SetAvatarUUID(uuid);
        participants_.append(p);
        connect(p, SIGNAL(StartSpeaking()), SLOT(OnUserStartSpeaking()) );
        connect(p, SIGNAL(StopSpeaking()), SLOT(OnUserStopSpeaking()) );
        connect(p, SIGNAL(Left()), SLOT(UpdateParticipantList()) );

        emit ParticipantJoined((Communications::InWorldVoice::ParticipantInterface*)p);
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
                    left_participants_.push_back(p);
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
            if (p->AvatarUUID().length() == 0)
            {
                QString uuid = p->UserPtr()->Comment();
                if (uuid.length() > 0)
                {
#ifdef ENABLE_TAIGA_SUPPORT
                    QString avatar_name = GetAvatarFullName(uuid);
#else
        QString avatar_name = ""; ///\todo Reimplement. This logic has never actually worked in Tundra. -jj.
#endif
                    if (avatar_name.size() > 0)
                    {
                        p->SetAvatarUUID(uuid);
                        QString avatar_name = p->UserPtr()->Name();
                        avatar_name.replace('_', ' ');        
                        p->SetName(avatar_name);
                    }
                }
            }
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
        const short max = 100; /// \todo Use more proper treshold value
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
            emit Communications::InWorldVoice::SessionInterface::SpeakerVoiceActivityChanged(activity);
    }

#ifdef ENABLE_TAIGA_SUPPORT
    bool Session::GetOwnAvatarPosition(Vector3df& position, Vector3df& direction)
    {
        using namespace Foundation;
        boost::shared_ptr<WorldLogicInterface> world_logic = framework_->GetServiceManager()->GetService<WorldLogicInterface>(Service::ST_WorldLogic).lock();
        if (!world_logic)
            return false;

        Scene::EntityPtr user_avatar = world_logic->GetUserAvatarEntity();
        if (!user_avatar)
            return false;

        boost::shared_ptr<EC_Placeable> ogre_placeable = user_avatar->GetComponent<EC_Placeable>();
        if (!ogre_placeable)
            return false;

        Quaternion q = ogre_placeable->GetOrientation();
        position = ogre_placeable->GetPosition(); 
        direction = q*Vector3df::UNIT_Z;

        return true;
    }

    QString Session::OwnAvatarId()
    {
        using namespace Foundation;
        boost::shared_ptr<WorldLogicInterface> world_logic = framework_->GetServiceManager()->GetService<WorldLogicInterface>(Service::ST_WorldLogic).lock();
        if (!world_logic)
            return "";

        Scene::EntityPtr user_avatar = world_logic->GetUserAvatarEntity();
        if (!user_avatar)
            return "";

        boost::shared_ptr<EC_OpenSimPresence> opensim_presence = user_avatar->GetComponent<EC_OpenSimPresence>();
        if (!opensim_presence)
            return "";

        return opensim_presence->agentId.ToQString();
    }

    QString Session::GetAvatarFullName(QString uuid) const
    {
        Scene::ScenePtr current_scene = framework_->Scene()->GetDefaultScene();
        if (current_scene)
        {
            for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
            {
                Scene::Entity &entity = *iter->second;
                EC_OpenSimPresence *presence_component = entity.GetComponent<EC_OpenSimPresence>().get();
                if (!presence_component)
                    continue;
                if (presence_component->agentId.ToQString() == uuid)
                {
                    QString name = ""; 
                    name = presence_component->GetFullName();
                    if (name.length() == 0)
                        name = presence_component->getfirstName();
                    return name;
                }
            }
        }
        return "";
    }
#endif

    void Session::SendRecordedAudio()
    {
        if (!framework_->Audio())
            return;

        if (!connection_)
            return;

        Vector3df avatar_position;
        Vector3df avatar_direction;
#ifdef ENABLE_TAIGA_SUPPORT
        GetOwnAvatarPosition(avatar_position, avatar_direction);
#else
        avatar_position = Vector3df(0,0,0);
        avatar_direction = Vector3df(1,0,0);
#endif

        while(framework_->Audio()->GetRecordedSoundSize() > SAMPLES_IN_FRAME*SAMPLE_WIDTH/8)
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
                connection_->SendAudioFrame(frame, avatar_position);
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
            if (state_ == STATE_OPEN)
            {
                // connection lost
            }
            break;
        }
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
        return active_channel_;
    }

    void Session::SetActiveChannel(QString channel_name) 
    {
        if (active_channel_ == channel_name)
            return;

        if (!channels_.contains(channel_name))
        {
            active_channel_ = "";
            LogInfo(QString("Active voice channel changed to: %1").arg(active_channel_).toStdString());
            emit Communications::InWorldVoice::SessionInterface::ActiceChannelChanged(active_channel_);
            Close();
            return;
        }

        if (GetState() != STATE_CLOSED)
            Close();

        ServerInfo server_info = channels_[channel_name];
        OpenConnection(server_info);
        active_channel_ = channel_name;
        LogInfo(QString("Active voice channel changed to: %1").arg(active_channel_).toStdString());
        emit Communications::InWorldVoice::SessionInterface::ActiceChannelChanged(channel_name);
    }

    QStringList Session::GetChannels()
    {
        return QStringList(channels_.keys());
    }

    void Session::AddChannel(QString name, const ServerInfo &server_info)
    {
        channels_[name] = server_info;
        emit Communications::InWorldVoice::SessionInterface::ChannelListChanged(GetChannels());
    }

    void Session::AddChannel(QString name, QString username, QString server, QString password, QString version, QString channelIdBase)
    {
        ServerInfo server_info;
        server_info.version = version;
        server_info.user_name = username;
        server_info.server = server;
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
            emit Communications::InWorldVoice::SessionInterface::ChannelListChanged(GetChannels());
        }

        if (channels_.contains(name))
        {
            channels_.remove(name);
            emit Communications::InWorldVoice::SessionInterface::ChannelListChanged(GetChannels());
        }
    }

} // MumbleVoip
