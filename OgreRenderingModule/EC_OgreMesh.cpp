// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Foundation.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreMesh::EC_OgreMesh(Foundation::ModuleInterface* module) :
        renderer_(static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        entity_(NULL)
    {
    }
    
    EC_OgreMesh::~EC_OgreMesh()
    {
        RemoveMesh();
    }
    
    bool EC_OgreMesh::SetMesh(const std::string& mesh_name)
    {
        if (!placeable_)
        {
            OgreRenderingModule::LogError("Could not set mesh: placeable not set");
            return false;
        }

        RemoveMesh();
        
        EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        try
        {
            entity_ = scene_mgr->createEntity(renderer_->GetUniqueObjectName(), mesh_name);
            node->attachObject(entity_);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set mesh " + mesh_name + ": " + e.what());
            return false;
        }
        
        return true;
    }
    
    void EC_OgreMesh::RemoveMesh()
    {
        if ((!placeable_) || (!entity_))
            return;

        EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();

        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        node->detachObject(entity_);
        scene_mgr->destroyEntity(entity_);
        entity_ = NULL;
    }
    
    Core::uint EC_OgreMesh::GetNumMaterials()
    {
        if (!entity_)
            return 0;
            
        return entity_->getNumSubEntities();
    }
    
    bool EC_OgreMesh::SetMaterial(unsigned index, const std::string& material_name)
    {
        if (!entity_)
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": no mesh");
            return false;
        }
        
        if (index >= entity_->getNumSubEntities())
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": illegal submesh index " + boost::lexical_cast<std::string>(index));
            return false;
        }
        
        try
        {
            entity_->getSubEntity(index)->setMaterialName(material_name);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": " + e.what());
            return false;
        }
        
        return true;
    }
}