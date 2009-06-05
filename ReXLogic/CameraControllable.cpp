
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraControllable.h"
#include "EC_NetworkPosition.h"
#include "SceneEvents.h"
#include "Entity.h"
#include "SceneManager.h"
#include "EC_OgrePlaceable.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
//#include "RexTypes.h"
#include "InputEvents.h"
#include "InputServiceInterface.h"
#include <Ogre.h>


namespace RexLogic
{
    CameraControllable::CameraControllable(Foundation::Framework *fw) : 
        framework_(fw)
      , action_event_category_(fw->GetEventManager()->QueryEventCategory("Action"))
      , current_state_(ThirdPerson)
      , firstperson_pitch_(0)
      , drag_pitch_(0)
    {
        camera_distance_ = framework_->GetDefaultConfig().DeclareSetting("Camera", "default_distance", 20.f);
        camera_min_distance_ = framework_->GetDefaultConfig().DeclareSetting("Camera", "min_distance", 1.f);
        camera_max_distance_ = framework_->GetDefaultConfig().DeclareSetting("Camera", "max_distance", 50.f);

        
        camera_offset_ = Core::ParseString<Core::Vector3df>(
            framework_->GetDefaultConfig().DeclareSetting("Camera", "third_person_offset", Core::ToString(Core::Vector3df(0, 0, 1.8f))));
        
        camera_offset_firstperson_ = Core::ParseString<Core::Vector3df>(
            framework_->GetDefaultConfig().DeclareSetting("Camera", "first_person_offset", Core::ToString(Core::Vector3df(0.5f, 0, 0.8f))));

        zoom_sensitivity_ = framework_->GetDefaultConfig().DeclareSetting("Camera", "zoom_sensitivity", 0.015f);
        firstperson_sensitivity_ = framework_->GetDefaultConfig().DeclareSetting("RexAvatar", "mouselook_rotation_sensitivity", 1.3f);

        head_bone_ = framework_->GetDefaultConfig().DeclareSetting<std::string>("RexAvatar", "headbone_name", "Bip01_Head");
    }

    bool CameraControllable::HandleSceneEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        //! \todo This is where our user agent model design breaks down. We assume only one controllable entity exists and that it is a target for the camera.
        //!       Should be changed so in some way the target can be changed and is not automatically assigned. -cm
        if (event_id == Scene::Events::EVENT_CONTROLLABLE_ENTITY)
            target_entity_ = checked_static_cast<Scene::Events::EntityEventData*>(data)->entity;
        
