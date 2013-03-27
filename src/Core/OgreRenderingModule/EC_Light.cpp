// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#define MATH_OGRE_INTEROP
#include "DebugOperatorNew.h"

#include "EC_Light.h"
#include "Renderer.h"
#include "EC_Placeable.h"

#include "Entity.h"
#include "Scene/Scene.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "OgreRenderingModule.h"
#include "OgreWorld.h"

#include <OgreSceneManager.h>
#include <OgreSceneNode.h>

#include "MemoryLeakCheck.h"

using namespace OgreRenderer;

EC_Light::EC_Light(Scene* scene) :
    IComponent(scene),
    light_(0),
    attached_(false),
    type(this, "light type", LT_Point),
    diffColor(this, "diffuse color", Color(1.0f, 1.0f, 1.0f)),
    specColor(this, "specular color", Color(0.0f, 0.0f, 0.0f)),
    castShadows(this, "cast shadows", false),
    range(this, "light range", 25.0f),
    brightness(this, "brightness", 1.0f),
    constAtten(this, "constant atten", 0.0f),
    linearAtten(this, "linear atten", 0.01f),
    quadraAtten(this, "quadratic atten", 0.01f),
    innerAngle(this, "light inner angle", 30.0f),
    outerAngle(this, "light outer angle", 40.0f)
{
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

    if (scene)
    {
        world_ = scene->GetWorld<OgreWorld>();
        if (!world_.expired() && scene->ViewEnabled())
        {
            OgreWorldPtr world = world_.lock();
            Ogre::SceneManager* sceneMgr = world->OgreSceneManager();
            light_ = sceneMgr->createLight(world->GetUniqueObjectName("EC_Light"));
            
            connect(this, SIGNAL(ParentEntitySet()), SLOT(UpdateSignals()));
        }
    }
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
        Ogre::SceneManager* sceneMgr = world_.lock()->OgreSceneManager();
        sceneMgr->destroyLight(light_);
        light_ = 0;
    }
}

void EC_Light::UpdateSignals()
{
    Entity* parent = ParentEntity();
    if (parent)
    {
        connect(parent, SIGNAL(ComponentAdded(IComponent*, AttributeChange::Type)), SLOT(OnComponentAdded(IComponent*, AttributeChange::Type)));
        connect(parent, SIGNAL(ComponentRemoved(IComponent*, AttributeChange::Type)), SLOT(OnComponentRemoved(IComponent*, AttributeChange::Type)));
        CheckForPlaceable();
    }
}

void EC_Light::CheckForPlaceable()
{
    if (!placeable_)
    {
        Entity* entity = ParentEntity();
        if (entity)
        {
            ComponentPtr placeable = entity->GetComponent(EC_Placeable::TypeNameStatic());
            if (placeable)
                SetPlaceable(placeable);
        }
    }
}

void EC_Light::OnComponentAdded(IComponent* component, AttributeChange::Type change)
{
    if (component->TypeId() == EC_Placeable::TypeIdStatic())
        CheckForPlaceable();
}

void EC_Light::OnComponentRemoved(IComponent* component, AttributeChange::Type change)
{
    if (component == placeable_.get())
        SetPlaceable(ComponentPtr());
}

void EC_Light::SetPlaceable(const ComponentPtr &placeable)
{
    if (!light_)
        return;
    
    if (placeable && dynamic_cast<EC_Placeable*>(placeable.get()) == 0)
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
    if (light_ && placeable_ && !attached_)
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(light_);
        attached_ = true;
    }
}

void EC_Light::DetachLight()
{
    if (light_ && placeable_ && attached_)
    {
        EC_Placeable* placeable = checked_static_cast<EC_Placeable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(light_);
        attached_ = false;
    }
}

void EC_Light::AttributesChanged()
{
    if (!light_)
        return;
    
    Ogre::Light::LightTypes ogreType = Ogre::Light::LT_POINT;

    switch(type.Get())
    {
    case LT_Spot:
        ogreType = Ogre::Light::LT_SPOTLIGHT;
        break;
    case LT_Directional:
        ogreType = Ogre::Light::LT_DIRECTIONAL;
        break;
    }
    
    try
    {
        float b = std::max(brightness.Get(), 1e-3f);
        Color diff = diffColor.Get();
        Color spec = specColor.Get();
        // Because attenuation equation (and therefore brightness) does not affect directional lights,
        // manually multiply the colors by brightness for a dir.light
        if (ogreType == Ogre::Light::LT_DIRECTIONAL)
        {
            diff.r *= b;
            diff.g *= b;
            diff.b *= b;
            spec.r *= b;
            spec.g *= b;
            spec.b *= b;
        }
        
        light_->setType(ogreType);
        light_->setCastShadows(castShadows.Get());
        light_->setDiffuseColour(diff);
        light_->setSpecularColour(spec);
        light_->setAttenuation(range.Get(), constAtten.Get() / b , linearAtten.Get() / b, quadraAtten.Get() / b);
        // Note: Ogre throws exception if we try to set this when light is not spotlight
        if (ogreType == Ogre::Light::LT_SPOTLIGHT)
            light_->setSpotlightRange(Ogre::Degree(innerAngle.Get()), Ogre::Degree(outerAngle.Get()));
    }
    catch(const Ogre::Exception& e)
    {
        LogError("Exception while setting EC_Light parameters to Ogre: " + std::string(e.what()));
    }
}
