// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SyncState_h
#define incl_TundraLogicModule_SyncState_h

#include "Foundation.h"
#include "IAttribute.h"
#include "UserConnection.h"
#include "Entity.h"

#include <QString>

#include <map>

namespace TundraLogic
{

//! State of component replication for a specific user
struct ComponentSyncState
{
    uint type_hash_;
    QString name_;
    // Note! These pointers are never dereferenced, and might be invalid. They are just for quick response to AttributeChanged signals.
    std::set<IAttribute*> dirty_attributes_;
};

//! State of entity replication for a specific user
struct EntitySyncState
{
    //! Components that this client is already aware of
    std::vector<ComponentSyncState> components_;
    //! Created/modified components
    std::set<std::pair<uint, QString> > dirty_components_;
    //! Pending removed components (typenamehash, name)
    std::set<std::pair<uint, QString> > removed_components_;
    
    ComponentSyncState* GetOrCreateComponent(uint type_hash, const QString& name)
    {
        // If we want to recreate the component and have a pending remove, remove the remove
        removed_components_.erase(std::make_pair<uint, QString>(type_hash, name));
        ComponentSyncState* old = GetComponent(type_hash, name);
        if (old)
            return old;
        ComponentSyncState newstate;
        newstate.type_hash_ = type_hash;
        newstate.name_ = name;
        components_.push_back(newstate);
        return &components_[components_.size()-1];
    }
    
    ComponentSyncState* GetComponent(uint type_hash, const QString& name)
    {
        for (int i = 0; i < (int)components_.size(); ++i)
        {
            if ((components_[i].type_hash_ == type_hash) && (components_[i].name_ == name))
                return &components_[i];
        }
        return 0;
    }
    
    void RemoveComponent(uint type_hash, const QString& name)
    {
        dirty_components_.erase(std::make_pair<uint, QString>(type_hash, name));
        removed_components_.erase(std::make_pair<uint, QString>(type_hash, name));
        for (int i = 0; i < (int)components_.size(); ++i)
        {
            if ((components_[i].type_hash_ == type_hash) && (components_[i].name_ == name))
                components_.erase(components_.begin() + i);
        }
    }
    
    void OnComponentAdded(uint type_hash, const QString& name)
    {
        dirty_components_.insert(std::make_pair<uint, QString>(type_hash, name));
        removed_components_.erase(std::make_pair<uint, QString>(type_hash, name));
    }
    
    void OnAttributeChanged(uint type_hash, const QString& name, IAttribute* attribute)
    {
        dirty_components_.insert(std::make_pair<uint, QString>(type_hash, name));
        removed_components_.erase(std::make_pair<uint, QString>(type_hash, name));
        // If client already has the component state, dirty the specific attribute
        if (attribute)
        {
            ComponentSyncState* compState = GetComponent(type_hash, name);
            if (compState)
                compState->dirty_attributes_.insert(attribute);
        }
    }
    
    void OnComponentRemoved(uint type_hash, const QString& name)
    {
        removed_components_.insert(std::make_pair<uint, QString>(type_hash, name));
        dirty_components_.erase(std::make_pair<uint, QString>(type_hash, name));
    }
    
    void AckDirty(uint type_hash, const QString& name)
    {
        dirty_components_.erase(std::make_pair<uint, QString>(type_hash, name));
        ComponentSyncState* compState = GetComponent(type_hash, name);
        if (compState)
            compState->dirty_attributes_.clear();
    }
    
    void AckRemove(uint type_hash, const QString& name)
    {
        removed_components_.erase(std::make_pair<uint, QString>(type_hash, name));
    }
};

//! State of scene replication for a specific user
struct SceneSyncState : public IUserData
{
    //! Entities that this client is already aware of
    std::map<entity_id_t, EntitySyncState> entities_;
    //! Created/modified entities
    std::set<entity_id_t> dirty_entities_;
    //! Pending removed entities
    std::set<entity_id_t> removed_entities_;
    
    EntitySyncState* GetOrCreateEntity(entity_id_t id)
    {
        // If we want to recreate the entity and have a pending remove, remove the remove
        removed_entities_.erase(id);
        std::map<entity_id_t, EntitySyncState>::iterator i = entities_.find(id);
        if (i == entities_.end())
        {
            EntitySyncState* newstate = &entities_[id];
            return newstate;
        }
        else
            return &i->second;
    }
    
    EntitySyncState* GetEntity(entity_id_t id)
    {
        std::map<entity_id_t, EntitySyncState>::iterator i = entities_.find(id);
        if (i != entities_.end())
            return &i->second;
        return 0;
    }
    
    void RemoveEntity(entity_id_t id)
    {
        dirty_entities_.erase(id);
        removed_entities_.erase(id);
        entities_.erase(id);
    }
    
    void OnEntityChanged(entity_id_t id)
    {
        dirty_entities_.insert(id);
    }
    
    void OnEntityRemoved(entity_id_t id)
    {
        removed_entities_.insert(id);
    }
    
    void OnAttributeChanged(entity_id_t id, uint type_hash, const QString& name, IAttribute* attribute)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnAttributeChanged(type_hash, name, attribute);
    }
    
    void OnComponentAdded(entity_id_t id, uint type_hash, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentAdded(type_hash, name);
    }
    
    void OnComponentRemoved(entity_id_t id, uint type_hash, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentRemoved(type_hash, name);
    }
    
    void AckDirty(entity_id_t id)
    {
        dirty_entities_.erase(id);
    }
    
    void AckRemove(entity_id_t id)
    {
        removed_entities_.erase(id);
    }
    
    void Clear()
    {
        entities_.clear();
        dirty_entities_.clear();
        removed_entities_.clear();
    }
};

}

#endif