        return false;
    }

    bool CameraControllable::HandleInputEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == Input::Events::SCROLL)
        {
            CameraZoomEvent event_data;
            //event_data.entity = entity_.lock(); // no entity for camera, :( -cm
            event_data.amount = checked_static_cast<Input::Events::SingleAxisMovement*>(data)->z_.rel_;
            //if (event_data.entity) // only send the event if we have an existing entity, no point otherwise
            framework_->GetEventManager()->SendEvent(action_event_category_, RexTypes::Actions::Zoom, &event_data);
        }

        return false;
    }

    bool CameraControllable::HandleActionEvent(Core::event_id_t event_id, Foundation::EventDataInterface* data)
    {
        if (event_id == RexTypes::Actions::Zoom)
        {
            Core::Real value = checked_static_cast<CameraZoomEvent*>(data)->amount;

            camera_distance_ -= (value * zoom_sensitivity_);
            camera_distance_ = Core::clamp(camera_distance_, camera_min_distance_, camera_max_distance_);
            return true;
        }
        return false;
    }

    void CameraControllable::AddTime(Core::f64 frametime)
    {
        boost::shared_ptr<Input::InputServiceInterface> input = framework_->GetService<Input::InputServiceInterface>(Foundation::Service::ST_Input).lock();
        if (input)
        {
            boost::optional<const Input::Events::Movement&> movement = input->PollSlider(Input::Events::MOUSELOOK);
            if (movement)
            {
                drag_pitch_ = static_cast<Core::Real>(movement->y_.rel_) * -0.005f;
            } else if (drag_pitch_ != 0)
            {
                drag_pitch_ = 0;
            }
        }

        boost::shared_ptr<OgreRenderer::Renderer> renderer = framework_->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        Scene::EntityPtr target = target_entity_.lock();

        if (renderer && target)
        {
            Ogre::Camera *camera = renderer->GetCurrentCamera();

            // for smoothness, we apparently need to get rotation from network position and position from placeable. Go figure. -cm
            EC_NetworkPosition *netpos = checked_static_cast<EC_NetworkPosition*>(target->GetComponent(EC_NetworkPosition::NameStatic()).get());
            OgreRenderer::EC_OgrePlaceable *placeable = 
                checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(target->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()).get());
            if (netpos && placeable)
            {
                Core::Vector3df avatar_pos = placeable->GetPosition();
                Core::Quaternion avatar_orientation = netpos->rotation_; 

                Core::Vector3df pos = avatar_pos;
                pos += (avatar_orientation * Core::Vector3df::NEGATIVE_UNIT_X * camera_distance_);
                pos += (avatar_orientation * camera_offset_);
                camera->setPosition(pos.x, pos.y, pos.z);
            
                Core::Vector3df lookat = avatar_pos + avatar_orientation * camera_offset_;
                camera->lookAt(lookat.x, lookat.y, lookat.z);

                if (current_state_ == FirstPerson)
                {
                    bool fallback = true;
                    // Try to use head bone from target entity to get the first person camera position
                    Foundation::ComponentPtr mesh_ptr = target->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
                    if (mesh_ptr)
                    {
                        OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh_ptr.get());
                        Ogre::Entity* ent = mesh.GetEntity();
                        if (ent)
                        {
                            Ogre::SkeletonInstance* skel = ent->getSkeleton();
                            if (skel->hasBone(head_bone_))
                            {
                                Ogre::Bone* bone = skel->getBone(head_bone_);
                                Ogre::Vector3 headpos = bone->_getDerivedPosition();
                                Core::Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y - 0.5f);
                                RexTypes::Vector3 campos = avatar_pos + (avatar_orientation * ourheadpos);
                                camera->setPosition(campos.x, campos.y, campos.z);
                                fallback = false;
                            }
                        }
                    }
                    // Fallback using fixed position
                    if (fallback)
                    {
                        RexTypes::Vector3 campos = avatar_pos + (avatar_orientation * camera_offset_firstperson_);
                        camera->setPosition(campos.x, campos.y, campos.z);
                    }

                    // update camera pitch
                    if (drag_pitch_ != 0)
                    {
                        firstperson_pitch_ += drag_pitch_ * firstperson_sensitivity_;
                        firstperson_pitch_ = Core::clamp(firstperson_pitch_, -Core::HALF_PI, Core::HALF_PI);
                        //if (firstperson_pitch_ < -Core::HALF_PI) firstperson_pitch_ = -Core::HALF_PI;
                        //if (firstperson_pitch_ > Core::HALF_PI) firstperson_pitch_ = Core::HALF_PI;
                    }
                    camera->pitch(Ogre::Radian(firstperson_pitch_));
                }
            }
        }
        
        // switch between first and third person modes, depending on how close we are to the avatar
        switch (current_state_)
        {
        case FirstPerson:
            {
                if (camera_distance_ != camera_min_distance_)
                {
                    current_state_ = ThirdPerson;
                    Core::event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                    framework_->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
                    
                    firstperson_pitch_ = 0.0f;
                    //StopStrafing();
                }
                break;
            }
        case ThirdPerson:
            {
                if (camera_distance_ == camera_min_distance_)
                {
                    Core::event_category_id_t event_category = framework_->GetEventManager()->QueryEventCategory("Input");
                    framework_->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FIRSTPERSON, NULL);
                    current_state_ = FirstPerson;
                    
                    firstperson_pitch_ = 0.0f;
                    //StopRotating();
                }
                break;
            }
        }

        //if (cameradistance_ == camera_min_distance_)
        //{
        //    if ((!firstperson_) || (!cached))
        //    {
        //        Core::event_category_id_t event_category = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Input");
        //        rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FIRSTPERSON, NULL);
        //        firstperson_ = true;
        //        firstperson_pitch_ = 0.0f;
        //        StopRotating();
        //    }
        //}
        //else
        //{
        //    if ((firstperson_) || (!cached))
        //    {
        //        Core::event_category_id_t event_category = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Input");
        //        rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
        //        firstperson_ = false;
        //        firstperson_pitch_ = 0.0f;
        //        StopStrafing();
        //    }
        //}
    }
}

