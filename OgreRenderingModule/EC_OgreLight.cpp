// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreLight.h"
#include "OgreConversionUtils.h"
#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreLight::EC_OgreLight(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        light_(0),
        attached_(false)
    {
        RendererPtr renderer = renderer_.lock();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        light_ = scene_mgr->createLight(renderer->GetUniqueObjectName());
    }
    
    EC_OgreLight::~EC_OgreLight()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();   
        
        if (light_)
        {
            DetachLight();
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            scene_mgr->destroyLight(light_);
            light_ = 0;
        }
    }
    
    void EC_OgreLight::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        DetachLight();
        placeable_ = placeable;
        AttachLight();
    }
    
    void EC_OgreLight::SetType(Type type)
    {
        Ogre::Light::LightTypes ogre_type = Ogre::Light::LT_POINT;

        switch (type)
        {
            case LT_Spot:
            ogre_type = Ogre::Light::LT_SPOTLIGHT;
            break;
            
            case LT_Directional:
            ogre_type = Ogre::Light::LT_DIRECTIONAL;
            break;
        }
        
        light_->setType(ogre_type);
    }
    
    void EC_OgreLight::SetColor(const Color& color)
    {
        light_->setDiffuseColour(ToOgreColor(color));
    }
    
    void EC_OgreLight::SetAttenuation(float range, float constant, float linear, float quad)
    {
        light_->setAttenuation(range, constant, linear, quad);
    }
    
    void EC_OgreLight::SetDirection(const Vector3df& direction)
    {
        light_->setDirection(ToOgreVector3(direction));
    }
        
    void EC_OgreLight::SetCastShadows(const bool &enabled)
    {
        light_->setCastShadows(enabled);
    }
    
    void EC_OgreLight::AttachLight()
    {
        if ((placeable_) && (!attached_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->attachObject(light_);
            attached_ = true;
        }
    }
    
    void EC_OgreLight::DetachLight()
    {
        if ((placeable_) && (attached_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->detachObject(light_);
            attached_ = false;
        }
    }
}
