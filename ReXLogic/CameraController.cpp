// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraController.h"
#include "Renderer.h"
#include "InputEvents.h"
#include "RexLogicModule.h"
#include "InputServiceInterface.h"

namespace RexLogic
{
    CameraController::CameraController(Foundation::ModuleInterface *module) : 
module_(module)
, pitch_(0.f)
, yaw_(0.f)
, translation_(Ogre::Vector3::ZERO)
    {
        assert (module_);

        sensitivity_ = module_->GetFramework()->GetDefaultConfig().DeclareSetting("Camera", "translation_sensitivity", 25.f);
        rot_sensitivity_ = module_->GetFramework()->GetDefaultConfig().DeclareSetting("Camera", "rotation_sensitivity", 1.6f);
    }

    CameraController::~CameraController()
    {
    }

    void CameraController::StartMovingForward()
    {
        translation_.z = -1;
        translation_.normalise();
    }

    void CameraController::StopMovingForward()
    {
        translation_.z = 0;
        translation_.normalise();
    }

    void CameraController::StartMovingBackward()
    {
        translation_.z = 1;
        translation_.normalise();
    }
    void CameraController::StopMovingBackward()
    {
        translation_.z = 0;
        translation_.normalise();
    }
    void CameraController::StartMovingLeft()
    {
        translation_.x = -1;
        translation_.normalise();
    }
    void CameraController::StopMovingLeft()
    {
        translation_.x = 0;
        translation_.normalise();
    }
    void CameraController::StartMovingRight()
    {
        translation_.x = 1;
        translation_.normalise();
    }
    void CameraController::StopMovingRight()
    {
        translation_.x = 0;
        translation_.normalise();
    }
    void CameraController::StartMovingUp()
    {
        translation_.y = 1;
        translation_.normalise();
    }
    void CameraController::StopMovingUp()
    {
        translation_.y = 0;
        translation_.normalise();
    }
    void CameraController::StartMovingDown()
    {
        translation_.y = -1;
        translation_.normalise();
    }
    void CameraController::StopMovingDown()
    {
        translation_.y = 0;
        translation_.normalise();
    }
    void CameraController::StartRotatingLeft()
    {
        yaw_ = 1;
    }
    void CameraController::StopRotatingLeft()
    {
        yaw_ = 0;
    }
    void CameraController::StartRotatingRight()
    {
        yaw_ = -1;
    }
    void CameraController::StopRotatingRight()
    {
        yaw_ = 0;
    }
    void CameraController::StartRotatingUp()
    {
        pitch_ = 1;
    }
    void CameraController::StopRotatingUp()
    {
        pitch_ = 0;
    }
    void CameraController::StartRotatingDown()
    {
        pitch_ = -1;
    }
    void CameraController::StopRotatingDown()
    {
        pitch_ = 0;
    }

    void CameraController::Drag(const Input::Events::Movement *movement)
    {
        //yaw_ = static_cast<float>(movement->x_.rel_) * -0.5f;
        //pitch_ = static_cast<float>(movement->y_.rel_) * -0.5f;

        //if (movement->x_.abs_ != 0)
        //{
        //    RexLogicModule::LogInfo("x mov: " + Core::ToString(movement->x_.abs_));
        //    RexLogicModule::LogInfo("yaw: " + Core::ToString(yaw_));
        //}
    }

    //! update camera position
    void CameraController::Update(Core::f64 frametime)
    {
        float trans_dt = (float)frametime * sensitivity_;

        OgreRenderer::Renderer *renderer = module_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        if (renderer)
        {
            Ogre::Camera *camera = renderer->GetCurrentCamera();
            Ogre::Vector3 pos = camera->getPosition();

            pos += camera->getOrientation() * translation_ * trans_dt;
            camera->setPosition(pos);

            float rot_dt = (float)frametime * rot_sensitivity_;

            camera->yaw(Ogre::Radian(yaw_ * rot_dt));
            camera->pitch(Ogre::Radian(pitch_ * rot_dt));

            Foundation::InputServiceInterface *input = module_->GetFramework()->GetService<Foundation::InputServiceInterface>(Foundation::Service::ST_Input);
            if (input)
            {
                boost::optional<const Input::Events::Movement&> movement = input->GetDragMovement(Input::Events::MOUSELOOK);
                if (movement)
                {
                    camera->yaw(Ogre::Degree(movement->x_.rel_ * -0.2f * rot_sensitivity_));
                    camera->pitch(Ogre::Degree(movement->y_.rel_ * -0.2f * rot_sensitivity_));
                }
            }
        }
    }
}
