// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_ParticleSystem.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "OgreParticleAsset.h"
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
    particleRef(this, "Particle ref" ),
    castShadows(this, "Cast shadows", false),
    renderingDistance(this, "Rendering distance", 0.0f),
    particleSystem_(0)
{
    renderer_ = GetFramework()->GetServiceManager()->GetService<Renderer>();
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(EntitySet()));
    connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(AttributeUpdated(IAttribute*)));
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
        EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
        if (!placeable)
        {
            LogError("Fail to create a new particle system, make sure that entity has EC_Placeable component created.");
            return;
        }

        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        particleSystem_ = scene_mgr->createParticleSystem(renderer->GetUniqueObjectName("EC_Particlesystem"), SanitateAssetIdForOgre(systemName.toStdString()));
        if (particleSystem_)
        {
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
    else if (attribute->GetNameString() == renderingDistance.GetNameString())
    {
        if (particleSystem_)
            particleSystem_->setRenderingDistance(renderingDistance.Get());
    }
    else if (attribute->GetNameString() == particleRef.GetNameString())
    {
        if (!ViewEnabled())
            return;

        // Request the new particle system resource. Once it has loaded, ParticleSystemAssetLoaded() will be called.
        AssetTransferPtr transfer = GetFramework()->Asset()->RequestAsset(particleRef.Get());
        if ( transfer.get() != 0)
        {
            connect(transfer.get(), SIGNAL(Loaded(IAssetTransfer*)), SLOT(ParticleSystemAssetLoaded()), Qt::UniqueConnection);
        }
        else
        {
            DeleteParticleSystem();
        }
        
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
    IAssetTransfer *transfer = dynamic_cast<IAssetTransfer*>(sender());
    assert(transfer);
    if (!transfer)
        return;

    OgreParticleAsset *asset = dynamic_cast<OgreParticleAsset*>(transfer->asset.get());
    if (!asset)
    {
        LogWarning("EC_ParticleSystem::ParticleSystemAssetLoaded: Asset transfer finished, but asset pointer was null!");
        return;
    }

    if (asset->GetNumTemplates() > 0)
        CreateParticleSystem(asset->GetTemplateName(0));
}

void EC_ParticleSystem::EntitySet()
{
    Scene::Entity *entity = this->GetParentEntity();
    if (!entity)
    {
        LogError("Failed to connect entity signals, component's parent entity is null");
        return;
    }
    
    QObject::disconnect(this, SLOT(DeleteParticleSystem()));
    QObject::connect(entity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(DeleteParticleSystem()));
}
