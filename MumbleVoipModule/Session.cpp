#include "StableHeaders.h"
#include "Session.h"
//#include "MemoryLeakCheck.h"
#include "ServerInfo.h"
#include "PCMAudioFrame.h"
#include "Vector3D.h"
#include "EC_OgrePlaceable.h" // for avatar position
#include "EC_OpenSimPresence.h" // for avatar position
#include "ModuleManager.h"    // for avatar info
#include "WorldLogicInterface.h" // for avatar position
#include "Entity.h" // for avatar position
#include "SceneManager.h"
#include "WorldStream.h"
#include "User.h"
#include "Channel.h"
#include "Connection.h"
#include "Participant.h"
#include "MumbleLibrary.h"
#include "MumbleVoipModule.h"

//#define BUILDING_DLL // for dll import/export declarations
//#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
////#include <mumbleclient/client_lib.h>
//#undef BUILDING_DLL // for dll import/export declarations
////#include "LibMumbleThread.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Session::Session(Foundation::Framework* framework, const ServerInfo &server_info) : 
            state_(STATE_INITIALIZING),
            reason_(""),
            framework_(framework),
            description_(""),
            sending_audio_(false),
            receiving_audio_(false),
            audio_sending_enabled_(false),
            audio_receiving_enabled_(false),
            speaker_voice_activity_(0),
