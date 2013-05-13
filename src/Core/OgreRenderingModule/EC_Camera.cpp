// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#define MATH_OGRE_INTEROP

#include "EC_Camera.h"
#include "EC_Mesh.h"
#include "EC_Placeable.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"
#include "TextureAsset.h"
#include "Renderer.h"

#include "Entity.h"
#include "FrameAPI.h"
#include "Scene/Scene.h"
#include "Profiler.h"
#include "LoggingFunctions.h"
#include "Application.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "AttributeMetadata.h"

#include <Ogre.h>

#include <QDir>
#include <QDateTime>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Camera::EC_Camera(Scene* scene) :
    IComponent(scene),
    upVector(this, "Up vector", float3::unitY),
    nearPlane(this, "Near plane", 0.1f),
    farPlane(this, "Far plane", 2000.f),
    verticalFov(this, "Vertical FOV", 45.f),
    aspectRatio(this, "Aspect ratio", ""),
    attached_(false),
    camera_(0),
    query_(0),
    queryFrameNumber_(-1)
{
    static AttributeMetadata minZero;
    static bool metadataInitialized = false;
    if (!metadataInitialized)
    {
        minZero.minimum = "0.0";
        metadataInitialized = true;
    }
    nearPlane.SetMetadata(&minZero);

    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
    if (framework)
        connect(framework->Frame(), SIGNAL(Updated(float)), SLOT(OnUpdated(float)));
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
    DestroyOgreCamera();

    if (query_)
    {
        Ogre::SceneManager *sceneMgr = world ? world->OgreSceneManager() : 0;
        if (sceneMgr)
            sceneMgr->destroyQuery(query_);
        else
            LogError("EC_Camera: failed to delete scene query object! SceneManager was null!");
        query_ = 0;
    }

    // Release rendering texture if one has been created
    if (!renderTextureName_.empty())
    {
        try
        {
            Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(renderTextureName_);
            if (!texture.isNull())
                Ogre::TextureManager::getSingleton().remove(renderTextureName_);
        }
        catch(const Ogre::Exception &) {}
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
    
    float3 euler = RadToDeg(rot.ToEulerZYX());
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
    
    Quat rotQuat = Quat::FromEulerZYX(DegToRad(rotVec.z), DegToRad(rotVec.y), DegToRad(rotVec.x));
    Quat final = rotQuat * adjustQuat;
    
    float3 euler = RadToDeg(final.ToEulerZYX());
    std::swap(euler.x, euler.z);
    return euler;
}

void EC_Camera::SetPlaceable(const ComponentPtr &placeable)
{
    if (placeable && !dynamic_cast<EC_Placeable*>(placeable.get()))
    {
        ::LogError("Attempted to set placeable which is not " + EC_Placeable::TypeNameStatic());
        return;
    }

    DetachCamera();
    placeable_ = placeable;
    AttachCamera();
}

void EC_Camera::SetNearClip(float nearclip)
{
    LogWarning("EC_Camera::SetNearClip: this functions is deprecated and will be removed. Use attribute nearPlane direcly.");
    SetNearClipDistance(nearclip);
}

void EC_Camera::SetFarClip(float farclip)
{
    LogWarning("EC_Camera::SetNearClip: this functions is deprecated and will be removed. Use attribute farPlane direcly.");
    SetFarClipDistance(farclip);
}

void EC_Camera::SetVerticalFov(float fov)
{
    LogWarning("EC_Camera::SetVerticalFov: this functions is deprecated and will be removed. Use attribute verticalFov direcly.");
    SetFovY(fov);
}

void EC_Camera::SetActive()
{
    if (!ViewEnabled())
        return;

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
    if (!ParentEntity())
    {
        LogError("EC_Camera::SetActive failed: The camera component is not attached to an Entity!");
        return;
    }

    world_.lock()->Renderer()->SetMainCamera(ParentEntity());

    // Forcibly update the aspect ratio for the new camera. Ogre has a bug that activating a new camera will not automatically re-apply the aspect ratio automatically,
    // if its setAutoAspectRatio was set to true. Therefore, re-apply the aspect ratio when activating a new camera to the main viewport.
    SetAspectRatio(AspectRatio());
}

float EC_Camera::NearClip() const
{
    LogWarning("EC_Camera::NearClip: this functions is deprecated and will be removed. Use attribute nearPlane direcly.");
    return (camera_ ? camera_->getNearClipDistance() : 0.0f);
}

float EC_Camera::FarClip() const
{
    LogWarning("EC_Camera::FarClip: this functions is deprecated and will be removed. Use attribute farPlane direcly.");
    return (camera_ ? camera_->getFarClipDistance() : 0.0f);
}

float EC_Camera::VerticalFov() const
{
    LogWarning("EC_Camera::VerticalFov: this functions is deprecated and will be removed. Use attribute verticalFov direcly.");
    return (camera_ ? camera_->getFOVy().valueRadians() : 0.0f);
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
    Ogre::Viewport *viewport = world->Renderer()->MainViewport();
    if (viewport)
        return (float)viewport->getActualWidth() / viewport->getActualHeight();
    LogWarning("EC_Camera::AspectRatio: No viewport or aspectRatio attribute set! Don't have an aspect ratio for the camera!");
    return 1.f;
}

void EC_Camera::SetAspectRatio(float ratio)
{
    if (camera_)
    {
        camera_->setAspectRatio(ratio);
        camera_->setAutoAspectRatio(aspectRatio.Get().trimmed().isEmpty()); /**< @note If user inputs garbage into the aspectRatio field, this will incorrectly go true. (but above line prints an error to user, so should be ok). */
    }
}

bool EC_Camera::IsActive() const
{
    if (!camera_ || world_.expired() || !ParentEntity())
        return false;

    return world_.lock()->Renderer()->MainCamera() == ParentEntity();
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

void EC_Camera::DestroyOgreCamera()
{
    if (!camera_)
        return;

    OgreWorldPtr world = world_.lock();

    Renderer *renderer = world ? world->Renderer() : 0;
    // If this camera was the current camera, set the current camera to 0.
    if (renderer && renderer->MainOgreCamera() == camera_)
        renderer->SetMainCamera(0);

    Ogre::SceneManager *sceneMgr = world ? world->OgreSceneManager() : 0;
    if (!sceneMgr)
    {
        LogError("EC_Camera: Cannot delete Ogre camera! Ogre SceneManager was null already!");
        return;
    }

    sceneMgr->destroyCamera(camera_);
    camera_ = 0;
}

Ray EC_Camera::ViewportPointToRay(float x, float y) const
{
    // Do a bit of sanity checking that the user didn't go and input absolute window coordinates.
    if (fabs(x) >= 10.f || fabs(y) >= 10.f || !isfinite(x) || !isfinite(y))
        LogError(QString("EC_Camera::ViewportPointToRay takes input (x,y) coordinates normalized in the range [0,1]! (You inputted x=%1, y=%2").arg(x).arg(y));
    return (camera_ ? Ray(camera_->getCameraToViewportRay(Clamp(x, 0.f, 1.f), Clamp(y, 0.f, 1.f))) : Ray());
}

Ray EC_Camera::ScreenPointToRay(uint x, uint y) const
{
    QGraphicsScene *gscene = GetFramework()->Ui()->GraphicsScene(); // graphics scene can be null if running headless mode
    return (gscene ? ViewportPointToRay((float)x / gscene->width(), (float)y / gscene->height()) : Ray());
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
        Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
        
        camera_ = sceneMgr->createCamera(world->GetUniqueObjectName("EC_Camera"));
        
        // Set default values for the camera
        SetNearClipDistance(nearPlane.Get());
        SetFarClipDistance(farPlane.Get());
        SetFovY(verticalFov.Get());
        SetAspectRatio(AspectRatio());

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

void EC_Camera::AttributesChanged()
{
    if (nearPlane.ValueChanged())
        SetNearClipDistance(nearPlane.Get());
    if (farPlane.ValueChanged())
        SetFarClipDistance(farPlane.Get());
    if (verticalFov.ValueChanged())
        SetFovY(verticalFov.Get());
    else if (aspectRatio.ValueChanged())
        SetAspectRatio(AspectRatio());
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

float4x4 EC_Camera::ViewMatrix() const
{
    return (camera_ ? camera_->getViewMatrix() : float4x4::nan);
}

float4x4 EC_Camera::ProjectionMatrix() const
{
    return (camera_ ? camera_->getProjectionMatrix() : float4x4::nan);
}

void EC_Camera::SetFromFrustum(const Frustum &f)
{
    /// @todo check that f.type == PerspectiveFrustum when FrustumType is exposed to scripts.
    EC_Placeable *p = dynamic_cast<EC_Placeable*>(placeable_.get());
    if (p)
        p->SetWorldTransform(float3x4::LookAt(f.pos, f.pos + f.front, -float3::unitZ, float3::unitY, f.up));
    else
        LogWarning("EC_Camera::SetFromFrustum: Camera entity has no Placeable, cannot set world transform.");

    upVector.Set(f.up, AttributeChange::Disconnected);
    nearPlane.Set(f.nearPlaneDistance, AttributeChange::Disconnected);
    farPlane.Set(f.farPlaneDistance, AttributeChange::Disconnected);
    // f.horizontalFov is used for calculating the aspect ratio
    verticalFov.Set(RadToDeg(f.verticalFov), AttributeChange::Disconnected);
    aspectRatio.Set(QString::number(f.AspectRatio()), AttributeChange::Default);
}

Frustum EC_Camera::ToFrustum() const
{
    Frustum f;
    f.type = InvalidFrustum;
    if (!camera_)
    {
        LogError("EC_Camera::ToFrustum failed: No Ogre camera initialized to EC_Camera!");
        return f;
    }
    EC_Placeable *p = dynamic_cast<EC_Placeable*>(placeable_.get());
    if (!p)
    {
        LogError("EC_Camera::ToFrustum failed: No Placeable set to the camera entity!");
        return f;
    }

    f.type = PerspectiveFrustum;
    f.pos = p->WorldPosition();
    f.front = p->WorldOrientation() * -float3::unitZ;
    f.up = upVector.Get().Normalized();
    f.nearPlaneDistance = nearPlane.Get();
    f.farPlaneDistance = farPlane.Get();
    f.horizontalFov = DegToRad(verticalFov.Get());
    f.verticalFov = AspectRatio() * f.horizontalFov;
    return f;
}

void EC_Camera::OnUpdated(float timeStep)
{
    if (visibilityTrackedEntities_.empty())
        return; // Do nothing if visibility not being tracked for any entities
    
    PROFILE(EC_Camera_OnUpdated);
    // Update visible objects now if necessary
    if (queryFrameNumber_ != framework->Frame()->FrameNumber())
        QueryVisibleEntities();
    
    for (size_t i = visibilityTrackedEntities_.size() - 1; i < visibilityTrackedEntities_.size(); --i)
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
            if (!last && now)
            {
                emit EntityEnterView(entity);
                entity->EmitEnterView(this);
            }
            else if (last && !now)
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
    
    PROFILE(EC_Camera_QueryVisibleEntities);

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
            IComponent *component = Ogre::any_cast<IComponent*>(any);
            entity = component ? component->ParentEntity() : 0;
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
    LogWarning("EC_Camera::QueryVisibleEntities: Not supported on your Ogre version!");
#endif
}

void EC_Camera::SetNearClipDistance(float distance)
{
    if (camera_ && !world_.expired())
        camera_->setNearClipDistance(Clamp(distance, 0.0f, floatMax)); // Ogre throws an exception if negative value given.
}

QString EC_Camera::SaveScreenshot(bool renderUi)
{
    QImage img = ToQImage(renderUi);
    if (img.isNull())
        return "";

    // Check that app dir has screenshots folder
    QDir appdataDir(QDir::fromNativeSeparators(Application::UserDataDirectory()));
    if (!appdataDir.cd("screenshots"))
    {
        appdataDir.mkdir("screenshots");
        appdataDir.cd("screenshots");
    }

    // Note you cant use ":" to separate the hours, mins and secs as its not valid as a filename char!
    QString filename = "Tundra-" + QDateTime::currentDateTime().toString("yyyy-MM-dd_hh.mm.ss");
    QString ext = ".png";

    // Check if we are creating multiple screenshots during the same second.
    if (appdataDir.exists(filename + ext))
    {
        int runningID = 1;
        QString filenameWithId = filename + "_" + QString::number(runningID);
        while (appdataDir.exists(filenameWithId + ext))
        {
            runningID += 1;
            filenameWithId = filename + "_" + QString::number(runningID);

            // Sanity safe guard. I'd hope no one is trying to take >10000 screenshots in one second.
            if (runningID >= 10000)
                return "";
        }

        filename = filenameWithId;
    }

    QString absFilePath = appdataDir.absoluteFilePath(filename + ext);
    if (img.save(absFilePath, "PNG"))
        return absFilePath;
    else
        return "";
}

QImage EC_Camera::ToQImage(bool renderUi)
{
    Ogre::TexturePtr texture = UpdateRenderTexture(renderUi);
    if (texture.isNull())
        return QImage();
    return TextureAsset::ToQImage(texture.get());
}

Ogre::Image EC_Camera::ToOgreImage(bool renderUi)
{
    Ogre::Image image;
    Ogre::TexturePtr texture = UpdateRenderTexture(renderUi);
    if (texture.isNull())
        return image;
    texture->convertToImage(image);
    return image;
}

Ogre::TexturePtr EC_Camera::UpdateRenderTexture(bool renderUi)
{
    if (!ViewEnabled() || !framework->Ui()->MainWindow())
        Ogre::TexturePtr();
    OgreWorldPtr world = world_.lock();
    if (!camera_ || !world || !world->Renderer())
        Ogre::TexturePtr();

    QSize textureSize(world->Renderer()->WindowWidth(), world->Renderer()->WindowHeight());
    try
    {
        // First run init the texture name.
        if (renderTextureName_.empty())
            renderTextureName_ =  world->GenerateUniqueObjectName("EC_Camera_RenderTexture_");

        Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().getByName(renderTextureName_);
        if (texture.isNull())
        {
            texture = Ogre::TextureManager::getSingleton().createManual(renderTextureName_,
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, Ogre::TEX_TYPE_2D, textureSize.width(), textureSize.height(), 0,
                Ogre::PF_A8R8G8B8, Ogre::TU_RENDERTARGET);
        }
        else if ((int)texture->getWidth() != textureSize.width() || (int)texture->getHeight() != textureSize.height())
        {
            texture->freeInternalResources();
            texture->setWidth(textureSize.width());
            texture->setHeight(textureSize.height());
            texture->createInternalResources();
        }

        Ogre::RenderTexture *renderTarget = texture->getBuffer()->getRenderTarget();
        if (renderTarget)
        {
            renderTarget->removeAllViewports();
            Ogre::Viewport *viewPort = renderTarget->addViewport(camera_);
            if (viewPort)
            {
                // Show/hide overlays. UI layer is an overlay in Tundra.
                viewPort->setOverlaysEnabled(renderUi);
                // Update texture
                renderTarget->update(false);
                renderTarget->setAutoUpdated(false);
                return texture;
            }
        }
    }
    catch(Ogre::Exception &e)
    {
        LogError("EC_Camera::UpdateRenderTexture() caught and expection: " + std::string(e.what()));
    }

    return Ogre::TexturePtr();
}

void EC_Camera::SetFarClipDistance(float distance)
{
    if (!camera_ || world_.expired())
        return;

    ///@todo Uncomment when SkyX 0.2.1 is used, http://www.ogre3d.org/forums/viewtopic.php?f=11&t=67137&sid=df2bdcd4352cea57057b7861f563f692&start=100#p445143
    // Commented out as it can cause SkyX missing bug, depending on the view distance.
    /*
    // Enforce that farclip doesn't go past renderer's view distance
    OgreWorldPtr world = world_.lock();
    Renderer* renderer = world->Renderer();
    if (farclip > renderer->ViewDistance())
        farclip = renderer->ViewDistance();
    */
    camera_->setFarClipDistance(distance);
}

void EC_Camera::SetFovY(float fov)
{
    if (camera_ && !world_.expired())
        camera_->setFOVy(Ogre::Radian(Ogre::Math::DegreesToRadians(fov)));
}
