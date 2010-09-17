// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Light.h"
#include "IModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreConversionUtils.h"
#include "XMLUtilities.h"
#include "RexNetworkUtils.h"
#include "LoggingFunctions.h"

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Light")

#include <Ogre.h>

#include <QDomDocument>

using namespace RexTypes;
using namespace OgreRenderer;

EC_Light::EC_Light(IModule *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    light_(0),
    attached_(false),
    typeAttr_(this, "light type", LT_Point),
    directionAttr_(this, "direction", Vector3df(0.0f, 0.0f, 1.0f)),
    diffColorAttr_(this, "diffuse color", Color(1.0f, 1.0f, 1.0f)),
    specColorAttr_(this, "specular color", Color(0.0f, 0.0f, 0.0f)),
    castShadowsAttr_(this, "cast shadows", false),
    rangeAttr_(this, "light range", 100.0f),
    constAttenAttr_(this, "constant atten", 0.0f),
    linearAttenAttr_(this, "linear atten", 0.01f),
    quadraAttenAttr_(this, "quadratic atten", 0.01f),
    innerAngleAttr_(this, "light inner angle", 30.0f),
    outerAngleAttr_(this, "light outer angle", 40.0f)
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
    typeAttr_.SetMetadata(&typeAttrData);

    boost::shared_ptr<Renderer> renderer = module->GetFramework()->GetServiceManager()->GetService
        <Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    light_ = scene_mgr->createLight(renderer->GetUniqueObjectName());
    
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateOgreLight()));

   

}

EC_Light::~EC_Light()
{
    if (!GetFramework())
        return;

    boost::shared_ptr<Renderer> renderer = GetFramework()->GetServiceManager()->GetService
        <Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    if (light_)
    {
        DetachLight();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroyLight(light_);
        light_ = 0;
    }
}

void EC_Light::SetPlaceable(Foundation::ComponentPtr placeable)
{
    if (dynamic_cast<EC_OgrePlaceable*>(placeable.get()) == 0)
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
    if ((placeable_) && (!attached_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(light_);
        attached_ = true;
    }
}

void EC_Light::DetachLight()
{
    if ((placeable_) && (attached_))
    {
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(light_);
        attached_ = false;
    }
}

void EC_Light::UpdateOgreLight()
{
    // Now the true hack: because we don't (yet) store EC links/references, we hope to find a valid placeable from the entity, and to set it
    if (parent_entity_)
    {
        Foundation::ComponentPtr placeable = parent_entity_->GetComponent(EC_OgrePlaceable::TypeNameStatic());
        if (placeable)
            SetPlaceable(placeable);
        else
            LogError("No EC_OgrePlaceable in entity, EC_Light could not attach itself to scenenode");
    }
    else
        LogError("Parent entity not set, EC_Light could not auto-set placeable");
    
    Ogre::Light::LightTypes ogre_type = Ogre::Light::LT_POINT;

    switch (typeAttr_.Get())
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
        light_->setDirection(ToOgreVector3(directionAttr_.Get()));
        light_->setDiffuseColour(ToOgreColor(diffColorAttr_.Get()));
        light_->setSpecularColour(ToOgreColor(specColorAttr_.Get()));
        light_->setAttenuation(rangeAttr_.Get(), constAttenAttr_.Get(), linearAttenAttr_.Get(), quadraAttenAttr_.Get());
        // Note: Ogre throws exception if we try to set this when light is not spotlight
        if (typeAttr_.Get() == LT_Spot)
            light_->setSpotlightRange(Ogre::Degree(innerAngleAttr_.Get()), Ogre::Degree(outerAngleAttr_.Get()));
    }
    catch (Ogre::Exception& e)
    {
        LogError("Exception while setting EC_Light parameters to Ogre: " + std::string(e.what()));
    }
}

