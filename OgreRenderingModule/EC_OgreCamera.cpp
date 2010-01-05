// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OgreCamera.h"
#include "EC_OgrePlaceable.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreCamera::EC_OgreCamera(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        attached_(false),
        camera_(0)
    {   
        RendererPtr renderer = renderer_.lock();               
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager(); 
        camera_ = scene_mgr->createCamera(renderer->GetUniqueObjectName());  
        
        // Set default values for the camera
        camera_->setNearClipDistance(0.1f);
        camera_->setFarClipDistance(2000.f);
        
        // Roll the view because of OpenSim coordinate axes
        camera_->setFixedYawAxis(true, Ogre::Vector3::UNIT_Z);
        camera_->roll(Ogre::Radian(Ogre::Math::HALF_PI));
        
        // Respond to viewport size automatically
        camera_->setAutoAspectRatio(true);
    }
    
    EC_OgreCamera::~EC_OgreCamera()
    {
        if (renderer_.expired())
            return;
            
        DetachCamera();
        
        if (camera_)
        {
            RendererPtr renderer = renderer_.lock();               
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();         
            scene_mgr->destroyCamera(camera_);
            camera_ = 0;       
        }
    }
    
    void EC_OgreCamera::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        if (!dynamic_cast<EC_OgrePlaceable*>(placeable.get()))
        {
            OgreRenderingModule::LogError("Attempted to set placeable which is not " + EC_OgrePlaceable::NameStatic());
            return;
        }       
        
        DetachCamera();
        placeable_ = placeable;
        AttachCamera();
    }
    
    void EC_OgreCamera::DetachCamera()
    {
        if ((!attached_) || (!camera_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(camera_);
                
        attached_ = false;
    }
    
    void EC_OgreCamera::AttachCamera()
    {
        if ((attached_) || (!camera_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(camera_);
                
        attached_ = true;
    }
  
}
