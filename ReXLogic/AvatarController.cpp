// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarController.h"
#include "RexLogicModule.h"
#include "RexServerConnection.h"
#include "Renderer.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "EC_NetworkPosition.h"
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
        cameraoffset_ = RexTypes::Vector3(0,0,1.8f);
        
        yaw_ = 0.0f;
        net_movementupdatetime_ = 0.0f;
        net_dirtymovement_ = false;
    }

    AvatarController::~AvatarController()
    {
        if(avatarentity_)
            avatarentity_.reset();
    }
    
    void AvatarController::SetAvatarEntity(Scene::EntityPtr avatar)    
    {
        avatarentity_ = avatar;
    }

    void AvatarController::StartMovingForward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_POS;
        SendMovementToServer();
    }

    void AvatarController::StopMovingForward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
        SendMovementToServer();
    }

    void AvatarController::StartMovingBackward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_NEG;
        SendMovementToServer();
    }

    void AvatarController::StopMovingBackward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
        SendMovementToServer();
    }

    void AvatarController::StartMovingLeft()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_POS;
        SendMovementToServer();
    }

    void AvatarController::StopMovingLeft()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
        SendMovementToServer();
    }

    void AvatarController::StartMovingRight()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_NEG;
        SendMovementToServer();
    }

    void AvatarController::StopMovingRight()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
        SendMovementToServer();
    }
    
    
    void AvatarController::StartMovingUp()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_UP_POS;
        SendMovementToServer();
    }

    void AvatarController::StopMovingUp()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_UP_POS;
        SendMovementToServer();
    }

    void AvatarController::StartMovingDown()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_UP_NEG;
        SendMovementToServer();
    }

    void AvatarController::StopMovingDown()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_UP_NEG;
        SendMovementToServer();
    }
    

    void AvatarController::Zoom(int value) 
    {    
        cameradistance_ -= (value*0.015f);
        cameradistance_ = std::max(camera_min_distance_, std::min(camera_max_distance_,cameradistance_));

    }

    void AvatarController::StartRotatingLeft()
    {
        yaw_ = 1;
        SendMovementToServer();
    }
    
    void AvatarController::StopRotatingLeft()
    {
        yaw_ = 0;
        SendMovementToServer();
    }
    
    void AvatarController::StartRotatingRight()
    {
        yaw_ = -1;
        SendMovementToServer();
    }
    
    void AvatarController::StopRotatingRight()
    {
        yaw_ = 0;
        SendMovementToServer();
    }

    Core::Quaternion AvatarController::GetBodyRotation()
    {
        if(avatarentity_)
        {
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_->GetComponent(EC_NetworkPosition::NameStatic()).get());
            return netpos.rotation_;      
        }
        else
            return Core::Quaternion(0,0,0,1);
    }

    void AvatarController::SetBodyRotation(Core::Quaternion rotation)
    {
        if(avatarentity_)
        {
            EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_->GetComponent(EC_NetworkPosition::NameStatic()).get());
            netpos.rotation_ = rotation;    
            netpos.Updated();
            
            net_dirtymovement_ = true;         
        }
    }


    void AvatarController::SendMovementToServer()
    {
        // 0 = walk, 1 = mouselook, 2 = type
        uint8_t flags = 0;
        
        Core::Quaternion bodyrot = Core::Quaternion(0,0,0,1);
        Core::Quaternion headrot = Core::Quaternion(0,0,0,1);

        if(avatarentity_)
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
        float fardist = 4000.0f;
        
        rexlogicmodule_->GetServerConnection()->SendAgentUpdatePacket(bodyrot,headrot,0,camcenter,camataxis,camleftaxis,camupaxis,fardist,controlflags_,flags);            
    }
    
    
    void AvatarController::Update(Core::f64 frametime)
    {
        boost::shared_ptr<OgreRenderer::Renderer> renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (!renderer || !avatarentity_)
            return;

        Ogre::Camera *camera = renderer->GetCurrentCamera();
        OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent(OgreRenderer::EC_OgrePlaceable::NameStatic()).get());

        // update body rotation
        if(yaw_ != 0)
        {
            Core::Quaternion rotchange;
            rotchange.fromAngleAxis((yaw_*frametime*0.5f),RexTypes::Vector3(0,0,1)); 
            Core::Quaternion newrot = rotchange * GetBodyRotation();
            SetBodyRotation(newrot.normalize());
        }

        // hack: so that there is no lag between avatar/camera, copy orientation to ogre scenenode here, instead
        // of waiting for the general update/interpolation cycle
        ogreplaceable.SetOrientation(GetBodyRotation());

        // update camera position
        RexTypes::Vector3 campos = ogreplaceable.GetPosition();
        campos += (ogreplaceable.GetOrientation() * RexTypes::Vector3(-1,0,0) * cameradistance_);
        campos += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->setPosition(campos.x,campos.y,campos.z);
        
        RexTypes::Vector3 lookat = ogreplaceable.GetPosition();
        lookat += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->lookAt(lookat.x,lookat.y,lookat.z);           
    }
    
    void AvatarController::HandleAgentMovementComplete(const RexTypes::Vector3& position, const RexTypes::Vector3& lookat)
    {
        if(!avatarentity_)
            return;
            
        // set position/rotation according to the value from server
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_->GetComponent(EC_NetworkPosition::NameStatic()).get());

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
        if(!avatarentity_)
            return;

        // client is authorative over own avatar rotation for now
            
        // set position according to the value from server
        EC_NetworkPosition &netpos = *checked_static_cast<EC_NetworkPosition*>(avatarentity_->GetComponent(EC_NetworkPosition::NameStatic()).get());

        netpos.position_ = position;        
        netpos.Updated();    
    }        
}

