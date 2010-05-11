#include "StableHeaders.h"
#include "Session.h"
#include "MemoryLeakCheck.h"
#include "Participant.h"
#include "ConnectionManager.h"
#include "ServerInfo.h"
#include "PCMAudioFrame.h"
#include "Vector3D.h"
#include "User.h"
#include "EC_OgrePlaceable.h" // for avatar position
#include "EC_OpenSimPresence.h" // for avatar position
#include "ModuleManager.h"    // for avatar info
#include "WorldLogicInterface.h" // for avatar position
#include "Entity.h" // for avatar position
#include "SceneManager.h"
#include "WorldStream.h"
#include "Channel.h"

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
            connection_manager_(new ConnectionManager(framework)),
            server_info_(server_info)
        {
            channel_name_ = server_info.channel;
            connection_manager_->OpenConnection(server_info);
            if (connection_manager_->GetState() != ConnectionManager::STATE_CONNECTION_OPEN)
            {
                state_ = STATE_ERROR;
                reason_ = connection_manager_->GetReason();
                return;
            }
            state_ = STATE_OPEN; // \todo get this information from connection_manager
            connection_manager_->SendAudio(audio_sending_enabled_);
            connect(connection_manager_, SIGNAL(UserJoined(User*)), SLOT(CreateNewParticipant(User*)) );
            connect(connection_manager_, SIGNAL(AudioFrameSent(PCMAudioFrame*)), SLOT(UpdateSpeakerActivity(PCMAudioFrame*)) );
        }

        Session::~Session()
        {
            SAFE_DELETE(connection_manager_);
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

        void Session::Close()
        {
            connection_manager_->CloseConnection(server_info_);
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
            audio_sending_enabled_ = true;
            connection_manager_->SendAudio(true);
            emit StartSendingAudio();
        }

        void Session::DisableAudioSending()
        {
            audio_sending_enabled_ = false;
            connection_manager_->SendAudio(false);
            emit StopSendingAudio();
        }

        bool Session::IsAudioSendingEnabled() const
        {
            return audio_sending_enabled_;
        }

        void Session::EnableAudioReceiving()
        {
            audio_receiving_enabled_ = true;
            connection_manager_->ReceiveAudio(true);
        }

        void Session::DisableAudioReceiving()
        {
            audio_receiving_enabled_ = false;
            connection_manager_->ReceiveAudio(false);
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
            if (connection_manager_)
            {
                Vector3df avatar_position;
                Vector3df avatar_direction;
                if (GetOwnAvatarPosition(avatar_position, avatar_direction))
                    connection_manager_->SetAudioSourcePosition(avatar_position);
                connection_manager_->Update(frametime);
            }
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

            speaker_voice_activity_ = activity;
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

    } // InWorldVoice

} // MumbleVoip
