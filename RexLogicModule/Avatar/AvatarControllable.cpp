// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Avatar/AvatarControllable.h"
#include "CameraControllable.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "EntityComponent/EC_OpenSimAvatar.h"
#include "RexLogicModule.h"
#include "SceneEvents.h"
#include "Entity.h"
#include "SceneManager.h"
#include "EC_OgrePlaceable.h"
#include "InputServiceInterface.h"

namespace RA = RexTypes::Actions;

namespace RexLogic
{
    uint32_t SetFPControlFlags(uint32_t control_flags, Core::Real pitch)
    {
        uint32_t net_controlflags = control_flags;

        // First person mode fly up/down automation
        if ((control_flags & RexTypes::AGENT_CONTROL_FLY) && ((control_flags & (RexTypes::AGENT_CONTROL_UP_POS|RexTypes::AGENT_CONTROL_UP_NEG)) == 0))
        {
            if ((control_flags & RexTypes::AGENT_CONTROL_AT_POS) && (pitch > Core::PI/12))
            {
                net_controlflags |= RexTypes::AGENT_CONTROL_UP_POS;
                if (pitch > Core::PI/3)
                    net_controlflags &= ~RexTypes::AGENT_CONTROL_AT_POS;
            }
            
            if ((control_flags & RexTypes::AGENT_CONTROL_AT_POS) && (pitch < -Core::PI/12))
            {
                net_controlflags |= RexTypes::AGENT_CONTROL_UP_NEG;
                if (pitch < -Core::PI/3)
                    net_controlflags &= ~RexTypes::AGENT_CONTROL_AT_POS;
            }
            if ((control_flags & RexTypes::AGENT_CONTROL_AT_NEG) && (pitch > Core::PI/12))
            {
                net_controlflags |= RexTypes::AGENT_CONTROL_UP_NEG;
                if (pitch > Core::PI/3)
                    net_controlflags &= ~RexTypes::AGENT_CONTROL_AT_NEG;
            }
            
            if ((control_flags & RexTypes::AGENT_CONTROL_AT_NEG) && (pitch < -Core::PI/12))
            {
                net_controlflags |= RexTypes::AGENT_CONTROL_UP_POS;
                if (pitch < -Core::PI/3)
                    net_controlflags &= ~RexTypes::AGENT_CONTROL_AT_NEG;
            }
        }
        return net_controlflags;
    }

    AvatarControllable::AvatarControllable(RexLogicModule *rexlogic) : 
        framework_(rexlogic->GetFramework())
      , connection_(rexlogic->GetServerConnection())
      , event_manager_(rexlogic->GetFramework()->GetEventManager())
      , rexlogic_(rexlogic)
      , net_dirty_(false)
      , net_movementupdatetime_(0.f)
      , net_updateinterval_(0.f)
      , current_state_(ThirdPerson)
      , drag_yaw_(0)
    {
        action_event_category_ = event_manager_->QueryEventCategory("Action");

        if (action_event_category_ == Core::IllegalEventCategory)
            RexLogicModule::LogError("AvatarControllable: failed to acquire action event category, controller disabled.");


        control_flags_[RA::MoveForward] = RexTypes::AGENT_CONTROL_AT_POS;
        control_flags_[RA::MoveBackward] = RexTypes::AGENT_CONTROL_AT_NEG;
        control_flags_[RA::MoveLeft] = RexTypes::AGENT_CONTROL_LEFT_POS;
        control_flags_[RA::MoveRight] = RexTypes::AGENT_CONTROL_LEFT_NEG;
        control_flags_[RA::MoveUp] = RexTypes::AGENT_CONTROL_UP_POS;
        control_flags_[RA::MoveDown] = RexTypes::AGENT_CONTROL_UP_NEG;

        rotation_sensitivity_ = framework_->GetDefaultConfig().DeclareSetting("RexAvatar", "rotation_speed", 1.1f);
        Core::Real updates_per_second = framework_->GetDefaultConfig().DeclareSetting("RexAvatar", "updates_per_second", 20.0f);
        if (updates_per_second <= 0.f) updates_per_second = 1.f;
        net_updateinterval_ = 1.0f / updates_per_second;
    }
        
