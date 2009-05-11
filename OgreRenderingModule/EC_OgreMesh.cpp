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
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        entity_(NULL),
        adjustment_node_(NULL),
        attached_(false),
        scale_to_unity_(false)
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        adjustment_node_ = scene_mgr->createSceneNode();        
    }
    
    EC_OgreMesh::~EC_OgreMesh()
    {
        RemoveMesh();
        
        if (adjustment_node_)
        {            
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
            scene_mgr->destroySceneNode(adjustment_node_);
            adjustment_node_ = NULL;
        }        
    }
    
    void EC_OgreMesh::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        DetachEntity();
        placeable_ = placeable;
        AttachEntity();
    }
    
    void EC_OgreMesh::SetScaleToUnity(bool enable)
    {
        scale_to_unity_ = enable;
        ScaleEntity();
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
        
        AttachEntity();
        ScaleEntity();
        return true;
    }
    
    void EC_OgreMesh::RemoveMesh()
    {
        if (!entity_)
            return;

        DetachEntity();

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
    
    const std::string& EC_OgreMesh::GetMaterialName(Core::uint index)
    {
        const static std::string empty;
        
        if (!entity_)
            return empty;
        
        if (index >= entity_->getNumSubEntities())
            return empty;
        
        return entity_->getSubEntity(index)->getMaterialName();
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
    
    void EC_OgreMesh::DetachEntity()
    {
        if ((!attached_) || (!entity_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        adjustment_node_->detachObject(entity_);
        node->removeChild(adjustment_node_);
                
        attached_ = false;
    }
    
    void EC_OgreMesh::AttachEntity()
    {
        if ((attached_) || (!entity_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->addChild(adjustment_node_);
        adjustment_node_->attachObject(entity_);
                
        attached_ = true;
    }
    
    const std::string& EC_OgreMesh::GetMeshName() const
    {
        static std::string empty_name;
        
        if (!entity_)
            return empty_name;
        else
            return entity_->getMesh()->getName();
    }
    
    void EC_OgreMesh::ScaleEntity()
    {
        if (!entity_)
            return;
     
        Ogre::Vector3 scale(1.0, 1.0, 1.0);
        
        if (scale_to_unity_)
        {
            const Ogre::AxisAlignedBox& bbox = entity_->getBoundingBox();
        
            Ogre::Vector3 size = bbox.getMaximum() - bbox.getMinimum();
            if (size.x != 0.0) scale.x /= size.x;
            if (size.y != 0.0) scale.y /= size.y;
	        if (size.z != 0.0) scale.z /= size.z;                 
        }
        
        adjustment_node_->setScale(scale);
    }
}