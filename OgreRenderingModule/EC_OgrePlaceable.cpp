// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgrePlaceable::EC_OgrePlaceable(Foundation::ModuleInterface* module) :
        renderer_(static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        scene_node_(NULL)
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        scene_node_ = scene_mgr->createSceneNode();
        
        AttachNode();
    }
    
    EC_OgrePlaceable::~EC_OgrePlaceable()
    {
        if (scene_node_)
        {
            DetachNode();
            
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            scene_mgr->destroySceneNode(scene_node_);
            scene_node_ = NULL;
        }
    }
    
    void EC_OgrePlaceable::SetParent(Foundation::ComponentPtr placeable)
    {
        DetachNode();
        parent_ = placeable;
        AttachNode();
    }
    
    Core::Vector3df EC_OgrePlaceable::GetPosition() const
    {
        const Ogre::Vector3& pos = scene_node_->getPosition();
        return Core::Vector3df(pos.x, pos.y, pos.z);
    }
    
    Core::Quaternion EC_OgrePlaceable::GetOrientation() const
    {
        const Ogre::Quaternion& orientation = scene_node_->getOrientation();
        return Core::Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    }
    
    Core::Vector3df EC_OgrePlaceable::GetScale() const
    {
        const Ogre::Vector3& scale = scene_node_->getScale();
        return Core::Vector3df(scale.x, scale.y, scale.z);
    }
    
    void EC_OgrePlaceable::SetPosition(const Core::Vector3df& position)
    {
        scene_node_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
    }

    void EC_OgrePlaceable::SetOrientation(const Core::Quaternion& orientation)
    {
        scene_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    }
    
    void EC_OgrePlaceable::SetScale(const Core::Vector3df& scale)
    {
        scene_node_->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
    }

    void EC_OgrePlaceable::AttachNode()
    {
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetSceneNode();
        }
        
        parent_node->addChild(scene_node_);
    }
    
    void EC_OgrePlaceable::DetachNode()
    {
        Ogre::SceneNode* parent_node;
        
        if (!parent_)
        {
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            parent_node = scene_mgr->getRootSceneNode();
        }
        else
        {
            EC_OgrePlaceable* parent = static_cast<EC_OgrePlaceable*>(parent_.get());
            parent_node = parent->GetSceneNode();
        }
        
        parent_node->removeChild(scene_node_);
    }
}