//            connection_manager_(new ConnectionManager(framework)),
            server_info_(server_info),
            connection_(0)
        {
            channel_name_ = server_info.channel;
            OpenConnection(server_info);
            

            //connection_manager_->OpenConnection(server_info);
            //if (connection_manager_->GetState() != ConnectionManager::STATE_CONNECTION_OPEN)
            //{
            //    state_ = STATE_ERROR;
            //    reason_ = connection_manager_->GetReason();
            //    return;
            //}
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
            if (connection_)
                SAFE_DELETE(connection_);
        }

        void Session::OpenConnection(ServerInfo server_info)
        {
            connection_ = new Connection(server_info);
            if (connection_->GetState() == Connection::STATE_ERROR)
            {
                state_ = STATE_ERROR;
                reason_ = connection_->GetReason();
                return;
            }
            state_ = STATE_OPEN;

            connect(connection_, SIGNAL(UserJoined(User*)), SLOT(CreateNewParticipant(User*)) );
            connect(connection_, SIGNAL(UserLeft(User*)), SLOT(UpdateUserList()) );
//            connections_[info.server] = connection;
            connection_->Join(server_info.channel);
            connection_->SendAudio(sending_audio_);
            connection_->SetEncodingQuality(0.5);
            connection_->SendPosition(true); 
            MumbleLibrary::Start();
            MumbleLibrary::Stop();
            //connect(MumbleLibrary::Instance(), SIGNAL(InteralError()), SLOT(HandleMumbleLibraryError()) );
//            connect(connection_manager_, SIGNAL(AudioFrameSent(PCMAudioFrame*)), SLOT(UpdateSpeakerActivity(PCMAudioFrame*)) );
        }

        void Session::Close()
        {
            connection_->Close();
            SAFE_DELETE(connection_);
            state_ = STATE_CLOSED;
            emit StateChanged(state_);
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
                boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
                if (sound_service.get())
                {
                    int frequency = SAMPLE_RATE;
                    bool sixteenbit = true;
                    bool stereo = false;
                    int buffer_size = SAMPLE_WIDTH/8*frequency*AUDIO_RECORDING_BUFFER_MS/1000;
                    sound_service->StartRecording(recording_device_, frequency, sixteenbit, stereo, buffer_size);
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
                boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
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
//                connection_manager_->Update(frametime);
            PlaybackReceivedAudio();
            SendRecordedAudio();
        }

        void Session::CreateNewParticipant(User* user)
        {
            if (user->Name() == OwnAvatarId())
            {
                self_user_ = user;
                return; 
            }

            if (user->Channel()->FullName() != channel_name_)
                return; 

            QString uuid = user->Name();
            QString name = GetAvatarFullName(uuid);
            if (name.size() == 0)
                name = QString("(Unknow) %0").arg(user->Name());
            Participant* p = new Participant(name, user);
            participants_.append(p);
            connect(p, SIGNAL(StartSpeaking()), SLOT(OnUserStartSpeaking()) );
            connect(p, SIGNAL(StopSpeaking()), SLOT(OnUserStopSpeaking()) );
            connect(p, SIGNAL(Left()), SLOT(UpdateParticipantList()) );

            emit ParticipantJoined((Communications::InWorldVoice::ParticipantInterface*)p);
        }

        void Session::UpdateParticipantList()
        {
            foreach(Participant* p, participants_)
            {
                if (p->UserPtr()->IsLeft())
                {
                    participants_.removeOne(p);
                    left_participants_.push_back(p);
                    emit ParticipantLeft(p);
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
            foreach(Participant* p, participants_)
            {
                if (p->IsSpeaking())
                {
                    receiving_audio_ = true;
                    return;
                }
            }
            receiving_audio_ = false;
            if (was_receiving_audio)
                emit StopReceivingAudio();
        }

        void Session::UpdateSpeakerActivity(PCMAudioFrame* frame)
        {
            static int counter;
            counter++;
            counter = counter % 10;
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

            boost::shared_ptr<OgreRenderer::EC_OgrePlaceable> ogre_placeable = user_avatar->GetComponent<OgreRenderer::EC_OgrePlaceable>();
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
                            return QString(presence_component->GetFullName().c_str());
                }
            }
            return "";
        }

        void Session::SendRecordedAudio()
        {
			if (!connection_)
				return;

            if (!audio_sending_enabled_)
                return;

            Vector3df avatar_position;
            Vector3df avatar_direction;

            boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
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
                connection_->SendAudioFrame(frame, avatar_position);
//                emit AudioFrameSent(frame);
                delete frame;
            }
        }

        void Session::PlaybackReceivedAudio()
        {
			if (!connection_)
				return;

			if (!audio_sending_enabled_)
				return;

            for(;;)
            {
                AudioPacket packet = connection_->GetAudioPacket();
                if (packet.second == 0)
                    break; // nothing to play anymore

				bool muted = false;
				foreach(Participant* participant, participants_)
				{
					if (participant->UserPtr() == packet.first && participant->IsMuted())
					{
						muted = true;
						break;
					}
				}
				if (!muted)
					PlaybackAudioFrame(packet.first, packet.second);
            }
        }

        void Session::PlaybackAudioFrame(User* user, PCMAudioFrame* frame)
        {
            boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = SoundService();
            if (!sound_service.get())
                return;    

            Foundation::SoundServiceInterface::SoundBuffer sound_buffer;
            
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

            QMutexLocker user_locker(user);
            if (audio_playback_channels_.contains(user->Session()))
                if (user->PositionKnown())
                    sound_service->PlaySoundBuffer3D(sound_buffer, Foundation::SoundServiceInterface::Voice, user->Position(), audio_playback_channels_[user->Session()]);
                else
                    sound_service->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, audio_playback_channels_[user->Session()]);
            else
                if (user->PositionKnown())
                    audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer3D(sound_buffer, Foundation::SoundServiceInterface::Voice, user->Position(), 0);
                else
                    audio_playback_channels_[user->Session()] = sound_service->PlaySoundBuffer(sound_buffer,  Foundation::SoundServiceInterface::Voice, 0);

            delete frame;
        }

        boost::shared_ptr<Foundation::SoundServiceInterface> Session::SoundService()
        {
            if (!framework_)
                return boost::shared_ptr<Foundation::SoundServiceInterface>();

            Foundation::ServiceManagerPtr service_manager = framework_->GetServiceManager();

            if (!service_manager.get())
                return boost::shared_ptr<Foundation::SoundServiceInterface>();

            boost::shared_ptr<Foundation::SoundServiceInterface> sound_service = service_manager->GetService<Foundation::SoundServiceInterface>(Foundation::Service::ST_Sound).lock();

            if (!sound_service.get())
                return boost::shared_ptr<Foundation::SoundServiceInterface>();

            return sound_service;
        }

    } // InWorldVoice

} // MumbleVoip
