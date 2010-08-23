// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreParticleSystem.h"
#include "RexTypes.h"

#include <Ogre.h>

namespace OgreRenderer
{
    EC_OgreParticleSystem::EC_OgreParticleSystem(Foundation::ModuleInterface* module) :
        renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
        adjustment_node_(0),
        attached_(false),
        draw_distance_(0.0)
    {
        RendererPtr renderer = renderer_.lock();     
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        adjustment_node_ = scene_mgr->createSceneNode();
    }
    
    EC_OgreParticleSystem::~EC_OgreParticleSystem()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();   
            
        DetachSystems();
        RemoveParticleSystems();
        
        if (adjustment_node_)
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            scene_mgr->destroySceneNode(adjustment_node_);
            adjustment_node_ = 0;
        }
    }
    
    void EC_OgreParticleSystem::SetPlaceable(Foundation::ComponentPtr placeable)
    {
        if (!dynamic_cast<EC_OgrePlaceable*>(placeable.get()))
        {
            OgreRenderingModule::LogError("Attempted to set placeable which is not " + EC_OgrePlaceable::TypeNameStatic().toStdString());
            return;
        }
        
        DetachSystems();
        placeable_ = placeable;
        AttachSystems();
    }
    
    bool EC_OgreParticleSystem::AddParticleSystem(const std::string& system_name)
    {
        if (renderer_.expired())
            return false;
        RendererPtr renderer = renderer_.lock();   
            
        try
        {
            Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
            Ogre::ParticleSystem* system = scene_mgr->createParticleSystem(renderer->GetUniqueObjectName(), system_name);
            if (system)
            {
                adjustment_node_->attachObject(system);
                systems_.push_back(system);
                system_names_.push_back(system_name);
                return true;
            }
        }
        catch (Ogre::Exception& e)
        {
            OgreRenderingModule::LogError("Could not add particle system " + system_name + ": " + std::string(e.what()));
        }
        
        return false;
    }
    
    Ogre::ParticleSystem* EC_OgreParticleSystem::GetParticleSystem(uint index) const
    {
       if (index >= systems_.size())
            return 0;
        
        return systems_[index];
    }
    
    const std::string& EC_OgreParticleSystem::GetParticleSystemName(uint index) const
    {
        static const std::string empty;
        if (index >= system_names_.size())
            return empty;
        
        return system_names_[index];
    }
    
    bool EC_OgreParticleSystem::RemoveParticleSystem(uint index)
    {
        if (renderer_.expired())
            return false;
        RendererPtr renderer = renderer_.lock();  
            
        if (index >= systems_.size())
            return false;
        
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        
        adjustment_node_->detachObject(systems_[index]);
        scene_mgr->destroyParticleSystem(systems_[index]);
        systems_.erase(systems_.begin() + index);
        system_names_.erase(system_names_.begin() + index);
        
        return true;
    }
    
    void EC_OgreParticleSystem::RemoveParticleSystems()
    {
        if (renderer_.expired())
            return;
        RendererPtr renderer = renderer_.lock();  
            
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        
        for (uint i = 0; i < systems_.size(); ++i)
        {
            adjustment_node_->detachObject(systems_[i]);
            scene_mgr->destroyParticleSystem(systems_[i]);
        }
        
        systems_.clear();
        system_names_.clear();
    }
    
    void EC_OgreParticleSystem::SetAdjustPosition(const Vector3df& position)
    {
        adjustment_node_->setPosition(Ogre::Vector3(position.x, position.y, position.z));
    }

    void EC_OgreParticleSystem::SetAdjustOrientation(const Quaternion& orientation)
    {
        adjustment_node_->setOrientation(Ogre::Quaternion(orientation.w, orientation.x, orientation.y, orientation.z));
    }   
    
    void EC_OgreParticleSystem::SetAdjustScale(const Vector3df& scale)
    {
        adjustment_node_->setScale(Ogre::Vector3(scale.x, scale.y, scale.z));
    }
    
    Vector3df EC_OgreParticleSystem::GetAdjustPosition() const
    {
        const Ogre::Vector3& pos = adjustment_node_->getPosition();
        return Vector3df(pos.x, pos.y, pos.z);
    }
    
    Quaternion EC_OgreParticleSystem::GetAdjustOrientation() const
    {
        const Ogre::Quaternion& orientation = adjustment_node_->getOrientation();
        return Quaternion(orientation.x, orientation.y, orientation.z, orientation.w);
    }     
    
    Vector3df EC_OgreParticleSystem::GetAdjustScale() const
    {
        const Ogre::Vector3& scale = adjustment_node_->getScale();
        return Vector3df(scale.x, scale.y, scale.z);
    }
    
    void EC_OgreParticleSystem::SetDrawDistance(float draw_distance)
    {
        draw_distance_ = draw_distance;
        for (uint i = 0; i < systems_.size(); ++i)
        {
            systems_[i]->setRenderingDistance(draw_distance_);
        }
    }
    
    void EC_OgreParticleSystem::DetachSystems()
    {
        if ((!attached_) && (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->removeChild(adjustment_node_);
        
        attached_ = false;
    }
    
    void EC_OgreParticleSystem::AttachSystems()
    {
        if ((attached_) || (!placeable_))
            return;
            
        EC_OgrePlaceable* placeable = checked_static_cast<EC_OgrePlaceable*>(placeable_.get());
        Ogre::SceneNode* node = placeable->GetSceneNode();
        node->addChild(adjustment_node_);
                
        attached_ = true;
    }
    
}