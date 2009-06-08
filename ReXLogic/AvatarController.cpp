// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarController.h"
#include "RexLogicModule.h"
#include "RexServerConnection.h"
#include "EventManager.h"
#include "InputEvents.h"
#include "Renderer.h"
#include "../OgreRenderingModule/EC_OgreMesh.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/OgreConversionUtils.h"
#include "EC_NetworkPosition.h"
#include "Entity.h"
#include <Ogre.h>

namespace RexLogic
{
    AvatarController::AvatarController(Foundation::Framework *framework, RexLogicModule *rexlogicmodule)
    {
        framework_ = framework;
        rexlogicmodule_ = rexlogicmodule;

        headrotation_ = Core::Quaternion(0,0,0,1);
        controlflags_ = 0;
        
        cameradistance_ = 20.0f;
        camera_min_distance_ = 1.0f;
        camera_max_distance_ = 50.0f;
        cameraoffset_ = Core::Vector3df(0,0,1.8f);
        cameraoffset_firstperson_ = Core::Vector3df(0.5f,0,0.8f);
        
        yaw_ = 0.0f;
        net_movementupdatetime_ = 0.0f;
        net_controlflags_ = 0;
        net_dirtymovement_ = false;
        
        drag_yaw_ = 0.f;
        drag_pitch_ = 0.f;
        firstperson_pitch_ = 0.f;
        firstperson_ = false;

        rot_sensitivity_ = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "mouselook_rotation_sensitivity", 1.3f);
        turn_sensitivity_ = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "thirdperson_rotation_sensitivity", 0.8f);
        head_bone_ = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting<std::string>("RexAvatar", "headbone_name", "Bip01_Head");
        float updates_per_second = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "updates_per_second", 20.0f);
        if (updates_per_second <= 0.0) updates_per_second = 1.0;
        net_updateinterval_ = 1.0 / updates_per_second;
    }

    AvatarController::~AvatarController()
    {
    }
    
    void AvatarController::SetAvatarEntity(Scene::EntityPtr avatar)
    {
        avatarentity_ = avatar;
    }

    void AvatarController::StartMovingForward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_POS;
    }

    void AvatarController::StopMovingForward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
    }

    void AvatarController::StartMovingBackward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_NEG;
    }

    void AvatarController::StopMovingBackward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
    }

    void AvatarController::StartMovingLeft()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_POS;
    }

    void AvatarController::StopMovingLeft()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
    }

    void AvatarController::StartMovingRight()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_NEG;
    }

    void AvatarController::StopMovingRight()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
    }
    
    
    void AvatarController::StartMovingUp()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_UP_POS;
    }

    void AvatarController::StopMovingUp()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_UP_POS;
    }

    void AvatarController::StartMovingDown()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_UP_NEG;
    }

    void AvatarController::StopMovingDown()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_UP_NEG;
    }
    
    void AvatarController::ToggleFlyMode()
    {
        controlflags_ ^= RexTypes::AGENT_CONTROL_FLY;
    }

    void AvatarController::Drag(const Input::Events::Movement *movement)
    {
        drag_yaw_ = static_cast<float>(movement->x_.rel_) * -0.2f;
        drag_pitch_ = static_cast<float>(movement->y_.rel_) * -0.2f;
    }

    void AvatarController::Zoom(int value) 
    {    
        cameradistance_ -= (value*0.015f);
        cameradistance_ = std::max(camera_min_distance_, std::min(camera_max_distance_,cameradistance_));
        
        CheckMode(true);
    }

    void AvatarController::StartRotatingLeft()
    {
        yaw_ = 1;
    }
    
    void AvatarController::StopRotatingLeft()
    {
        yaw_ = 0;
    }
    
    void AvatarController::StartRotatingRight()
    {
        yaw_ = -1;
    }
    
    void AvatarController::StopRotatingRight()
    {
        yaw_ = 0;
    }

    void AvatarController::StopRotating()
    {
        yaw_ = 0;
    }
    
    void AvatarController::StopStrafing()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
        net_dirtymovement_ = true;
    }
    
    Core::Quaternion AvatarController::GetBodyRotation()
    {
        if(avatarentity_.lock())
        {
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_.lock()->GetComponent(EC_NetworkPosition::NameStatic()).get());
            return netpos.rotation_;      
        }
        else
            return Core::Quaternion(0,0,0,1);
    }

    void AvatarController::SetBodyRotation(Core::Quaternion rotation)
    {
        if(avatarentity_.lock())
        {
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_.lock()->GetComponent(EC_NetworkPosition::NameStatic()).get());
            netpos.rotation_ = rotation;    
            netpos.Updated();
            
            net_dirtymovement_ = true;
        }
    }

    void AvatarController::SendMovementToServer()
    {
        if (!net_dirtymovement_)
            return;
        if (net_movementupdatetime_ < net_updateinterval_)
            return;
        
        // 0 = walk, 1 = mouselook, 2 = type
        uint8_t flags = 0;
        if (firstperson_)
            flags = 1;
        
        Core::Quaternion bodyrot = Core::Quaternion(0,0,0,1);
        Core::Quaternion headrot = Core::Quaternion(0,0,0,1);

        if(avatarentity_.lock())
        {
            bodyrot = GetBodyRotation();
            headrot = bodyrot;
        }
        else
        {
            bodyrot.normalize();
            headrot.normalize();
        }

        RexTypes::Vector3 camcenter = Vector3(0,0,0);
        RexTypes::Vector3 camataxis = Vector3(0,0,0);
        RexTypes::Vector3 camleftaxis = Vector3(0,0,0);
        RexTypes::Vector3 camupaxis = Vector3(0,0,0);
        float fardist = 1000.0f;
        
        rexlogicmodule_->GetServerConnection()->SendAgentUpdatePacket(bodyrot,headrot,0,camcenter,camataxis,camleftaxis,camupaxis,fardist,net_controlflags_,flags);
        net_movementupdatetime_ = 0.0f;
        net_dirtymovement_ = false;
    }
    
    
    void AvatarController::Update(Core::f64 frametime)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer && avatarentity_.lock())
        {
            Ogre::Camera *camera = renderer->GetCurrentCamera();
            OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_.lock()->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()).get());

            if (!firstperson_)
            {
                drag_yaw_ = 0.0;
                drag_pitch_ = 0.0;
            }
            else
            {
                // update camera pitch
                if (drag_pitch_ != 0)
                {
                    firstperson_pitch_ += Ogre::Degree(drag_pitch_ * rot_sensitivity_).valueRadians();
                    if (firstperson_pitch_ < -Core::PI/2) firstperson_pitch_ = -Core::PI/2;
                    if (firstperson_pitch_ > Core::PI/2) firstperson_pitch_ = Core::PI/2;
                }
            }
            
            // update body rotation
            if ((yaw_ != 0) ||(drag_yaw_ != 0))
            {
                Core::Quaternion rotchange;
                rotchange.fromAngleAxis((yaw_*frametime* turn_sensitivity_ + Ogre::Degree(drag_yaw_ * rot_sensitivity_).valueRadians()),RexTypes::Vector3(0,0,1)); 
                
                Core::Quaternion newrot = rotchange * GetBodyRotation();
                SetBodyRotation(newrot.normalize());
            }

            // update camera position
            RexTypes::Vector3 campos = ogreplaceable.GetPosition();
            campos += (GetBodyRotation() * RexTypes::Vector3(-1,0,0) * cameradistance_);
            campos += (GetBodyRotation() * cameraoffset_);
            camera->setPosition(campos.x,campos.y,campos.z);
            
            RexTypes::Vector3 lookat = ogreplaceable.GetPosition();
            lookat += (ogreplaceable.GetOrientation() * cameraoffset_);
            camera->lookAt(lookat.x,lookat.y,lookat.z);
                
            if (firstperson_)
            {
                bool fallback = true;
                // Try to use head bone from avatar to get the first person camera position
                Foundation::ComponentPtr mesh_ptr = avatarentity_.lock()->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic());
                if (mesh_ptr)
                {
                    OgreRenderer::EC_OgreMesh& mesh = *checked_static_cast<OgreRenderer::EC_OgreMesh*>(mesh_ptr.get());
                    Ogre::Entity* ent = mesh.GetEntity();
                    if (ent)
                    {
                        Ogre::SkeletonInstance* skel = ent->getSkeleton();
                        if (skel->hasBone(head_bone_))
                        {
                            //! \todo the bone position never updates if the entity is not in camera frustum
                            Ogre::Bone* bone = skel->getBone(head_bone_);
                            Ogre::Vector3 headpos = bone->_getDerivedPosition();
                            Core::Vector3df ourheadpos(-headpos.z + 0.5f, -headpos.x, headpos.y - 0.5f);
                            RexTypes::Vector3 campos = ogreplaceable.GetPosition();
                            campos += GetBodyRotation() * ourheadpos;
                            camera->setPosition(campos.x,campos.y,campos.z);
                            fallback = false;
                        }
                    }
                }
                
                // Fallback using fixed position
                if (fallback)
                {
                    RexTypes::Vector3 campos = ogreplaceable.GetPosition();
                    campos += (GetBodyRotation() * cameraoffset_firstperson_);
                    camera->setPosition(campos.x,campos.y,campos.z);
                }            
                camera->pitch(Ogre::Radian(firstperson_pitch_));
            }
        }
        
        net_movementupdatetime_ += frametime;
        SetNetControlFlags();
        SendMovementToServer();
    }
    
    void AvatarController::HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat)
    {
        Scene::EntityPtr avatarentity = avatarentity_.lock();
        if(!avatarentity)
            return;
            
        // set position/rotation according to the value from server
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity->GetComponent(EC_NetworkPosition::NameStatic()).get());

        //! \todo handle lookat to set initial avatar orientation
        
        netpos.position_ = position;
        netpos.velocity_ = Core::Vector3Df::ZERO;
        netpos.accel_ = Core::Vector3Df::ZERO;
        
        // Initial position within region, do not damp
        netpos.NoPositionDamping();
        netpos.NoRotationDamping();
        netpos.Updated();    
    }    
    
    void AvatarController::HandleNetworkUpdate(const RexTypes::Vector3& position, const Core::Quaternion& rotation)
    {
        Scene::EntityPtr avatarentity = avatarentity_.lock();
        if(!avatarentity)
            return;

        // client is authorative over own avatar rotation for now
            
        // set position according to the value from server
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity->GetComponent(EC_NetworkPosition::NameStatic()).get());

        netpos.position_ = position;        
        netpos.Updated();    
    }        
    
    void AvatarController::CheckMode(bool cached)
    {
        if (cameradistance_ == camera_min_distance_)
        {
            if ((!firstperson_) || (!cached))
            {
                Core::event_category_id_t event_category = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Input");
                rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_FIRSTPERSON, NULL);
                firstperson_ = true;
                firstperson_pitch_ = 0.0f;
                StopRotating();
            }
        }
        else
        {
            if ((firstperson_) || (!cached))
            {
                Core::event_category_id_t event_category = rexlogicmodule_->GetFramework()->GetEventManager()->QueryEventCategory("Input");
                rexlogicmodule_->GetFramework()->GetEventManager()->SendEvent(event_category, Input::Events::INPUTSTATE_THIRDPERSON, NULL);
                firstperson_ = false;
                firstperson_pitch_ = 0.0f;
                StopStrafing();
            }
        }
    }
    
    void AvatarController::SetNetControlFlags()
    {
        uint32_t oldflags = net_controlflags_;
        net_controlflags_ = controlflags_;
     
        // First person mode fly up/down automation
        if ((controlflags_ & RexTypes::AGENT_CONTROL_FLY) && (firstperson_) && ((controlflags_ & (RexTypes::AGENT_CONTROL_UP_POS|RexTypes::AGENT_CONTROL_UP_NEG)) == 0))
        {
            if ((controlflags_ & RexTypes::AGENT_CONTROL_AT_POS) && (firstperson_pitch_ > Core::PI/12))
            {
                net_controlflags_ |= RexTypes::AGENT_CONTROL_UP_POS;
                if (firstperson_pitch_ > Core::PI/3)
                    net_controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
            }
            
            if ((controlflags_ & RexTypes::AGENT_CONTROL_AT_POS) && (firstperson_pitch_ < -Core::PI/12))
            {
                net_controlflags_ |= RexTypes::AGENT_CONTROL_UP_NEG;
                if (firstperson_pitch_ < -Core::PI/3)
                    net_controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
            }
            if ((controlflags_ & RexTypes::AGENT_CONTROL_AT_NEG) && (firstperson_pitch_ > Core::PI/12))
            {
                net_controlflags_ |= RexTypes::AGENT_CONTROL_UP_NEG;
                if (firstperson_pitch_ > Core::PI/3)
                    net_controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
            }
            
            if ((controlflags_ & RexTypes::AGENT_CONTROL_AT_NEG) && (firstperson_pitch_ < -Core::PI/12))
            {
                net_controlflags_ |= RexTypes::AGENT_CONTROL_UP_POS;
                if (firstperson_pitch_ < -Core::PI/3)
                    net_controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
            }
        }
        
        if (net_controlflags_ != oldflags)
            net_dirtymovement_ = true;
    }
}

