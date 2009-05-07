// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraController.h"
#include "Renderer.h"
#include "InputEvents.h"

namespace RexLogic
{
    CameraController::CameraController(Foundation::ModuleInterface *module) : 
module_(module)
, pitch_(0.f)
, yaw_(0.f)
, drag_yaw_(0.f)
, drag_pitch_(0.f)
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
        drag_yaw_ = static_cast<float>(movement->x_.rel_) * -0.2f;
        drag_pitch_ = static_cast<float>(movement->y_.rel_) * -0.2f;
    }

    //! update camera position
    void CameraController::Update(Core::f64 frametime)
    {
        float trans_dt = (float)frametime * sensitivity_;

        boost::shared_ptr<OgreRenderer::Renderer> renderer = module_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        if (renderer)
        {
            Ogre::Camera *camera = renderer->GetCurrentCamera();
            Ogre::Vector3 pos = camera->getPosition();

            pos += camera->getOrientation() * translation_ * trans_dt;
            camera->setPosition(pos);

            float rot_dt = (float)frametime * rot_sensitivity_;

            camera->yaw(Ogre::Radian(yaw_ * rot_dt)     + Ogre::Degree(drag_yaw_ * rot_sensitivity_));
            camera->pitch(Ogre::Radian(pitch_ * rot_dt) + Ogre::Degree(drag_pitch_ * rot_sensitivity_));
        }
    }
}
