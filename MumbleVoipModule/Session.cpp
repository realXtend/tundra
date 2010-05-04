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
#include "SceneManager.h"     // for avatar position
#include "ModuleManager.h"    // for avatar position
#include "RexLogicModule.h"   // for avatar position
#include "Avatar/Avatar.h"    // for avatar position
#include "WorldStream.h"

namespace MumbleVoip
{
    namespace InWorldVoice
    {
        Session::Session(Foundation::Framework* framework, const ServerInfo &server_info) : 
            state_(STATE_INITIALIZING),
            framework_(framework),
            description_(""),
            sending_audio_(false),
            receiving_audio_(false),
            audio_sending_enabled_(false),
            audio_receiving_enabled_(false),
            speaker_voice_activity_(0),
            connection_manager_(new ConnectionManager(framework))
        {
            connection_manager_->OpenConnection(server_info);
            connection_manager_->SendAudio(audio_sending_enabled_);
            connect(connection_manager_, SIGNAL(UserJoined(User*)), SLOT(OnUserJoined(User*)) );
            connect(connection_manager_, SIGNAL(UserLeft(User*)), SLOT(OnUserLeft(User*)) );
            connect(connection_manager_, SIGNAL(AudioFrameSent(PCMAudioFrame*)), SLOT(UpdateSpeakerActivity(PCMAudioFrame*)) );

            //SpeakerVoiceActivity
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
//            emit StartReceivingAudio();
        }

        void Session::DisableAudioReceiving()
        {
            audio_receiving_enabled_ = false;
            connection_manager_->ReceiveAudio(false);
//            emit StopReceivingAudio();
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

        void Session::OnUserJoined(User* user)
        {
            if (user->Name() == OwnAvatarId())
                return; 

            Participant* p = new Participant(user);
            participants_.append(p);
            connect((Communications::InWorldVoice::ParticipantInterface*)p, SIGNAL(StartSpeaking()), SLOT(OnUserStartSpeaking()) );
            connect(p, SIGNAL(StopSpeaking()), SLOT(OnUserStopSpeaking()) );
            connect((Communications::InWorldVoice::ParticipantInterface*)p, SIGNAL(Left()), SLOT(OnUserLeft()) );

            emit ParticipantJoined((Communications::InWorldVoice::ParticipantInterface*)p);
        }

        void Session::OnUserLeft()
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
            short max = 50; //! \todo Use more proper treshold value
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
            RexLogic::RexLogicModule *rex_logic_module = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
            if (!rex_logic_module)
                return false;

            RexLogic::AvatarPtr avatar = rex_logic_module->GetAvatarHandler();
            if (!avatar)
                return false;

            Scene::EntityPtr entity = avatar->GetUserAvatar();
            if (!entity)
                return false;

            const Foundation::ComponentInterfacePtr &placeable_component = entity->GetComponent("EC_OgrePlaceable");
            if (!placeable_component)
                return false;

            OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
            Quaternion q = ogre_placeable->GetOrientation();
            position = ogre_placeable->GetPosition(); 
            direction = q*Vector3df::UNIT_Z;

            return true;
        }

        QString Session::OwnAvatarId()
        {
            ProtocolUtilities::WorldStreamPtr world_stream = framework_->GetModuleManager()->GetModule<RexLogic::RexLogicModule>(Foundation::Module::MT_WorldLogic).lock().get()->GetServerConnection();
            return world_stream->GetInfo().agentID.ToQString();
        }

    } // InWorldVoice

} // MumbleVoip
