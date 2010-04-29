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
    type_(LT_Point),
    direction_(Vector3df(0.0f, 0.0f, 1.0f)),
    diffuse_color_(Color(1.0f, 1.0f, 1.0f)),
    specular_color_(Color(0.0f, 0.0f, 0.0f)),
    cast_shadows_(false),
    range_(10.0f),
    constant_atten_(1.0f),
    linear_atten_(0.0f),
    quadratic_atten_(0.0f),
    inner_angle_(30.0f),
    outer_angle_(40.0f),
    attached_(false)
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
    switch (type_)
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
    
    WriteAttribute(doc, comp_element, "direction", WriteVector3(direction_));
    WriteAttribute(doc, comp_element, "diffuse", WriteColor(diffuse_color_));
    WriteAttribute(doc, comp_element, "specular", WriteColor(specular_color_));
    WriteAttribute(doc, comp_element, "shadows", WriteBool(cast_shadows_));
    WriteAttribute(doc, comp_element, "range", WriteReal(range_));
    
    Vector3df atten(constant_atten_, linear_atten_, quadratic_atten_);
    WriteAttribute(doc, comp_element, "attenuation", WriteVector3(atten));
    
    WriteAttribute(doc, comp_element, "spot inner", WriteReal(inner_angle_));
    WriteAttribute(doc, comp_element, "spot outer", WriteReal(outer_angle_));
}

void EC_Light::DeserializeFrom(QDomElement& element, Foundation::ChangeType change)
{
    // Check that type is right, otherwise do nothing
    if (!BeginDeserialization(element))
        return;
    
    type_ = LT_Point;
    std::string type = ReadAttribute(element, "type");
    if (type == "directional")
        type_ = LT_Directional;
    if (type == "spot")
        type_ = LT_Spot;
    
    direction_ = ParseVector3(ReadAttribute(element, "direction"));
    diffuse_color_ = ParseColor(ReadAttribute(element, "diffuse"));
    specular_color_ = ParseColor(ReadAttribute(element, "specular"));
    cast_shadows_ = ParseBool(ReadAttribute(element, "shadows"));
    range_ = ParseReal(ReadAttribute(element, "range"));
    
    Vector3df atten = ParseVector3(ReadAttribute(element, "attenuation"));
    constant_atten_ = atten.x;
    linear_atten_ = atten.y;
    quadratic_atten_ = atten.z;
    
    inner_angle_ = ParseReal(ReadAttribute(element, "spot inner"));
    outer_angle_ = ParseReal(ReadAttribute(element, "spot outer"));
    
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
    
    OnChanged();
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

    switch (type_)
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
        light_->setDirection(ToOgreVector3(direction_));
        light_->setDiffuseColour(ToOgreColor(diffuse_color_));
        light_->setSpecularColour(ToOgreColor(specular_color_));
        light_->setAttenuation(range_, constant_atten_, linear_atten_, quadratic_atten_);
        // Note: Ogre throws exception if we try to set this when light is not spotlight
        if (type_ == LT_Spot)
            light_->setSpotlightRange(Ogre::Degree(inner_angle_), Ogre::Degree(outer_angle_));
    }
    catch (Ogre::Exception& e)
    {
        LogError("Exception while setting EC_Light parameters to Ogre: " + e.what());
    }
}
