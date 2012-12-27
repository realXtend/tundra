// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_ParticleSystem.h"
#include "EC_Placeable.h"
#include "OgreParticleAsset.h"
#include "OgreWorld.h"

#include "Entity.h"
#include "Scene/Scene.h"
#include "AssetAPI.h"
#include "IAssetTransfer.h"
#include "LoggingFunctions.h"
#include "AttributeMetadata.h"
#include "Framework.h"

#include <Ogre.h>
#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_ParticleSystem::EC_ParticleSystem(Scene* scene):
    IComponent(scene),
    particleRef(this, "Particle ref", AssetReference("", "OgreParticle")),
    castShadows(this, "Cast shadows", false),
    enabled(this, "Enabled", true),
    renderingDistance(this, "Rendering distance", 0.0f)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(EntitySet()));

    particleAsset_ = AssetRefListenerPtr(new AssetRefListener());
    connect(particleAsset_.get(), SIGNAL(Loaded(AssetPtr)), this, SLOT(OnParticleAssetLoaded(AssetPtr)), Qt::UniqueConnection);
    connect(particleAsset_.get(), SIGNAL(TransferFailed(IAssetTransfer*, QString)), this, SLOT(OnParticleAssetFailed(IAssetTransfer*, QString)), Qt::UniqueConnection);
}

EC_ParticleSystem::~EC_ParticleSystem()
{
    DeleteParticleSystem();
}

void EC_ParticleSystem::CreateParticleSystem(const QString &systemName)
{
    if (!ViewEnabled())
        return;
    
    OgreParticleAsset *particleAsset = dynamic_cast<OgreParticleAsset*>(particleAsset_->Asset().get());
    
    if (systemName.trimmed().isEmpty())
    {
        if (particleAsset)
        {
            for(int i = 0 ; i < particleAsset->GetNumTemplates(); ++i)
                CreateParticleSystem(particleAsset->GetTemplateName(i));
        }
        else
            LogError("Particle asset is not loaded, can not create all particle systems");
        return;
    }
    
    OgreWorldPtr world = world_.lock();
    if (!world)
        return;

    QString sanitatedSystemName = AssetAPI::SanitateAssetRef(systemName);

    if (particleAsset)
    {
        QString sanitatedAssetName = AssetAPI::SanitateAssetRef(particleAsset->Name());
        // Add the assetname to the system name if it is not included yet
        if (!sanitatedSystemName.startsWith(sanitatedAssetName) && systemName != "ParticleAssetLoadError")
            sanitatedSystemName = sanitatedAssetName + "_" + sanitatedSystemName;
    }
    
    // If system already exists, just make sure the emitter(s) are active
    if (particleSystems_.find(sanitatedSystemName) != particleSystems_.end())
    {
        particleSystems_[sanitatedSystemName]->setEmitting(true);
        return;
    }
    else
    {
        try
        {
            EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
            if (!placeable)
            {
                LogError("Fail to create a new particle system, make sure that entity has EC_Placeable component created.");
                return;
            }

            Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
            
            Ogre::ParticleSystem* system = sceneMgr->createParticleSystem(world->GetUniqueObjectName("EC_Particlesystem"), sanitatedSystemName.toStdString());
            if (system)
            {
                placeable->GetSceneNode()->attachObject(system);
                particleSystems_[sanitatedSystemName] = system;
                system->setCastShadows(castShadows.Get());
                system->setRenderingDistance(renderingDistance.Get());
                return;
            }
        }
        catch(Ogre::Exception& e)
        {
            LogError("Could not add particle system " + Name() + ": " + QString(e.what()));
        }
    }
    
    return;
}

void EC_ParticleSystem::DeleteParticleSystem(const QString& systemName)
{
    if (!ViewEnabled())
        return;
    
    if (systemName.trimmed().isEmpty())
    {
        // Make a copy of the names for iteration, as DeleteParticleSystem(name) modifies the map
        std::set<QString> names;
        for (std::map<QString, Ogre::ParticleSystem*>::const_iterator i = particleSystems_.begin(); i != particleSystems_.end(); ++i)
            names.insert(i->first);
        for (std::set<QString>::const_iterator i = names.begin(); i != names.end(); ++i)
            DeleteParticleSystem(*i);
        return;
    }
    
    OgreWorldPtr world = world_.lock();
    if (!world)
    {
        if (particleSystems_.size())
            LogError("EC_ParticleSystem: World has expired, skipping delete!");
        return;
    }
    
    Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
    if (!sceneMgr)
        return;

    QString sanitatedSystemName = AssetAPI::SanitateAssetRef(systemName);
    // Add the assetname to the system name if it is not included yet
    if (particleAsset_->Asset())
    {
        QString sanitatedAssetName = AssetAPI::SanitateAssetRef(particleAsset_->Asset()->Name());
        if (!sanitatedSystemName.startsWith(sanitatedAssetName) && systemName != "ParticleAssetLoadError")
            sanitatedSystemName = sanitatedAssetName + "_" + sanitatedSystemName;
    }
    
    std::map<QString, Ogre::ParticleSystem*>::iterator i = particleSystems_.find(sanitatedSystemName);
    if (i == particleSystems_.end())
    {
        LogWarning("Particle system " + AssetAPI::DesanitateAssetRef(sanitatedSystemName) + " not found, can not delete!");
        return;
    }

    try
    {
        EC_Placeable *placeable = dynamic_cast<EC_Placeable *>(FindPlaceable().get());
        if (placeable)
        {
            Ogre::SceneNode *node = placeable->GetSceneNode();
            if (node)
                node->detachObject(i->second);
        }
        sceneMgr->destroyParticleSystem(i->second);
    }
    catch(Ogre::Exception& /*e*/)
    {
        LogError("Could not delete particle system");
    }
    
    particleSystems_.erase(i);
}

