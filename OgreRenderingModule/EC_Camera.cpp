// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_Camera.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Scene.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"
#include "LoggingFunctions.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Camera::EC_Camera(Scene* scene) :
    IComponent(scene),
    attached_(false),
    camera_(0),
    upVector(this, "Up vector", Vector3df::UNIT_Y)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
}

EC_Camera::~EC_Camera()
{
    if (world_.expired())
    {
        if (camera_)
            LogError("EC_Camera: World has expired, skipping uninitialization!");
        return;
    }
    
    OgreWorldPtr world = world_.lock();
    
    DetachCamera();

    if (camera_)
    {
        Renderer* renderer = world->GetRenderer();
        if (renderer->GetActiveCamera() == this)
            renderer->SetActiveCamera(0);
        world->GetSceneManager()->destroyCamera(camera_);
        camera_ = 0;
    }
}

void EC_Camera::AutoSetPlaceable()
{
    Entity* entity = GetParentEntity();
    if (entity)
    {
        ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
        if (placeable)
            SetPlaceable(placeable);
    }
}

Vector3df EC_Camera::GetInitialRotation() const
{
    Vector3df normUpVector = upVector.Get();
    normUpVector.normalize();
    
    // Identity rotation corresponds to -Z = forward & Y = up. Calculate which rotation we need to adjust for our up vector.
    Quaternion rot;
    rot.rotationFromTo(Vector3df::UNIT_Y, normUpVector);
    
    Vector3df euler;
    rot.toEuler(euler);
    euler *= RADTODEG;
    return euler;
}

Vector3df EC_Camera::GetAdjustedRotation(const Vector3df& rotVec) const
{
    // Optimization: if the up vector is Y-positive, no adjustment is necessary
    Vector3df normUpVector = upVector.Get();
    normUpVector.normalize();
    if (normUpVector.equals(Vector3df::UNIT_Y))
        return rotVec;
    
    Quaternion adjustQuat;
    adjustQuat.rotationFromTo(Vector3df::UNIT_Y, normUpVector);
    
    Quaternion rotQuat(rotVec.x * DEGTORAD, rotVec.y * DEGTORAD, rotVec.z * DEGTORAD);
    Quaternion final = rotQuat * adjustQuat;
    
    Vector3df euler;
    final.toEuler(euler);
    euler *= RADTODEG;
    return euler;
}

void EC_Camera::SetPlaceable(ComponentPtr placeable)
{
    if (!dynamic_cast<EC_Placeable*>(placeable.get()))
    {
        ::LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic().toStdString());
        return;
    }

    DetachCamera();
    placeable_ = placeable;
    AttachCamera();
}

void EC_Camera::SetNearClip(float nearclip)
{
    if (!camera_)
        return;

    camera_->setNearClipDistance(nearclip);
}

void EC_Camera::SetFarClip(float farclip)
{
    if (!camera_)
        return;
    if (world_.expired())
        return;

    // Enforce that farclip doesn't go past renderer's view distance
    OgreWorldPtr world = world_.lock();
    Renderer* renderer = world->GetRenderer();
    if (farclip > renderer->GetViewDistance())
        farclip = renderer->GetViewDistance();
    camera_->setFarClipDistance(farclip);
}

void EC_Camera::SetVerticalFov(float fov)
{
    if (!camera_)
        return;

    camera_->setFOVy(Ogre::Radian(fov));
}

void EC_Camera::SetActive()
{
    if (!camera_)
        return;
    if (world_.expired())
        return;

    world_.lock()->GetRenderer()->SetActiveCamera(this);
}

float EC_Camera::GetNearClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getNearClipDistance();
}

float EC_Camera::GetFarClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFarClipDistance();
}

float EC_Camera::GetVerticalFov() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFOVy().valueRadians();
}

bool EC_Camera::IsActive() const
{
    if (!camera_)
        return false;
    if (world_.expired())
        return false;

    return world_.lock()->GetRenderer()->GetActiveCamera() == this;
}

void EC_Camera::DetachCamera()
{
    if ((!attached_) || (!camera_) || (!placeable_))
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->detachObject(camera_);

    attached_ = false;
}

void EC_Camera::AttachCamera()
{
    if ((attached_) || (!camera_) || (!placeable_))
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->attachObject(camera_);

    attached_ = true;
}

Ogre::Ray EC_Camera::GetMouseRay(float x, float y)
{
    if (camera_)
        return camera_->getCameraToViewportRay(clamp(x, 0.f, 1.f), clamp(y, 0.f, 1.f));
    else
        return Ogre::Ray();
}

void EC_Camera::UpdateSignals()
{
    Entity* parent = GetParentEntity();
    if (parent)
    {
        // Connect to ComponentRemoved signal of the parent entity, so we can check if the placeable gets removed
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)),
            SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
        
        // If scene is not view-enabled, no further action
        if (!ViewEnabled())
            return;
        
        // Create camera now if not yet created
        if (!camera_)
        {
            OgreWorldPtr world = world_.lock();
            Ogre::SceneManager* sceneMgr = world->GetSceneManager();
            Ogre::Viewport* viewport = world->GetRenderer()->GetViewport();
            
            camera_ = sceneMgr->createCamera(world->GetUniqueObjectName("EC_Camera"));
            
            // Set default values for the camera
            camera_->setNearClipDistance(0.1f);
            camera_->setFarClipDistance(2000.f);
            camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth() / Ogre::Real(viewport->getActualHeight())));
            camera_->setAutoAspectRatio(true);
        }
    }
}

void EC_Camera::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == placeable_.get())
        SetPlaceable(ComponentPtr());
}

bool EC_Camera::IsEntityVisible(Entity* entity) const
{
    if ((!entity) || (!camera_) || (!parentEntity_))
        return false;
    if (entity->GetScene() != parentEntity_->GetScene())
        return false;
    EC_Placeable* placeable = entity->GetComponent<EC_Placeable>().get();
    if (!placeable)
        return false;
    Ogre::SceneNode* placeableNode = placeable->GetSceneNode();
    if (!placeableNode)
        return false;
    
    // Test all movable objects attached to the placeable node
    unsigned numObjects = placeableNode->numAttachedObjects();
    for (unsigned i = 0; i < numObjects; ++i)
    {
        if (camera_->isVisible(placeableNode->getAttachedObject(i)->getWorldBoundingBox()))
            return true;
    }
    // Treat the mesh as a special case, because it has its own adjustment node, and is not detected by the code above
    /// \todo Should be a general way
    EC_Mesh* mesh = entity->GetComponent<EC_Mesh>().get();
    if (mesh)
    {
        Ogre::Entity* entity = mesh->GetEntity();
        if ((entity) && (camera_->isVisible(entity->getWorldBoundingBox())))
            return true;
    }
    
    return false;
}
