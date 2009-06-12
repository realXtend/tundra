// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreCustomObject.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreCustomObject::EC_OgreCustomObject(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        object_(NULL),
        attached_(false)
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        object_ = scene_mgr->createManualObject(renderer_->GetUniqueObjectName());
        object_->setCastShadows(false);
    }
    
    EC_OgreCustomObject::~EC_OgreCustomObject()
    {
        if (object_)
        {
            DetachObject();

            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            
            scene_mgr->destroyManualObject(object_);
            object_ = NULL;
        }
    }
    
    void EC_OgreCustomObject::SetPlaceable(Foundation::ComponentPtr placeable, Scene::Entity *parent_entity)
    {
        DetachObject();
        placeable_ = placeable;
        AttachObject();

        if (object_)
            object_->setUserAny(Ogre::Any(parent_entity));
    }
    
    void EC_OgreCustomObject::SetDrawDistance(float draw_distance)
    {
        if (object_)
            object_->setRenderingDistance(draw_distance);
    }
    
   void EC_OgreCustomObject::SetCastShadows(bool enabled)
    {
        if (object_)
            object_->setCastShadows(enabled);
    }
    
   void EC_OgreCustomObject::AttachObject()
    {
        if ((placeable_) && (!attached_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->attachObject(object_);
            attached_ = true;
        }
    }
    
    void EC_OgreCustomObject::DetachObject()
    {
        if ((placeable_) && (attached_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->detachObject(object_);
            attached_ = false;
        }
    }
}