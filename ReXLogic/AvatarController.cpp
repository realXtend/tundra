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

        controlflags_ = 0;
        
        cameradistance_ = 20.0f;
        camera_min_distance_ = 1.0f;
        camera_max_distance_ = 50.0f;
        cameraoffset_ = RexTypes::Vector3(0,1.8f,0);
    }

    AvatarController::~AvatarController()
    {
        if(avatarentity_)
            avatarentity_.reset();
    }
    
    void AvatarController::SetAvatarEntity(Foundation::EntityPtr avatar)    
    {
        avatarentity_ = avatar;
    }

    Core::Quaternion AvatarController::GetBodyRotation()
    {
        return Core::Quaternion(0,0,0,1); //! \todo tucofixme
    }
    
    Core::Quaternion AvatarController::GetHeadRotation()
    {
        return Core::Quaternion(0,0,0,1); //! \todo tucofixme    
    }

    void AvatarController::StartMovingForward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_POS;
        UpdateMovementState();
    }

    void AvatarController::StopMovingForward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_POS;
        UpdateMovementState();
    }

    void AvatarController::StartMovingBackward()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_AT_NEG;
        UpdateMovementState();
    }

    void AvatarController::StopMovingBackward()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_AT_NEG;
        UpdateMovementState();
    }

    void AvatarController::StartMovingLeft()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_POS;
        UpdateMovementState();
    }

    void AvatarController::StopMovingLeft()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_POS;
        UpdateMovementState();
    }

    void AvatarController::StartMovingRight()
    {
        controlflags_ |= RexTypes::AGENT_CONTROL_LEFT_NEG;
        UpdateMovementState();
    }

    void AvatarController::StopMovingRight()
    {
        controlflags_ &= ~RexTypes::AGENT_CONTROL_LEFT_NEG;
        UpdateMovementState();
    }

    void AvatarController::Zoom(int value) 
    {    
        cameradistance_ -= (value*0.015f);
        cameradistance_ = std::max(camera_min_distance_, std::min(camera_max_distance_,cameradistance_));

    }

    void AvatarController::UpdateMovementState()
    {
        // 0 = walk, 1 = mouselook, 2 = type
        uint8_t flags = 0;
        
        Core::Quaternion bodyrot = GetBodyRotation();
        bodyrot.normalize();
        
        Core::Quaternion headrot = GetHeadRotation();
        headrot.normalize();
    
        RexTypes::Vector3 camcenter = Vector3(0,0,0);
        RexTypes::Vector3 camataxis = Vector3(0,0,0);
        RexTypes::Vector3 camleftaxis = Vector3(0,0,0);
        RexTypes::Vector3 camupaxis = Vector3(0,0,0);       
        float fardist = 4000.0f;
        
        rexlogicmodule_->GetServerConnection()->SendAgentUpdatePacket(bodyrot,headrot,0,camcenter,camataxis,camleftaxis,camupaxis,fardist,controlflags_,flags);            
    }
    
    //! update camera position
    void AvatarController::Update(Core::f64 frametime)
    {
        OgreRenderer::Renderer *renderer = rexlogicmodule_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        if (!renderer || !avatarentity_)
            return;

        Ogre::Camera *camera = renderer->GetCurrentCamera();
        OgreRenderer::EC_OgrePlaceable &ogreplaceable = *checked_static_cast<OgreRenderer::EC_OgrePlaceable*>(avatarentity_->GetComponent("EC_OgrePlaceable").get());

        RexTypes::Vector3 campos = ogreplaceable.GetPosition();
        campos += (ogreplaceable.GetOrientation() * RexTypes::Vector3(0,0,-1) * cameradistance_);
        campos += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->setPosition(campos.x,campos.y,campos.z);
        
        RexTypes::Vector3 lookat = ogreplaceable.GetPosition();
        lookat += (ogreplaceable.GetOrientation() * cameraoffset_);
        camera->lookAt(lookat.x,lookat.y,lookat.z);
    }    
    
}