void EC_ParticleSystem::SoftStopParticleSystem(const QString& systemName)
{
    if (!ViewEnabled())
        return;
    
    if (systemName.trimmed().isEmpty())
    {
        for (std::map<QString, Ogre::ParticleSystem*>::const_iterator i = particleSystems_.begin(); i != particleSystems_.end(); ++i)
            SoftStopParticleSystem(i->first);
        return;
    }
    
    QString sanitatedSystemName = AssetAPI::SanitateAssetRef(systemName);
    // Add the assetname to the system name if it is not included yet
    if (particleAsset_->Asset())
    {
        QString sanitatedAssetName = AssetAPI::SanitateAssetRef(particleAsset_->Asset()->Name());
        if (!sanitatedSystemName.startsWith(sanitatedAssetName))
            sanitatedSystemName = sanitatedAssetName + "_" + sanitatedSystemName;
    }
    
    std::map<QString, Ogre::ParticleSystem*>::iterator i = particleSystems_.find(sanitatedSystemName);
    if (i == particleSystems_.end())
    {
        LogWarning("Particle system " + AssetAPI::DesanitateAssetRef(sanitatedSystemName) + " not found, can not soft-stop!");
        return;
    }
    
    Ogre::ParticleSystem* system = i->second;
    system->setEmitting(false);
}

void EC_ParticleSystem::AttibutesChanged()
{
    if (!ViewEnabled())
        return;

    if (castShadows.ValueChanged())
        for(std::map<QString, Ogre::ParticleSystem*>::const_iterator i = particleSystems_.begin(); i != particleSystems_.end(); ++i)
            i->second->setCastShadows(castShadows.Get());

    if (renderingDistance.ValueChanged())
        for(std::map<QString, Ogre::ParticleSystem*>::const_iterator i = particleSystems_.begin(); i != particleSystems_.end(); ++i)
            i->second->setRenderingDistance(renderingDistance.Get());

    if (particleRef.ValueChanged())
    {
        if (!particleRef.Get().ref.trimmed().isEmpty())
            particleAsset_->HandleAssetRefChange(&particleRef);
        else // If the ref is cleared, delete any existing particle systems.
            DeleteParticleSystem();
    }

    if (enabled.ValueChanged())
    {
        if (enabled.Get() && particleAsset_->Asset())
            CreateParticleSystem(); // True: create/start all systems
        else if (!enabled.Get())
            DeleteParticleSystem(); // False: delete all systems
    }
}

ComponentPtr EC_ParticleSystem::FindPlaceable() const
{
    if (ParentEntity())
        return ParentEntity()->GetComponent<EC_Placeable>();
    else
        return ComponentPtr();
}

void EC_ParticleSystem::OnParticleAssetLoaded(AssetPtr asset)
{
    assert(asset);
    if (!asset)
        return;

    OgreParticleAsset *particleAsset = dynamic_cast<OgreParticleAsset*>(asset.get());
    if (!particleAsset)
    {
        LogError("OnMaterialAssetLoaded: Material asset load finished for asset \"" +
            asset->Name() + "\", but downloaded asset was not of type OgreParticleAsset!");
        return;
    }

    if (enabled.Get())
    {
        DeleteParticleSystem();
        CreateParticleSystem();
    }
}

void EC_ParticleSystem::OnParticleAssetFailed(IAssetTransfer* asset, QString reason)
{
    DeleteParticleSystem();
    CreateParticleSystem("ParticleAssetLoadError");
}

void EC_ParticleSystem::EntitySet()
{
    Entity *entity = this->ParentEntity();
    if (!entity)
    {
        LogError("Failed to connect entity signals, component's parent entity is null");
        return;
    }
    
    entity->ConnectAction("StartParticleSystem", this, SLOT(CreateParticleSystem(const QString &)));
    entity->ConnectAction("HardStopParticleSystem", this, SLOT(DeleteParticleSystem(const QString &)));
    entity->ConnectAction("SoftStopParticleSystem", this, SLOT(SoftStopParticleSystem(const QString &)));
    
    QObject::connect(entity, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), this, SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)), Qt::UniqueConnection);
}

void EC_ParticleSystem::OnComponentRemoved(IComponent *component, AttributeChange::Type change)
{
    // If the component is the Placeable, delete particle systems now for safety
    if (component->TypeName() == EC_Placeable::TypeNameStatic())
        DeleteParticleSystem();
}
