// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_ParticleSystem.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "OgreParticleResource.h"
#include "OgreConversionUtils.h"
#include "SceneManager.h"
#include "RexUUID.h"
#include "EventManager.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ParticleSystem")

#include <Ogre.h>
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_ParticleSystem::EC_ParticleSystem(IModule *module):
    IComponent(module->GetFramework()),
        particleRef(this, "Particle ref", AssetReference(QString(), OgreParticleResource::GetTypeStatic().c_str())),
//    particleId(this, "Particle id"),
    castShadows(this, "Cast shadows", false),
    renderingDistance(this, "Rendering distance", 0.0f),
    particleSystem_(0)//,
//    particle_tag_(0)
{
    renderer_ = GetFramework()->GetServiceManager()->GetService<Renderer>();
//    framework_->GetEventManager()->RegisterEventSubscriber(this, 99);
//    resource_event_category_ = framework_->GetEventManager()->QueryEventCategory("Resource");

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(EntitySet()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));
}

EC_ParticleSystem::~EC_ParticleSystem()
{
    DeleteParticleSystem();
}

/*
bool EC_ParticleSystem::HandleResourceEvent(event_id_t event_id, IEventData* data)
{
    // Making sure that event type is RESOURCE_READY before we start to dynamic cast.
    if (event_id != Resource::Events::RESOURCE_READY)
        return false;

    Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
    if(!event_data || particle_tag_ != event_data->tag_)
        return false;

    OgreParticleResource* partres = checked_static_cast<OgreParticleResource*>(event_data->resource_.get());
    if (!partres)
        return true;

    if (partres->GetNumTemplates())
        CreateParticleSystem(QString::fromStdString(partres->GetTemplateName(0)));
    return true;
}
*/

void EC_ParticleSystem::CreateParticleSystem(const QString &systemName)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();

    try
    {
        DeleteParticleSystem();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        particleSystem_ = scene_mgr->createParticleSystem(renderer->GetUniqueObjectName(), SanitateAssetIdForOgre(systemName.toStdString()));
        if(particleSystem_)
        {
            EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
            if(!placeable)
                return;
            placeable->GetSceneNode()->attachObject(particleSystem_);
            particleSystem_->setCastShadows(castShadows.Get());
            particleSystem_->setRenderingDistance(renderingDistance.Get());
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
    RendererPtr renderer = renderer_.lock();

    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    if (!scene_mgr)
        return;

    try
    {
        EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
        if (placeable)
        {
            Ogre::SceneNode *node = placeable->GetSceneNode();
            if (!node)
                return;
            node->detachObject(particleSystem_);
        }
        scene_mgr->destroyParticleSystem(particleSystem_);
        particleSystem_ = 0;
    }
    catch (Ogre::Exception& e)
    {
        LogError("Could not delete particle system " + Name().toStdString() + ": " + std::string(e.what()));
    }

    return;
}

/*bool EC_ParticleSystem::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    if(category_id == resource_event_category_)
    {
        if(event_id == Resource::Events::RESOURCE_READY)
        {
            return HandleResourceEvent(event_id, data);
        }
    }
    return false;
}*/

void EC_ParticleSystem::AttributeUpdated(IAttribute *attribute)
{
    /*
    if(attribute->GetNameString() == particleId.GetNameString())
    {
        particle_tag_ = RequestResource(particleId.Get().toStdString(), OgreParticleResource::GetTypeStatic());
        if(!particle_tag_) // To visualize that resource id was wrong delete previous particle effect off.
            DeleteParticleSystem();
    }
    else */if(attribute->GetNameString() == castShadows.GetNameString())
    {
        if (particleSystem_)
            particleSystem_->setCastShadows(castShadows.Get());
    }
    else if(attribute->GetNameString() == renderingDistance.GetNameString())
    {
        if (particleSystem_)
            particleSystem_->setRenderingDistance(renderingDistance.Get());
    }
    else if (attribute->GetNameString() == particleRef.GetNameString())
    {
        // Request the new particle system resource. Once it has loaded, ParticleSystemAssetLoaded() will be called.
        IAssetTransfer *transfer = GetFramework()->Asset()->RequestAsset(particleRef.Get());
        connect(transfer, SIGNAL(Loaded()), SLOT(ParticleSystemAssetLoaded()), Qt::UniqueConnection);
    }
}

ComponentPtr EC_ParticleSystem::FindPlaceable() const
{
    if (GetParentEntity())
        return GetParentEntity()->GetComponent<EC_Placeable>();
    else
        return ComponentPtr();
}

void EC_ParticleSystem::ParticleSystemAssetLoaded()
{
    IAssetTransfer *transfer = dynamic_cast<IAssetTransfer*>(QObject::sender());
    assert(transfer);
    if (!transfer)
        return;

    OgreParticleResource *resource = dynamic_cast<OgreParticleResource *>(transfer->resourcePtr.get());
    assert(resource);
    if (!resource)
        return;

    if (resource->GetNumTemplates())
        CreateParticleSystem(QString::fromStdString(resource->GetTemplateName(0)));
}

void EC_ParticleSystem::EntitySet()
{
    Scene::Entity *entity = this->GetParentEntity();
    if (!entity)
    {
        LogError("Failed to connect entity signals, component's parent entity is null");
        return;
    }
    disconnect(this, SLOT(DeleteParticleSystem()));
    connect(entity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(DeleteParticleSystem()));
}
/*
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
*/
