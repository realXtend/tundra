// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_OgreCamera.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_OgreCamera::EC_OgreCamera(IModule* module) :
    IComponent(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    attached_(false),
    camera_(0)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_OgreCamera::~EC_OgreCamera()
{
    if (renderer_.expired())
        return;

    DetachCamera();

    if (camera_)
    {
        RendererPtr renderer = renderer_.lock();
        if (renderer->GetCurrentCamera() == camera_)
            renderer->SetCurrentCamera(0);
        renderer->GetSceneManager()->destroyCamera(camera_);
        camera_ = 0;
    }
}

void EC_OgreCamera::AutoSetPlaceable()
{
    Scene::Entity* entity = GetParentEntity();
    if (entity)
    {
        ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
        if (placeable)
            SetPlaceable(placeable);
    }
}

void EC_OgreCamera::SetPlaceable(ComponentPtr placeable)
{
    if (!dynamic_cast<EC_Placeable*>(placeable.get()))
    {
        OgreRenderingModule::LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic().toStdString());
        return;
    }

    DetachCamera();
    placeable_ = placeable;
    AttachCamera();
}

void EC_OgreCamera::SetNearClip(float nearclip)
{
    if (!camera_)
        return;

    camera_->setNearClipDistance(nearclip);
}

void EC_OgreCamera::SetFarClip(float farclip)
{
    if (!camera_)
        return;
    if (renderer_.expired())
        return;

    // Enforce that farclip doesn't go past renderer's view distance
    Renderer* renderer = renderer_.lock().get();
    if (farclip > renderer->GetViewDistance())
        farclip = renderer->GetViewDistance();
    camera_->setFarClipDistance(farclip);
}

void EC_OgreCamera::SetVerticalFov(float fov)
{
    if (!camera_)
        return;

    camera_->setFOVy(Ogre::Radian(fov));
}

void EC_OgreCamera::SetActive()
{
    if (!camera_)
        return;
    if (renderer_.expired())
        return;

    renderer_.lock()->SetCurrentCamera(camera_);
}

float EC_OgreCamera::GetNearClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getNearClipDistance();
}

float EC_OgreCamera::GetFarClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFarClipDistance();
}

float EC_OgreCamera::GetVerticalFov() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFOVy().valueRadians();
}

bool EC_OgreCamera::IsActive() const
{
    if (!camera_)
        return false;
    if (renderer_.expired())
        return false;

    return renderer_.lock()->GetCurrentCamera() == camera_;
}

void EC_OgreCamera::DetachCamera()
{
    if ((!attached_) || (!camera_) || (!placeable_))
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->detachObject(camera_);

    attached_ = false;
}

void EC_OgreCamera::AttachCamera()
{
    if ((attached_) || (!camera_) || (!placeable_))
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->attachObject(camera_);

    attached_ = true;
}

void EC_OgreCamera::UpdateSignals()
{
    Scene::Entity* parent = GetParentEntity();
    if (parent)
    {
        // Connect to ComponentRemoved signal of the parent entity, so we can check if the mesh gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)),
            SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
        
        // If scene is not view-enabled, no further action
        if (!ViewEnabled())
            return;
        
        // Create camera now if not yet created
        if (!camera_)
        {
            RendererPtr renderer = renderer_.lock();
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            Ogre::Viewport* viewport = renderer->GetViewport();
            
            camera_ = scene_mgr->createCamera(renderer->GetUniqueObjectName("EC_OgreCamera"));
            
            // Set default values for the camera
            camera_->setNearClipDistance(0.1f);
            camera_->setFarClipDistance(2000.f);
            camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth() / Ogre::Real(viewport->getActualHeight())));
            camera_->setAutoAspectRatio(true);
        }
    }
}

void EC_OgreCamera::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == placeable_.get())
        SetPlaceable(ComponentPtr());
}
