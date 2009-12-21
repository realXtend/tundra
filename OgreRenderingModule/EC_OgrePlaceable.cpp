// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgrePlaceable::EC_OgrePlaceable(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        scene_node_(0),
        attached_(false),
        select_priority_(0)
    {
        RendererPtr renderer = renderer_.lock();      
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_node_ = scene_mgr->createSceneNode();
    }
    
    EC_OgrePlaceable::~EC_OgrePlaceable()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        if (scene_node_)
        {
            DetachNode();
            
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            scene_mgr->destroySceneNode(scene_node_);
            scene_node_ = 0;
        }
    }
    
    void EC_OgrePlaceable::SetParent(Foundation::ComponentPtr placeable)
    {
        if (!dynamic_cast<EC_OgrePlaceable*>(placeable.get()))
        {
            OgreRenderingModule::LogError("Attempted to set parent placeable which is not " + NameStatic());
            return;
        }
        DetachNode();
        parent_ = placeable;
        AttachNode();
    }
    
    Vector3df EC_OgrePlaceable::GetPosition() const
    {
        const Ogre::Vector3& pos = scene_node_->getPosition();
        return Vector3df(pos.x, pos.y, pos.z);
    }
    
    Quaternion EC_OgrePlaceable::GetOrientation() const
    {
        const Ogre::Quaternion& orientation = scene_node_->getOrientation();
        return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    }
    
    Vector3df EC_OgrePlaceable::GetScale() const
    {
        const Ogre::Vector3& scale = scene_node_->getScale();
        return Vector3df(scale.x, scale.y, scale.z);
    }
    
    void EC_OgrePlaceable::SetPosition(const Vector3df& position)
    {
        scene_node_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
        AttachNode(); // Nodes become visible only after having their position set at least once
    }

    void EC_OgrePlaceable::SetOrientation(const Quaternion& orientation)
    {
        scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    }
    
    void EC_OgrePlaceable::SetScale(const Vector3df& scale)
    {
        scene_node_->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
    }

    void EC_OgrePlaceable::AttachNode()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        if (attached_)
            return;
                
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = checked_static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetSceneNode();
        }
        
        parent_node->addChild(scene_node_);
        attached_ = true;
    }
    
    void EC_OgrePlaceable::DetachNode()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        if (!attached_)
            return;
            
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = checked_static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetSceneNode();
        }
        
        parent_node->removeChild(scene_node_);
        attached_ = false;
    }
}