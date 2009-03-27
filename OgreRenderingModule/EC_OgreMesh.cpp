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
        Foundation::ComponentInterface(module->GetFramework()),
        renderer_(static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        entity_(NULL),
        attached_(false)
    {
    }
    
    EC_OgreMesh::~EC_OgreMesh()
    {
        RemoveMesh();
    }
    
    void EC_OgreMesh::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        DetachMesh();
        placeable_ = placeable;
        AttachMesh();
    }
    
    bool EC_OgreMesh::SetMesh(const std::string& mesh_name)
    {
        RemoveMesh();
        
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        try
        {
            entity_ = scene_mgr->createEntity(renderer_->GetUniqueObjectName(), mesh_name);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set mesh " + mesh_name + ": " + std::string(e.what()));
            return false;
        }
        
        AttachMesh();
        return true;
    }
    
    void EC_OgreMesh::RemoveMesh()
    {
        if (!entity_)
            return;

        DetachMesh();

        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        scene_mgr->destroyEntity(entity_);
        entity_ = NULL;
    }
    
    Core::uint EC_OgreMesh::GetNumMaterials()
    {
        if (!entity_)
            return 0;
            
        return entity_->getNumSubEntities();
    }
    
    bool EC_OgreMesh::SetMaterial(Core::uint index, const std::string& material_name)
    {
        if (!entity_)
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": no mesh");
            return false;
        }
        
        if (index >= entity_->getNumSubEntities())
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": illegal submesh index " + Core::ToString<Core::uint>(index));
            return false;
        }
        
        try
        {
            entity_->getSubEntity(index)->setMaterialName(material_name);
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": " + std::string(e.what()));
            return false;
        }
        
        return true;
    }
    
    void EC_OgreMesh::DetachMesh()
    {
        if ((!attached_) || (!entity_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->detachObject(entity_);
                
        attached_ = false;
    }
    
    void EC_OgreMesh::AttachMesh()
    {
        if ((attached_) || (!entity_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->attachObject(entity_);
                
        attached_ = true;
    }
}