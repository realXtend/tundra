// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define OGRE_INTEROP

#include "EC_Camera.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "Renderer.h"

#include "Entity.h"
#include "FrameAPI.h"
#include "Scene.h"
#include "Profiler.h"
#include "LoggingFunctions.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Camera::EC_Camera(Scene* scene) :
    IComponent(scene),
    attached_(false),
    camera_(0),
    query_(0),
    queryFrameNumber_(-1),
    upVector(this, "Up vector", float3::unitY),
    nearPlane(this, "Near plane", 0.1f),
    farPlane(this, "Far plane", 2000.f),
    verticalFov(this, "Vertical FOV", 45.f),
    aspectRatio(this, "Aspect ratio", "")
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    
    if (framework)
        connect(framework->Frame(), SIGNAL(Updated(float)), this, SLOT(OnUpdated(float)));

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(OnAttributeUpdated(IAttribute*)));
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
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        sceneMgr->destroyCamera(camera_);
        camera_ = 0;
        
        if (query_)
        {
            sceneMgr->destroyQuery(query_);
            query_ = 0;
        }
    }
}

float3 EC_Camera::InitialRotation() const
{
    float3 normUpVector = upVector.Get();
    bool success = normUpVector.Normalize();
    if (!success)
        return float3::zero; // Pass out identity Euler angles.
    
    // Identity rotation corresponds to -Z = forward & Y = up. Calculate which rotation we need to adjust for our up vector.
    Quat rot = Quat::RotateFromTo(float3::unitY, normUpVector);
    
    float3 euler = rot.ToEulerZYX() * RADTODEG;
    std::swap(euler.x, euler.z); // Take into account our different convention for storing euler angles. (See Transform class).
    return euler;
}

float3 EC_Camera::AdjustedRotation(const float3& rotVec) const
{
    // Optimization: if the up vector is Y-positive, no adjustment is necessary
    float3 normUpVector = upVector.Get();
    bool success = normUpVector.Normalize();
    if (!success)
        return float3::zero; // Pass out identity Euler angles.
    if (normUpVector.Equals(float3::unitY))
        return rotVec;
    
    Quat adjustQuat = Quat::RotateFromTo(float3::unitY, normUpVector);
    
    Quat rotQuat = Quat::FromEulerZYX(rotVec.z * DEGTORAD, rotVec.y * DEGTORAD, rotVec.x * DEGTORAD);
    Quat final = rotQuat * adjustQuat;
    
    float3 euler = final.ToEulerZYX() * RADTODEG;
    std::swap(euler.x, euler.z);
    return euler;
}

void EC_Camera::SetPlaceable(ComponentPtr placeable)
{
    if (placeable && !dynamic_cast<EC_Placeable*>(placeable.get()))
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
    {
        LogError("EC_Camera::SetActive failed: No Ogre camera initialized to EC_Camera!");
        return;
    }
    if (world_.expired())
    {
        LogError("EC_Camera::SetActive failed: The camera component is in a scene that has already been destroyed!");
        return;
    }

    world_.lock()->GetRenderer()->SetActiveCamera(this);
}

float EC_Camera::NearClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getNearClipDistance();
}

float EC_Camera::FarClip() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFarClipDistance();
}

float EC_Camera::VerticalFov() const
{
    if (!camera_)
        return 0.0f;

    return camera_->getFOVy().valueRadians();
}

float EC_Camera::AspectRatio() const
{
    if (!aspectRatio.Get().trimmed().isEmpty())
    {
        if (!aspectRatio.Get().contains(":"))
        {
            float ar = aspectRatio.Get().toFloat();
            if (ar > 0.f)
                return ar;
        }
        else
        {
            QStringList str = aspectRatio.Get().split(":");

            if (str.length() == 2)
            {
                float width = str[0].toFloat();
                float height = str[1].toFloat();

                if (width > 0.f && height > 0.f)
                    return width / height;
            }
        }
        LogError("Invalid format for the aspectRatio field: \"" + aspectRatio.Get() + "\"! Should be of form \"float\" or \"float:float\". Leave aspectRatio empty to match the current main viewport aspect ratio.");
    }

    OgreWorldPtr world = world_.lock();
    Ogre::Viewport *viewport = world->GetRenderer()->GetViewport();
    if (viewport)
        return (float)viewport->getActualWidth() / viewport->getActualHeight();
    LogWarning("EC_Camera::AspectRatio(): No viewport or aspectRatio attribute set! Don't have an aspect ratio for the camera!");
    return 1.f;
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
    if (!attached_ || !camera_ || !placeable_)
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->detachObject(camera_);

    attached_ = false;
}

