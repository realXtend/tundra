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
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_ParticleSystem")

#include <Ogre.h>
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_ParticleSystem::EC_ParticleSystem(IModule *module):
    IComponent(module->GetFramework()),
    particleRef(this, "Particle ref"),
    castShadows(this, "Cast shadows", false),
    renderingDistance(this, "Rendering distance", 0.0f),
    particleSystem_(0)
{
    renderer_ = GetFramework()->GetServiceManager()->GetService<Renderer>();
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(EntitySet()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), SLOT(AttributeUpdated(IAttribute*)));
}

EC_ParticleSystem::~EC_ParticleSystem()
{
    DeleteParticleSystem();
}

void EC_ParticleSystem::CreateParticleSystem(const QString &systemName)
{
    if (!ViewEnabled())
        return;
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

void EC_ParticleSystem::AttributeUpdated(IAttribute *attribute)
{
    if(attribute->GetNameString() == castShadows.GetNameString())
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
        if (!ViewEnabled())
            return;

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
