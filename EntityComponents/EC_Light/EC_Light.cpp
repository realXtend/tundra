// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_Light.h"
#include "ModuleInterface.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "Entity.h"
#include "OgreConversionUtils.h"
#include "XMLUtilities.h"
#include "RexNetworkUtils.h"

#include <Ogre.h>

#include <QDomDocument>

using namespace RexTypes;
using namespace OgreRenderer;

#define LogError(msg) Poco::Logger::get("EC_Light").error(std::string("Error: ") + msg);
#define LogInfo(msg) Poco::Logger::get("EC_Light").information(msg);

EC_Light::EC_Light(Foundation::ModuleInterface *module) :
    Foundation::ComponentInterface(module->GetFramework()),
    light_(0),
    typeAttr_(this, "light type", LT_Point),
    directionAttr_(this, "direction", Vector3df(0.0f, 0.0f, 1.0f)),
    diffColorAttr_(this, "diffuse color", Color(1.0f, 1.0f, 1.0f)),
    specColorAttr_(this, "specular color", Color(0.0f, 0.0f, 0.0f)),
    castShadowsAttr_(this, "cast shadows", false),
    rangeAttr_(this, "light range", 10.0f),
    constAttenAttr_(this, "constant atten", 1.0f),
    linearAttenAttr_(this, "linear atten", 0.0f),
    quadraAttenAttr_(this, "quadratic atten", 0.0f),
    innerAngleAttr_(this, "light inner angle", 30.0f),
    outerAngleAttr_(this, "light outer angle", 40.0f)
{
    boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService
        <Renderer>(Foundation::Service::ST_Renderer).lock();
    if (!renderer)
        return;

    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    light_ = scene_mgr->createLight(renderer->GetUniqueObjectName());
    
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateOgreLight()));
}

EC_Light::~EC_Light()
{
    boost::shared_ptr<Renderer> renderer = framework_->GetServiceManager()->GetService
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

void EC_Light::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
{
    QDomElement comp_element = BeginSerialization(doc, base_element);

    std::string type;
    switch (typeAttr_.Get())
    {
    case LT_Point:
        type = "point";
        break;
    case LT_Spot:
        type = "spot";
        break;
    case LT_Directional:
        type = "directional";
        break;
    }
    WriteAttribute(doc, comp_element, "type", type);
    
    WriteAttribute(doc, comp_element, "direction", WriteVector3(directionAttr_.Get()));//direction_));
    WriteAttribute(doc, comp_element, "diffuse", WriteColor(diffColorAttr_.Get()));
    WriteAttribute(doc, comp_element, "specular", WriteColor(specColorAttr_.Get()));
    WriteAttribute(doc, comp_element, "shadows", WriteBool(castShadowsAttr_.Get()));
    WriteAttribute(doc, comp_element, "range", WriteReal(rangeAttr_.Get()));
    
    Vector3df atten(constAttenAttr_.Get(), linearAttenAttr_.Get(), quadraAttenAttr_.Get());
    WriteAttribute(doc, comp_element, "attenuation", WriteVector3(atten));
    
    WriteAttribute(doc, comp_element, "spot inner", WriteReal(innerAngleAttr_.Get()));
    WriteAttribute(doc, comp_element, "spot outer", WriteReal(outerAngleAttr_.Get()));
}

void EC_Light::DeserializeFrom(QDomElement& element, Foundation::ChangeType change)
{
    // Check that type is right, otherwise do nothing
    if (!BeginDeserialization(element))
        return;
    
    typeAttr_.Set(LT_Point, Foundation::Local);
    std::string type = ReadAttribute(element, "type");
    if (type == "directional")
        typeAttr_.Set(LT_Directional, Foundation::Local);
    if (type == "spot")
        typeAttr_.Set(LT_Spot, Foundation::Local);
    
    //direction_ = ParseVector3(ReadAttribute(element, "direction"));
    directionAttr_.Set(ParseVector3(ReadAttribute(element, "direction")), Foundation::Local);
    diffColorAttr_.Set(ParseColor(ReadAttribute(element, "diffuse")), Foundation::Local);
    specColorAttr_.Set(ParseColor(ReadAttribute(element, "specular")), Foundation::Local);
    castShadowsAttr_.Set(ParseBool(ReadAttribute(element, "shadows")), Foundation::Local);
    rangeAttr_.Set(ParseReal(ReadAttribute(element, "range")), Foundation::Local);
    
    Vector3df atten = ParseVector3(ReadAttribute(element, "attenuation"));
    constAttenAttr_.Set(atten.x, Foundation::Local);
    linearAttenAttr_.Set(atten.y, Foundation::Local);
    quadraAttenAttr_.Set(atten.z, Foundation::Local);
    
    innerAngleAttr_.Set(ParseReal(ReadAttribute(element, "spot inner")), Foundation::Local);
    outerAngleAttr_.Set(ParseReal(ReadAttribute(element, "spot outer")), Foundation::Local);
    
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
    
    emit OnChanged();
}

void EC_Light::SetPlaceable(Foundation::ComponentPtr placeable)
{
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
        LogError("Exception while setting EC_Light parameters to Ogre: " + e.what());
    }
}