void EC_Camera::AttachCamera()
{
    if (attached_ || !camera_ || !placeable_)
        return;

    EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
    Ogre::SceneNode* node = placeable->GetSceneNode();
    node->attachObject(camera_);

    attached_ = true;
}

Ray EC_Camera::GetMouseRay(float x, float y)
{
    if (camera_)
        return camera_->getCameraToViewportRay(clamp(x, 0.f, 1.f), clamp(y, 0.f, 1.f));
    else
        return Ray();
}

void EC_Camera::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (!parent)
        return;

    connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentStructureChanged()), Qt::UniqueConnection);
    connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentStructureChanged()), Qt::UniqueConnection);
    
    // If scene is not view-enabled, no further action
    if (!ViewEnabled())
        return;
    
    // Create camera now if not yet created
    if (!camera_)
    {
        OgreWorldPtr world = world_.lock();
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        
        camera_ = sceneMgr->createCamera(world->GetUniqueObjectName("EC_Camera"));
        
        // Set default values for the camera
		camera_->setNearClipDistance(nearPlane.Get());
		camera_->setFarClipDistance(farPlane.Get());
        camera_->setAspectRatio(AspectRatio());
        camera_->setAutoAspectRatio(aspectRatio.Get().trimmed().isEmpty()); ///\note If user inputs garbage into the aspectRatio field, this will incorrectly go true. (but above line prints an error to user, so should be ok). 
        camera_->setFOVy(Ogre::Radian(Ogre::Math::DegreesToRadians(verticalFov.Get())));

        // Create a reusable frustum query
        Ogre::PlaneBoundedVolumeList dummy;
        query_ = sceneMgr->createPlaneBoundedVolumeQuery(dummy);
    }

    // Make sure we attach to the EC_Placeable if exists.
    OnComponentStructureChanged();
}

void EC_Camera::OnComponentStructureChanged()
{
    ComponentPtr placeable;
    Entity *entity = ParentEntity();
    if (entity)
        placeable = entity->GetComponent<EC_Placeable>();
    SetPlaceable(placeable);
}

void EC_Camera::OnAttributeUpdated(IAttribute *attribute)
{
    if (attribute == &nearPlane)
        SetNearClip(nearPlane.Get());
    else if (attribute == &farPlane)
        SetFarClip(farPlane.Get());
    else if (attribute == &verticalFov)
        SetVerticalFov(Ogre::Math::DegreesToRadians(verticalFov.Get()));
    else if (attribute == &aspectRatio && camera_)
    {
        camera_->setAspectRatio(AspectRatio());
        camera_->setAutoAspectRatio(aspectRatio.Get().trimmed().isEmpty()); ///\note If user inputs garbage into the aspectRatio field, this will incorrectly go true. (but above line prints an error to user, so should be ok). 
    }
}

bool EC_Camera::IsEntityVisible(Entity* entity)
{
    if (!entity || !camera_)
        return false;
    
    // Update query if not updated this frame
    if (queryFrameNumber_ != framework->Frame()->FrameNumber())
        QueryVisibleEntities();
    
    return visibleEntities_.find(entity->Id()) != visibleEntities_.end();
}

QList<Entity*> EC_Camera::VisibleEntities()
{
    QList<Entity*> l;
    
    if (!camera_ || !parentEntity || !parentEntity->ParentScene())
        return l;
    
    Scene* scene = parentEntity->ParentScene();
    
    // Update query if not updated this frame
    if (queryFrameNumber_ != framework->Frame()->FrameNumber())
        QueryVisibleEntities();
    
    for (std::set<entity_id_t>::iterator i = visibleEntities_.begin(); i != visibleEntities_.end(); ++i)
    {
        Entity* entity = scene->GetEntity(*i).get();
        if (entity)
            l.push_back(entity);
    }
    
    return l;
}

