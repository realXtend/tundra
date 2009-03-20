// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"

#include "Ogre.h"

namespace OgreRenderer
{
    EC_OgreMesh::EC_OgreMesh(Foundation::ModuleInterface* module) :
        module_(static_cast<OgreRenderingModule*>(module)),
        ogre_entity_(NULL)
    {
    }
    
    EC_OgreMesh::~EC_OgreMesh()
    {
        RemoveMesh();
    }
    
    bool EC_OgreMesh::SetMesh(const std::string& mesh_name)
    {
        assert (placeable_);
        EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        
        Ogre::SceneManager* scene_mgr = module_->GetRenderer()->GetSceneManager();
        
        RemoveMesh();
        
        try
        {
            ogre_entity_ = scene_mgr->createEntity(node->getName() + "mesh", mesh_name);
            node->attachObject(ogre_entity_);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set mesh " + mesh_name + ":" + e.what());
            return false;
        }
        
        return true;
    }
    
    void EC_OgreMesh::RemoveMesh()
    {
        if (ogre_entity_)
        {
            assert (placeable_);
            EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();

            Ogre::SceneManager* scene_mgr = module_->GetRenderer()->GetSceneManager();
            
            node->detachObject(ogre_entity_);
            scene_mgr->destroyEntity(ogre_entity_);
            ogre_entity_ = NULL;
        }
    }
}