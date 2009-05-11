// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarController.h"
#include "RexLogicModule.h"
#include "RexServerConnection.h"
#include "Renderer.h"
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
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
        cameraoffset_ = RexTypes::Vector3(0,1.8f,0);
        
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
            OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent("EC_OgrePlaceable").get());
            return ogreplaceable.GetOrientation();        
        }
        else
            return Core::Quaternion(0,0,0,1);
    }

    void AvatarController::SetBodyRotation(Core::Quaternion rotation)
    {
        if(avatarentity_)
        {
            OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent("EC_OgrePlaceable").get());
            ogreplaceable.SetOrientation(rotation);
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
        OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent("EC_OgrePlaceable").get());

        // update body rotation
        if(yaw_ != 0)
        {
            Core::Quaternion rotchange;
            rotchange.fromAngleAxis((yaw_*frametime*0.5f),RexTypes::Vector3(0,1,0)); 
            Core::Quaternion newrot = rotchange * ogreplaceable.GetOrientation();
            SetBodyRotation(newrot.normalize());
        }

        // update camera position
        RexTypes::Vector3 campos = ogreplaceable.GetPosition();
        campos += (ogreplaceable.GetOrientation() * RexTypes::Vector3(0,0,-1) * cameradistance_);
        campos += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->setPosition(campos.x,campos.y,campos.z);
        
        RexTypes::Vector3 lookat = ogreplaceable.GetPosition();
        lookat += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->lookAt(lookat.x,lookat.y,lookat.z);
        
        // send movement update to server if necessary
        if(net_dirtymovement_)
        {
            // send max 20 updates per second
            net_movementupdatetime_ += (float)frametime;
            if(net_movementupdatetime_ > 0.05f)
            {
                SendMovementToServer();
                net_movementupdatetime_ = 0.0f;
                net_dirtymovement_ = false;
            }        
        }        
    }
    
    void AvatarController::HandleServerObjectUpdate(RexTypes::Vector3 position, Core::Quaternion rotation)
    {
        if(!avatarentity_)
            return;

        // client is authorative over own avatar rotation for now
            
        // set position according to the value from server
        OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent("EC_OgrePlaceable").get());
        ogreplaceable.SetPosition(position);            
    }    
    
}