const std::set<entity_id_t>& EC_Camera::VisibleEntityIDs()
{
    if (camera_)
    {
        // Update query if not updated this frame
        if (queryFrameNumber_ != framework->Frame()->FrameNumber())
            QueryVisibleEntities();
    }
    
    return visibleEntities_;
}

void EC_Camera::StartViewTracking(Entity* entity)
{
    if (!entity)
        return;

    EntityPtr entityPtr = entity->shared_from_this();
    for (unsigned i = 0; i < visibilityTrackedEntities_.size(); ++i)
    {
        if (visibilityTrackedEntities_[i].lock() == entityPtr)
            return; // Already added
    }
    
    visibilityTrackedEntities_.push_back(entity->shared_from_this());
}

void EC_Camera::StopViewTracking(Entity* entity)
{
    if (!entity)
        return;
    
    EntityPtr entityPtr = entity->shared_from_this();
    for (unsigned i = 0; i < visibilityTrackedEntities_.size(); ++i)
    {
        if (visibilityTrackedEntities_[i].lock() == entityPtr)
        {
            visibilityTrackedEntities_.erase(visibilityTrackedEntities_.begin() + i);
            return;
        }
    }
}

void EC_Camera::OnUpdated(float timeStep)
{
    // Do nothing if visibility not being tracked for any entities
    if (visibilityTrackedEntities_.empty())
        return;
    
    // Update visible objects now if necessary
    if (queryFrameNumber_ != framework->Frame()->FrameNumber())
        QueryVisibleEntities();
    
    for (unsigned i = visibilityTrackedEntities_.size() - 1; i < visibilityTrackedEntities_.size(); --i)
    {
        // Check if the entity has expired; erase from list in that case
        if (visibilityTrackedEntities_[i].expired())
            visibilityTrackedEntities_.erase(visibilityTrackedEntities_.begin() + i);
        else
        {
            Entity* entity = visibilityTrackedEntities_[i].lock().get();
            entity_id_t id = entity->Id();
            
            // Check for change in visibility status
            bool last = lastVisibleEntities_.find(id) != lastVisibleEntities_.end();
            bool now = visibleEntities_.find(id) != visibleEntities_.end();
            
            if ((!last) && (now))
            {
                emit EntityEnterView(entity);
                entity->EmitEnterView(this);
            }
            else if ((last) && (!now))
            {
                emit EntityLeaveView(entity);
                entity->EmitLeaveView(this);
            }
        }
    }
}

void EC_Camera::QueryVisibleEntities()
{
    if (!camera_ || !query_)
        return;
    
    PROFILE(OgreWorld_FrustumQuery);

#if OGRE_VERSION_MAJOR >= 1 && OGRE_VERSION_MINOR >= 7
    lastVisibleEntities_ = visibleEntities_;
    visibleEntities_.clear();
    
    Ogre::PlaneBoundedVolumeList volumes;
    Ogre::PlaneBoundedVolume p = camera_->getPlaneBoundedVolume();
    volumes.push_back(p);
    query_->setVolumes(volumes);

    Ogre::SceneQueryResult results = query_->execute();
    for(Ogre::SceneQueryResultMovableList::iterator iter = results.movables.begin(); iter != results.movables.end(); ++iter)
    {
        Ogre::MovableObject *m = *iter;
        const Ogre::Any& any = m->getUserAny();
        if (any.isEmpty())
            continue;
        
        Entity *entity = 0;
        try
        {
            entity = Ogre::any_cast<Entity*>(any);
        }
        catch(Ogre::InvalidParametersException &/*e*/)
        {
            continue;
        }
        if (entity)
            visibleEntities_.insert(entity->Id());
    }
    
    queryFrameNumber_ = framework->Frame()->FrameNumber();
#else
    visibleEntities_.clear();
    LogWarning("EC_Camera::QueryVisibleEntities: Not supported on your Ogre version!"); ///\todo Check which exact version has the above getPlaneBoundedVolume(), 1.6.4 doesn't seem to, 1.7.1 does.
#endif

}