    bool AvatarControllable::HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Scene::Events::EVENT_CONTROLLABLE_ENTITY)
        {
            //! \todo this is where our user agent model design breaks. We store a single controllable entity, but we should be able to handle many. -cm
            entity_ = checked_static_cast<Scene::Events::EntityEventData*>(data)->entity;
            Scene::EntityPtr entity = entity_.lock();

            assert (entity && "Received event EVENT_CONTROLLABLE_ENTITY with null entity.");
            component_ = entity->GetComponent(EC_Controllable::NameStatic());
            Foundation::ComponentPtr component = component_.lock();

            assert (component && "Received event EVENT_CONTROLLABLE_ENTITY with null controllable component.");
            EC_Controllable *controllable = checked_static_cast<EC_Controllable*>(component.get());

            /// \bug Aren't we supposed to be able to post the ControllableEntity multiple times for the same entity to set the currently
            /// active controlled entity? This code will stack up the actions here? Or should we have different messages in the style of
            /// EVENT_MAKE_ENTITY_A_CONTROLLABLE_ENTITY and EVENT_SET_AS_CURRENT_CONTROLLABLE_ENTITY? -jj.
            input_events_ = Actions::AssignCommonActions(controllable);
            controllable->AddAction(RA::FlyMode);
            controllable->SetType(CT_AVATAR);
            input_events_[Input::Events::TOGGLE_FLYMODE] = RA::FlyMode;
            input_events_[Input::Events::TOGGLE_FLYMODE_REL] = RA::FlyMode + 1;
        }

        return false;
    }

    bool AvatarControllable::HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        // switch between first and third person modes

        if (event_id == Input::Events::INPUTSTATE_THIRDPERSON && current_state_ != ThirdPerson)
        {
            current_state_ = ThirdPerson;

            Scene::EntityPtr entity = entity_.lock();
            if (entity)
            {
                //! \todo this is where our user agent model design breaks. We use single controllable entity, but we should be able to handle many. -cm
                EC_OpenSimAvatar *avatar = checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent(EC_OpenSimAvatar::NameStatic()).get());
                avatar->controlflags &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
                avatar->controlflags &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
                net_dirty_ = true;
            }
        }

        if (event_id == Input::Events::INPUTSTATE_FIRSTPERSON && current_state_ != FirstPerson)
        {
            current_state_ = FirstPerson;

            Scene::EntityPtr entity = entity_.lock();
            if (entity)
            {
                //! \todo this is where our user agent model design breaks. We use single controllable entity, but we should be able to handle many. -cm
                EC_OpenSimAvatar *avatar = checked_static_cast<EC_OpenSimAvatar*>(entity->GetComponent(EC_OpenSimAvatar::NameStatic()).get());
                avatar->yaw = 0;
                net_dirty_ = true;
            }            
        }

        if (event_id == Input::Events::INPUTSTATE_FREECAMERA && current_state_ != InActive)
        {
            current_state_ = InActive;
        }

        // send action events corresponding to input events
        RA::ActionInputMap::const_iterator it = input_events_.find(event_id);
        if (it != input_events_.end())
        {
            Scene::Events::EntityEventData event_data;
            event_data.entity = entity_.lock();
            if (event_data.entity) // only send the event if we have an existing entity, no point otherwise
                event_manager_->SendEvent(action_event_category_, it->second, &event_data);
        }

        return false;
    }

    bool AvatarControllable::HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        Scene::Events::EntityEventData *entity_data = dynamic_cast<Scene::Events::EntityEventData*>(data);
        if (!entity_data) // a bit of a hax, we need to watchout as different action events contain different data
            return false;

        assert (entity_data->entity && "Action event received without valid entity!");


        Foundation::ComponentPtr component = entity_data->entity->GetComponent(EC_Controllable::NameStatic());
        if (IsAvatar(component))
        {
            EC_OpenSimAvatar *avatar = checked_static_cast<EC_OpenSimAvatar*>(entity_data->entity->GetComponent(EC_OpenSimAvatar::NameStatic()).get());

            if (event_id == RexTypes::Actions::FlyMode)
            {
                // flying is a special case as it is a mode that is switched on /off by keypress
                avatar->controlflags ^= RexTypes::AGENT_CONTROL_FLY;
            } else
            {
                ActionControlFlagMap::const_iterator it = control_flags_.find(event_id);
                if (it != control_flags_.end())
                {
                    // Set flag
                    avatar->controlflags |= it->second;
                } else
                {
                    it = control_flags_.find(event_id - 1);
                    if (it != control_flags_.end())
                    {
                        // Unset flag
                        avatar->controlflags &= ~it->second;
                    } else
                    {
                        // No control flags for rotation, do as special cases

                        switch (event_id)
                        {
                        case RA::RotateLeft:
                            avatar->yaw = -1;
                            break;
                        case RA::RotateRight:
                            avatar->yaw = 1;
                            break;
                        case RA::RotateLeft + 1:
                        case RA::RotateRight + 1:
                            avatar->yaw = 0;
                            break;
                        }
                    }
                }
            }
            net_dirty_ = true;
        }
        return false;
    }

    void AvatarControllable::AddTime(Core::f64 frametime)
    {
        if (current_state_ == InActive)
            return;

        boost::shared_ptr<Input::InputServiceInterface> input = framework_->GetService<Input::InputServiceInterface>(Foundation::Service::ST_Input).lock();
        if (input)
        {
            boost::optional<const Input::Events::Movement&> movement = input->PollSlider(Input::Events::MOUSELOOK);
            if (movement)
            {
                drag_yaw_ = static_cast<Core::Real>(movement->x_.rel_) * -0.005f;
                net_dirty_ = true;
            } else if (drag_yaw_ != 0)
            {
                drag_yaw_ = 0;
                net_dirty_ = true;
            }
        }

        //! \todo for simplicity, we just go over all entities in the scene. For performance, some other solution may be prudent
        Scene::ScenePtr scene = framework_->GetScene("World");
        Scene::SceneManager::iterator it = scene->begin();
        Foundation::ComponentPtr component;
        for ( ; it != scene->end() ; ++it)
        {
            component = (*it)->GetComponent(EC_Controllable::NameStatic());
            if (IsAvatar(component))
            {
                EC_OpenSimAvatar *avatar = checked_static_cast<EC_OpenSimAvatar*>((*it)->GetComponent(EC_OpenSimAvatar::NameStatic()).get());
                if (avatar->yaw != 0 || drag_yaw_ != 0)
                {
                    EC_NetworkPosition *netpos = checked_static_cast<EC_NetworkPosition*>((*it)->GetComponent(EC_NetworkPosition::NameStatic()).get());

                    Core::Quaternion rotchange(0, 0, (-avatar->yaw * (Core::Real)frametime + drag_yaw_) * rotation_sensitivity_);
                    netpos->orientation_ = rotchange * netpos->orientation_;
                    netpos->Updated();

                    net_dirty_ = true;
                }

                //! \todo hax to get camera pitch. Should be fixed once camera is a proper entity and component. -cm
                Core::Real pitch = rexlogic_->GetCameraControllable()->GetPitch();
                
                uint32_t net_controlflags = SetFPControlFlags(avatar->controlflags, pitch);
                if (net_controlflags != avatar->cached_controlflags)
                    net_dirty_ = true;

                avatar->cached_controlflags = net_controlflags;

                SendScheduledMovementToServer(net_controlflags);
            }
        }

        net_movementupdatetime_ += (Core::Real)frametime;
    }

    const Core::Quaternion &AvatarControllable::GetBodyRotation() const
    {
        Scene::EntityPtr avatarentity = entity_.lock();
        if(avatarentity)
        {
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity->GetComponent(EC_NetworkPosition::NameStatic()).get());
            return netpos.orientation_;
        }

        return Core::Quaternion::IDENTITY;
    }

    void AvatarControllable::SendMovementToServer(Core::uint controlflags)
    {
        //! \todo this is more or less where our user agent model design breaks. We can have multiple controllables, but the update for them all goes through here. -cm

        // 0 = walk, 1 = mouselook, 2 = type
        uint8_t flags = 0;
        
        Core::Quaternion bodyrot = GetBodyRotation();
        Core::Quaternion headrot = bodyrot;

        RexTypes::Vector3 camcenter = Vector3::ZERO;
        RexTypes::Vector3 camataxis = Vector3::ZERO;
        RexTypes::Vector3 camleftaxis = Vector3::ZERO;
        RexTypes::Vector3 camupaxis = Vector3::ZERO;  
        float fardist = 4000.0f;
        
        connection_->SendAgentUpdatePacket(bodyrot,headrot,0,camcenter,camataxis,camleftaxis,camupaxis,fardist,controlflags,flags);
    }

    void AvatarControllable::SendScheduledMovementToServer(Core::uint controlflags)
    {
        if (!net_dirty_)
            return;
        if (net_movementupdatetime_ < net_updateinterval_)
            return;

        net_movementupdatetime_ = 0.f;
        net_dirty_ = false;

        SendMovementToServer(controlflags);
    }

    void AvatarControllable::HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat)
    {
        //! \todo this is more or less where our user agent model design breaks. We can have multiple controllables, but this function can handle exactly one controllable. -cm

        Scene::EntityPtr avatarentity = entity_.lock();
        if(!avatarentity)
            return;
            
        // set position/rotation according to the value from server
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity->GetComponent(EC_NetworkPosition::NameStatic()).get());

        //! \todo handle lookat to set initial avatar orientation
        netpos.SetPosition(position);
        netpos.Updated();    
    }    

	void AvatarControllable::SetYaw(Core::Real newyaw)
	{
		//keys left/right set to -1/1 .. but this can use fractions too, right?
		//and is seeminly not overridden by anything at least in AddTime.
        //XXX ! \todo for simplicity, we just go over all entities in the scene. For performance, some other solution may be prudent
        Scene::ScenePtr scene = framework_->GetScene("World");
        Scene::SceneManager::iterator it = scene->begin();
        Foundation::ComponentPtr component;
        for ( ; it != scene->end() ; ++it)
        {
            component = (*it)->GetComponent(EC_Controllable::NameStatic());
            if (IsAvatar(component))
            {
                EC_OpenSimAvatar *avatar = checked_static_cast<EC_OpenSimAvatar*>((*it)->GetComponent(EC_OpenSimAvatar::NameStatic()).get());
                avatar->yaw = newyaw;
			}
		}
		net_dirty_ = true;
	}

	void AvatarControllable::SetRotation(Core::Quaternion newrot)
	{
		std::cout << "AvatarControllable::SetRotation" << std::endl;
		Scene::ScenePtr scene = framework_->GetScene("World");
        Scene::SceneManager::iterator it = scene->begin();
        Foundation::ComponentPtr component;
        for ( ; it != scene->end() ; ++it)
        {
            component = (*it)->GetComponent(EC_Controllable::NameStatic());
            if (IsAvatar(component))
            {
				EC_NetworkPosition *netpos = checked_static_cast<EC_NetworkPosition*>((*it)->GetComponent(EC_NetworkPosition::NameStatic()).get());
				netpos->orientation_ = newrot * netpos->orientation_;
				netpos->Updated();
			}
		}


        net_dirty_ = true;
	}
}

