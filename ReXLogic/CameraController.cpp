// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CameraController.h"
#include "Renderer.h"

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

    //! update camera position
    void CameraController::Update(Core::f64 frametime)
    {
        float trans_dt = (float)frametime * sensitivity_;

        OgreRenderer::Renderer *renderer = module_->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
        Ogre::Camera *camera = renderer->GetCurrentCamera();
        Ogre::Vector3 pos = camera->getPosition();

        pos += camera->getOrientation() * translation_ * trans_dt;
        camera->setPosition(pos);

        float rot_dt = (float)frametime * rot_sensitivity_;

        camera->yaw(Ogre::Radian(yaw_ * rot_dt));
        camera->pitch(Ogre::Radian(pitch_ * rot_dt));
    }
}
