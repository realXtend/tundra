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
        object_(0),
        entity_(0),
        attached_(false),
        cast_shadows_(false),
        draw_distance_(0.0f)
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        object_ = scene_mgr->createManualObject(renderer_->GetUniqueObjectName());
    }
    
    EC_OgreCustomObject::~EC_OgreCustomObject()
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        DestroyEntity();

        if (object_)
        {
            scene_mgr->destroyManualObject(object_);
            object_ = 0;
        }
    }
    
    void EC_OgreCustomObject::SetPlaceable(Foundation::ComponentPtr placeable, Scene::Entity* parent_entity)
    {
        if (!dynamic_cast<EC_OgrePlaceable*>(placeable.get()))
        {
            OgreRenderingModule::LogError("Attempted to set placeable which is not " + EC_OgrePlaceable::NameStatic());
            return;
        }
        
        parent_entity_ = parent_entity;
        
        DetachEntity();
        placeable_ = placeable;
        AttachEntity();
    }
    
    bool EC_OgreCustomObject::CommitChanges()
    {
        DestroyEntity();
        
        if (!object_->getNumSections())
            return true;
            
        try
        {
            std::string mesh_name = renderer_->GetUniqueObjectName();
            object_->convertToMesh(mesh_name);
            object_->clear();
        
            Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();

            entity_ = scene_mgr->createEntity(renderer_->GetUniqueObjectName(), mesh_name);
            if (entity_)
            {
                AttachEntity();
                entity_->setRenderingDistance(draw_distance_);
                entity_->setCastShadows(cast_shadows_);
                entity_->setUserAny(Ogre::Any(parent_entity_));
            }
            else
            {
                OgreRenderingModule::LogError("Could not create entity from manualobject mesh");
                return false;
            }
        }   
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not convert manualobject to mesh: " + std::string(e.what()));
            return false;
        }
        
        return true;
    }
    
    void EC_OgreCustomObject::SetDrawDistance(float draw_distance)
    {
        draw_distance_ = draw_distance;
        if (entity_)
            entity_->setRenderingDistance(draw_distance);
    }
    
    void EC_OgreCustomObject::SetCastShadows(bool enabled)
    {
        cast_shadows_ = enabled;
        if (entity_)
            entity_->setCastShadows(enabled);
    }
    
    bool EC_OgreCustomObject::SetMaterial(uint index, const std::string& material_name)
    {
        if (!entity_)
            return false;
        
        if (index >= entity_->getNumSubEntities())
        {
            OgreRenderingModule::LogError("Could not set material " + material_name + ": illegal submesh index " + ToString<uint>(index));
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
    
    uint EC_OgreCustomObject::GetNumMaterials() const
    {
        if (!entity_)
            return 0;
            
        return entity_->getNumSubEntities();
    }
    
    const std::string& EC_OgreCustomObject::GetMaterialName(uint index) const
    {
        const static std::string empty;
        
        if (!entity_)
            return empty;
        
        if (index >= entity_->getNumSubEntities())
            return empty;
        
        return entity_->getSubEntity(index)->getMaterialName();
    }    
           
    void EC_OgreCustomObject::AttachEntity()
    {
        if ((placeable_) && (!attached_) && (entity_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->attachObject(entity_);
            attached_ = true;
        }
    }
    
    void EC_OgreCustomObject::DetachEntity()
    {
        if ((placeable_) && (attached_) && (entity_))
        {
            EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
            Ogre::SceneNode* node = placeable->GetSceneNode();
            node->detachObject(entity_);
            attached_ = false;
        }
    }
    
    void EC_OgreCustomObject::DestroyEntity()
    {
        Ogre::SceneManager* scene_mgr = renderer_->GetSceneManager();
        
        if (entity_)
        {
            DetachEntity();
            std::string mesh_name = entity_->getMesh()->getName();
            scene_mgr->destroyEntity(entity_);
            entity_ = 0;
            try
            {
                Ogre::MeshManager::getSingleton().remove(mesh_name);
            }
            catch (...) {}
        }
    }

	void EC_OgreCustomObject::GetBoundingBox(Vector3df& min, Vector3df& max) const
	{
        if (!entity_)
        {
            min = Vector3df(0.0, 0.0, 0.0);
            max = Vector3df(0.0, 0.0, 0.0);
            return;
        }
     
        const Ogre::AxisAlignedBox& bbox = entity_->getBoundingBox();//getMesh()->getBounds();
        const Ogre::Vector3& bboxmin = bbox.getMinimum();
        const Ogre::Vector3& bboxmax = bbox.getMaximum();
        
        min = Vector3df(bboxmin.x, bboxmin.y, bboxmin.z);
        max = Vector3df(bboxmax.x, bboxmax.y, bboxmax.z);
	}
}