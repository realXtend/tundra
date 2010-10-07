// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Session.h"
#include "ServerInfo.h"
#include "PCMAudioFrame.h"
#include "Vector3D.h"
#include "EC_Placeable.h" // for avatar position
#include "EC_OpenSimPresence.h" // for avatar position
#include "ModuleManager.h"    // for avatar info
#include "WorldLogicInterface.h" // for avatar position
#include "Entity.h" // for avatar position
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
    Session::Session(Foundation::Framework* framework, const ServerInfo &server_info, Settings* settings) : 
        state_(STATE_INITIALIZING),
        reason_(""),
        framework_(framework),
        description_(""),
        sending_audio_(false),
        receiving_audio_(false),
        audio_sending_enabled_(false),
        audio_receiving_enabled_(true),
        speaker_voice_activity_(0),
        server_info_(server_info),
        connection_(0),
        settings_(settings),
        local_echo_mode_(false),
        server_address_("")
    {
        channel_name_ = server_info.channel;
        OpenConnection(server_info);
        connect(settings_, SIGNAL(PlaybackBufferSizeMsChanged(int)), this, SLOT(SetPlaybackBufferSizeMs(int)));
        connect(settings_, SIGNAL(EncodeQualityChanged(double)), this, SLOT(SetEncodeQuality(double)));
    }

    Session::~Session()
    {
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
        SAFE_DELETE(connection_);
    }

    void Session::OpenConnection(ServerInfo server_info)
    {
        SAFE_DELETE(connection_);
        connection_ = new MumbleLib::Connection(server_info, 200);
        if (connection_->GetState() == MumbleLib::Connection::STATE_ERROR)
        {
            state_ = STATE_ERROR;
            reason_ = connection_->GetReason();
            return;
        }
        state_ = STATE_OPEN;
        server_address_ = server_info_.server;

        connect(connection_, SIGNAL(UserJoinedToServer(MumbleLib::User*)), SLOT(CreateNewParticipant(MumbleLib::User*)) );
        connect(connection_, SIGNAL(UserLeftFromServer(MumbleLib::User*)), SLOT(UpdateParticipantList()) );
        connect(connection_, SIGNAL(StateChanged(MumbleLib::Connection::State)), SLOT(CheckConnectionState()));
        connection_->Join(server_info.channel);
        connection_->SendAudio(sending_audio_);
        connection_->SetEncodingQuality(DEFAULT_AUDIO_QUALITY_);
        connection_->SendPosition(true); 
        connection_->SendAudio(audio_sending_enabled_);
        connection_->ReceiveAudio(audio_receiving_enabled_);
        
        MumbleLib::MumbleLibrary::Start();

        if (settings_->GetDefaultVoiceMode() == Settings::ContinuousTransmission)
            EnableAudioSending();
        else
            DisableAudioSending();
        EnableAudioReceiving();
    }

    void Session::Close()
    {
        if (connection_)
        {
            connection_->Close();
        }
        if (state_ != STATE_CLOSED && state_ != STATE_ERROR)
        {
            state_ = STATE_CLOSED;
            emit StateChanged(state_);
        }
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
        if (connection_)
            connection_->SendAudio(true);
        bool audio_sending_was_enabled = audio_sending_enabled_;
        audio_sending_enabled_ = true;
        if (!audio_sending_was_enabled)
        {
            boost::shared_ptr<ISoundService> sound_service = SoundService();
            if (sound_service.get())
            {
                int frequency = SAMPLE_RATE;
                bool sixteenbit = true;
                bool stereo = false;
                int buffer_size = SAMPLE_WIDTH/8*frequency*AUDIO_RECORDING_BUFFER_MS/1000;
                sound_service->StartRecording(QString::fromStdString(recording_device_), frequency, sixteenbit, stereo, buffer_size);
            }

            emit StartSendingAudio();
        }
    }

    void Session::DisableAudioSending()
    {
        if (connection_)
            connection_->SendAudio(false);
        bool audio_sending_was_enabled = audio_sending_enabled_;
        audio_sending_enabled_ = false;
        if (audio_sending_was_enabled)
        {
            boost::shared_ptr<ISoundService> sound_service = SoundService();
            if (sound_service.get())
                sound_service->StopRecording();

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

        if (user->Name() == OwnAvatarId())
        {
            self_user_ = user;
            return; 
        }

        if (user->Channel()->FullName() != channel_name_)
        {
            other_channel_users_.append(user);
            return; 
        }

        QString uuid = user->Name();
        QString name = GetAvatarFullName(uuid);
        if (name.size() == 0)
            name = QString("%0 (no avatar)").arg(user->Name());
        Participant* p = new Participant(name, user);
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

        if (user->Channel()->FullName() == channel_name_)
        {
            foreach(MumbleLib::User* u, other_channel_users_)
            {
                if (u == user)
                    other_channel_users_.removeOne(u);
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
            QString own_avatar_id_ = OwnAvatarId();
            if (p->AvatarUUID() == own_avatar_id_)
            {
                // for some reason we have own avatar as participant here!
                participants_.removeOne(p);
                other_channel_users_.append(p->UserPtr());
                emit ParticipantLeft(p);
                continue;
            }
            if (p->AvatarUUID() == p->Name().left(p->AvatarUUID().size()) || p->AvatarUUID().length() == 0)
            {
                // For some reason do not have real name for this participant here!
                QString full_name = GetAvatarFullName(p->AvatarUUID());
                if (full_name.length() > 0)
                    p->SetName( full_name );
                continue;
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
            emit Communications::InWorldVoice::SessionInterface::SpeakerVoiceActivityChanged(activity);
    }

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
        Scene::ScenePtr current_scene = framework_->GetDefaultWorldScene();
        if (current_scene.get())
        {
            for(Scene::SceneManager::iterator iter = current_scene->begin(); iter != current_scene->end(); ++iter)
            {
                Scene::Entity &entity = **iter;
                EC_OpenSimPresence *presence_component = entity.GetComponent<EC_OpenSimPresence>().get();
                if (presence_component)
                    if (presence_component->agentId.ToQString() == uuid)
                    {
                        QString name = ""; 
                        name = QString(presence_component->GetFullName().c_str());
                        if (name.length() == 0)
                            name = QString(presence_component->GetFirstName().c_str());
                        return name;
                    }
            }
        }
        return "";
    }

    void Session::SendRecordedAudio()
    {
		if (!connection_)
			return;

        Vector3df avatar_position;
        Vector3df avatar_direction;
        GetOwnAvatarPosition(avatar_position, avatar_direction);

        boost::shared_ptr<ISoundService> sound_service = SoundService();
        if (!sound_service)
        {
            MumbleVoipModule::LogDebug("Cannot record audio: Soundservice cannot be found.");
            return;
        }

        while (sound_service->GetRecordedSoundSize() > SAMPLES_IN_FRAME*SAMPLE_WIDTH/8)
        {
            int bytes_to_read = SAMPLES_IN_FRAME*SAMPLE_WIDTH/8;
            PCMAudioFrame* frame = new PCMAudioFrame(SAMPLE_RATE, SAMPLE_WIDTH, NUMBER_OF_CHANNELS, bytes_to_read );
            int bytes = sound_service->GetRecordedSoundData(frame->DataPtr(), bytes_to_read);
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
        boost::shared_ptr<ISoundService> sound_service = SoundService();
        if (!sound_service.get())
            return;    

        ISoundService::SoundBuffer sound_buffer;
        
        sound_buffer.data_.resize(frame->DataSize());
        memcpy(&sound_buffer.data_[0], frame->DataPtr(), frame->DataSize());

        sound_buffer.frequency_ = frame->SampleRate();
        if (frame->SampleWidth() == 16)
            sound_buffer.sixteenbit_ = true;
        else
            sound_buffer.sixteenbit_ = false;
        
        if (frame->Channels() == 2)
            sound_buffer.stereo_ = true;
        else
            sound_buffer.stereo_ = false;

        if (!user)
        {
            const int source_id = 0;
            if (audio_playback_channels_.contains(source_id))
                sound_service->PlaySoundBuffer(sound_buffer,  ISoundService::Voice, audio_playback_channels_[0]);
            else
                audio_playback_channels_[0] = sound_service->PlaySoundBuffer(sound_buffer,  ISoundService::Voice, 0);
        }
        else
        {
            QMutexLocker user_locker(user);
            if (audio_playback_channels_.contains(user->Session()))
                if (user->PositionKnown() && settings_->GetPositionalAudioEnabled())
                    sound_service->PlaySoundBuffer3D(sound_buffer, ISoundService::Voice, user->Position(), audio_playback_channels_[user->Session()]);
                else
                    sound_service->PlaySoundBuffer(sound_buffer,  ISoundService::Voice, audio_playback_channels_[user->Session()]);
            else
                if (user->PositionKnown() && settings_->GetPositionalAudioEnabled())
                    audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer3D(sound_buffer, ISoundService::Voice, user->Position(), 0);
                else
                    audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer(sound_buffer,  ISoundService::Voice, 0);
        }

        delete frame;
    }

    boost::shared_ptr<ISoundService> Session::SoundService()
    {
        if (!framework_)
            return boost::shared_ptr<ISoundService>();

        Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

        if (!service_manager.get())
            return boost::shared_ptr<ISoundService>();

        boost::shared_ptr<ISoundService> sound_service = service_manager->GetService<ISoundService>(Foundation::Service::ST_Sound).lock();

        if (!sound_service.get())
            return boost::shared_ptr<ISoundService>();

        return sound_service;
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

} // MumbleVoip
