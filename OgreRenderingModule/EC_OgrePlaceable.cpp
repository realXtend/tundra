// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"

#include "Ogre.h"

namespace OgreRenderer
{
    EC_OgrePlaceable::EC_OgrePlaceable(Foundation::ModuleInterface* module) :
        module_(static_cast<OgreRenderingModule*>(module)),
        scene_node_(NULL)
    {
        Ogre::SceneManager* scene_mgr = module_->GetRenderer()->GetSceneManager();
        
        Ogre::SceneNode* scene_node_ = scene_mgr->createSceneNode();
        scene_mgr->getRootSceneNode()->addChild(scene_node_);
    }
    
    EC_OgrePlaceable::~EC_OgrePlaceable()
    {
        if (scene_node_)
        {
            Ogre::SceneManager* scene_mgr = module_->GetRenderer()->GetSceneManager();
            
            scene_mgr->getRootSceneNode()->removeChild(scene_node_);
            scene_mgr->destroySceneNode(scene_node_);
            scene_node_ = NULL;
        }
    }
}