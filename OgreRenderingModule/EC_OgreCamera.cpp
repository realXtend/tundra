// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_OgreCamera.h"
#include "EC_OgrePlaceable.h"
#include "OgreRenderingModule.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreCamera::EC_OgreCamera(Foundation::ModuleInterface* module) :
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        attached_(false),
        camera_(0)
    {
        RendererPtr renderer = renderer_.lock();
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        Ogre::Viewport* viewport = renderer->GetViewport();
        camera_ = scene_mgr->createCamera(renderer->GetUniqueObjectName());
        
        // Set default values for the camera
        camera_->setNearClipDistance(0.1f);
        camera_->setFarClipDistance(2000.f);
        
        camera_->setAspectRatio(Ogre::Real(viewport->getActualWidth() / Ogre::Real(viewport->getActualHeight())));
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
         
            if (renderer->GetCurrentCamera() == camera_)
                renderer->SetCurrentCamera(0);
                
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();         
            scene_mgr->destroyCamera(camera_);
            camera_ = 0;       
        }
    }
    
    void EC_OgreCamera::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        if (!dynamic_cast<EC_OgrePlaceable*>(placeable.get()))
        {
            OgreRenderingModule::LogError("Attempted to set placeable which is not " + EC_OgrePlaceable::TypeNameStatic().toStdString());
            return;
        }       
        
        DetachCamera();
        placeable_ = placeable;
        AttachCamera();
    }
    
    void EC_OgreCamera::SetNearClip(Real nearclip)
    {
        camera_->setNearClipDistance(nearclip);    
    }
    
    void EC_OgreCamera::SetFarClip(Real farclip)
    {
        // Enforce that farclip doesn't go past renderer's view distance
        if (renderer_.expired())
            return;   
        Renderer* renderer = renderer_.lock().get();
        if (farclip > renderer->GetViewDistance())
            farclip = renderer->GetViewDistance();
        camera_->setFarClipDistance(farclip);
    }
    
    void EC_OgreCamera::SetVerticalFov(Real fov)
    {
        camera_->setFOVy(Ogre::Radian(fov));
    }
    
    void EC_OgreCamera::SetActive()
    {
        if (renderer_.expired())
            return;           
        RendererPtr renderer = renderer_.lock();
        renderer->SetCurrentCamera(camera_);
    }
    
    Real EC_OgreCamera::GetNearClip() const
    {
        return camera_->getNearClipDistance();
    }

    Real EC_OgreCamera::GetFarClip() const
    {
        return camera_->getFarClipDistance();
    }
    
    Real EC_OgreCamera::GetVerticalFov() const
    {
        return camera_->getFOVy().valueRadians();
    }  
    
    bool EC_OgreCamera::IsActive() const
    {
        if (renderer_.expired())
            return false;           
        RendererPtr renderer = renderer_.lock();    
        return renderer->GetCurrentCamera() == camera_;
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
