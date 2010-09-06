#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_ParticleSystem.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "OgreParticleResource.h"
#include "SceneManager.h"
#include "OgreRenderingModule.h"
#include "RexUUID.h"
#include "EventManager.h"

#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ParticleSystem")

#include <Ogre.h>
#include "MemoryLeakCheck.h"

EC_ParticleSystem::EC_ParticleSystem(Foundation::ModuleInterface *module):
    Foundation::ComponentInterface(module->GetFramework()),
    particleSystem_(0),
    particle_tag_(0),
    particleId_(this, "Particle id"),
    castShadows_(this, "Cast shadows", false),
    renderingDistance_(this, "Rendering distance", 0.0f)
{
    OgreRenderer::OgreRenderingModule *rendererModule = framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock().get();
    if(!rendererModule)
        return;
    renderer_ = OgreRenderer::RendererWeakPtr(rendererModule->GetRenderer());

    Foundation::EventManager *event_manager = framework_->GetEventManager().get();
    if(event_manager)
    {
        event_manager->RegisterEventSubscriber(this, 99);
        resource_event_category_ = event_manager->QueryEventCategory("Resource");
    }
    QObject::connect(this, SIGNAL(ParentEntitySet()), this, SLOT(UpdateSignals()));
}

EC_ParticleSystem::~EC_ParticleSystem()
{
    DeleteParticleSystem();
}

bool EC_ParticleSystem::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    // Making sure that event type is RESOURCE_READY before we start to dynamic cast.
    if (event_id != Resource::Events::RESOURCE_READY)
        return false;

    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    if(!event_data || particle_tag_ != event_data->tag_)
        return false;

    OgreRenderer::OgreParticleResource* partres = checked_static_cast<OgreRenderer::OgreParticleResource*>(event_data->resource_.get());
    if (!partres)
        return true;

    if (partres->GetNumTemplates())
        CreateParticleSystem(QString::fromStdString(partres->GetTemplateName(0)));
    return true;
}

void EC_ParticleSystem::CreateParticleSystem(const QString &systemName)
{
    if (renderer_.expired())
        return;
    OgreRenderer::RendererPtr renderer = renderer_.lock();

    try
    {
        DeleteParticleSystem();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        particleSystem_ = scene_mgr->createParticleSystem(renderer->GetUniqueObjectName(), systemName.toStdString());
        if(particleSystem_)
        {
            OgreRenderer::EC_OgrePlaceable *placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(FindPlaceable().get());
            if(!placeable)
                return;
            placeable->GetSceneNode()->attachObject(particleSystem_);
            particleSystem_->setCastShadows(castShadows_.Get());
            particleSystem_->setRenderingDistance(renderingDistance_.Get());
            return;
        }
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not add particle system " + Name().toStdString() + ": " + std::string(e.what()));
    }

    return;
}

void EC_ParticleSystem::DeleteParticleSystem()
{
    if (renderer_.expired() || !particleSystem_)
        return;
    OgreRenderer::RendererPtr renderer = renderer_.lock();

    OgreRenderer::EC_OgrePlaceable *placeable = dynamic_cast<OgreRenderer::EC_OgrePlaceable *>(FindPlaceable().get());
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    if(!placeable || !scene_mgr)
        return;

    try
    {
        //placeable->GetSceneNode()->detachObject(particleSystem_);
        Ogre::SceneNode *node = placeable->GetSceneNode();
        if(!node)
            return;
        node->detachObject(particleSystem_);
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not delete particle system " + Name().toStdString() + ": " + std::string(e.what()));
    }

    scene_mgr->destroyParticleSystem(particleSystem_);
    particleSystem_ = 0;
    return;
}

bool EC_ParticleSystem::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    if(category_id == resource_event_category_)
    {
        if(event_id == Resource::Events::RESOURCE_READY)
        {
            return HandleResourceEvent(event_id, data);
        }
    }
    return false;
}

void EC_ParticleSystem::AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute)
{
    if(component != this)
        return;

    if(attribute->GetNameString() == particleId_.GetNameString())
    {
        particle_tag_ = RequestResource(particleId_.Get().toStdString(), OgreRenderer::OgreParticleResource::GetTypeStatic());
        if(!particle_tag_) // To visualize that resource id was wrong delete previous particle effect off.
            DeleteParticleSystem();
    }
    else if(attribute->GetNameString() == castShadows_.GetNameString())
    {
        if(particleSystem_)
            particleSystem_->setCastShadows(castShadows_.Get());
    }
    else if(attribute->GetNameString() == renderingDistance_.GetNameString())
    {
        if(particleSystem_)
            particleSystem_->setRenderingDistance(renderingDistance_.Get());
    }
}

void EC_ParticleSystem::UpdateSignals()
{
    disconnect(this, SLOT(AttributeUpdated(Foundation::ComponentInterface *, Foundation::AttributeInterface *)));
    if(!GetParentEntity())
        return;

    Scene::SceneManager *scene = GetParentEntity()->GetScene();
    if(scene)
        connect(scene, SIGNAL(AttributeChanged(Foundation::ComponentInterface*, Foundation::AttributeInterface*, AttributeChange::Type)),
                this, SLOT(AttributeUpdated(Foundation::ComponentInterface*, Foundation::AttributeInterface*))); 
}

Foundation::ComponentPtr EC_ParticleSystem::FindPlaceable() const
{
    assert(framework_);
    Foundation::ComponentPtr comp;
    if(!GetParentEntity())
        return comp;
    comp = GetParentEntity()->GetComponent<OgreRenderer::EC_OgrePlaceable>();
    return comp;
}

request_tag_t EC_ParticleSystem::RequestResource(const std::string& id, const std::string& type)
{
    request_tag_t tag = 0;
    Foundation::RenderServiceInterface *renderInter = framework_->GetService<Foundation::RenderServiceInterface>();
    if(!renderInter)
        return tag;

    tag = renderInter->RequestResource(id, type);
    if(tag == 0)
    {
        LogWarning("Failed to request resource:" + id + " : " + type);
        return 0;
    }

    return tag;
}