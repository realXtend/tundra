// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_Light.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "OgreConversionUtils.h"

#include "IModule.h"
#include "Entity.h"
#include "Scene.h"
#include "XMLUtilities.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"

#include <QDomDocument>
#include <QList>
#include <QVector>

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Light::EC_Light(Scene* scene) :
    IComponent(scene),
    light_(0),
    attached_(false),
    type(this, "light type", LT_Point),
    direction(this, "direction", Vector3df(0.0f, 0.0f, 1.0f)),
    diffColor(this, "diffuse color", Color(1.0f, 1.0f, 1.0f)),
    specColor(this, "specular color", Color(0.0f, 0.0f, 0.0f)),
    castShadows(this, "cast shadows", false),
    range(this, "light range", 100.0f),
    constAtten(this, "constant atten", 0.0f),
    linearAtten(this, "linear atten", 0.01f),
    quadraAtten(this, "quadratic atten", 0.01f),
    innerAngle(this, "light inner angle", 30.0f),
    outerAngle(this, "light outer angle", 40.0f)
{
    if (scene)
        world_ = scene->GetWorld<OgreWorld>();
    
    static AttributeMetadata typeAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        typeAttrData.enums[LT_Point]       = "Point";
        typeAttrData.enums[LT_Spot]        = "Spot";
        typeAttrData.enums[LT_Directional] = "Directional";
        metadataInitialized = true;
    }
    type.SetMetadata(&typeAttrData);

    if (scene->ViewEnabled())
    {
        OgreWorldPtr world = world_.lock();
        Ogre::SceneManager* sceneMgr = world->GetSceneManager();
        light_ = sceneMgr->createLight(world->GetUniqueObjectName("EC_Light"));
    }
    
    QObject::connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(UpdateOgreLight()));
}

EC_Light::~EC_Light()
{
    if (world_.expired())
    {
        if (light_)
            LogError("EC_Light: World has expired, skipping uninitialization!");
        return;
    }
    
    if (light_)
    {
        DetachLight();
        Ogre::SceneManager* sceneMgr = world_.lock()->GetSceneManager();
        sceneMgr->destroyLight(light_);
        light_ = 0;
    }
}

void EC_Light::SetPlaceable(ComponentPtr placeable)
{
    if (!light_)
        return;
    
    if (dynamic_cast<EC_Placeable*>(placeable.get()) == 0)
    {
        LogError("Attempted to set a placeable which is not a placeable");
        return;
    }
    
    if (placeable_ == placeable)
        return;
    
    DetachLight();
    placeable_ = placeable;
    AttachLight();
}

void EC_Light::AttachLight()
{
    if ((light_) && (placeable_) && (!attached_))
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(light_);
        attached_ = true;
    }
}

void EC_Light::DetachLight()
{
    if ((light_) && (placeable_) && (attached_))
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(light_);
        attached_ = false;
    }
}

void EC_Light::UpdateOgreLight()
{
    if (!light_)
        return;
    
    // If placeable is not set yet, set it manually by searching it from the parent entity
    if (!placeable_)
    {
        Entity* entity = GetParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                SetPlaceable(placeable);
        }
    }
    
    Ogre::Light::LightTypes ogre_type = Ogre::Light::LT_POINT;

    switch (type.Get())
    {
        case LT_Spot:
        ogre_type = Ogre::Light::LT_SPOTLIGHT;
        break;
        
        case LT_Directional:
        ogre_type = Ogre::Light::LT_DIRECTIONAL;
        break;
    }
    
    try
    {
        light_->setType(ogre_type);
        light_->setDirection(ToOgreVector3(direction.Get()));
        light_->setDiffuseColour(ToOgreColor(diffColor.Get()));
        light_->setSpecularColour(ToOgreColor(specColor.Get()));
        light_->setAttenuation(range.Get(), constAtten.Get(), linearAtten.Get(), quadraAtten.Get());
        // Note: Ogre throws exception if we try to set this when light is not spotlight
        if (type.Get() == LT_Spot)
            light_->setSpotlightRange(Ogre::Degree(innerAngle.Get()), Ogre::Degree(outerAngle.Get()));
    }
    catch(Ogre::Exception& e)
    {
        LogError("Exception while setting EC_Light parameters to Ogre: " + std::string(e.what()));
    }
}

