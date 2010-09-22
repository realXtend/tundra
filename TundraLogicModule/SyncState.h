// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SyncState_h
#define incl_TundraLogicModule_SyncState_h

#include "Foundation.h"
#include "UserConnection.h"
#include "Entity.h"

#include <QString>

#include <map>

namespace TundraLogic
{

//! State of component replication for a specific user
struct ComponentSyncState
{
    QString type_name_;
    QString name_;
    //! Last sent component data (full binary state from IComponent::SerializeToBinary)
    std::vector<unsigned char> data_;
};

//! State of entity replication for a specific user
struct EntitySyncState
{
    //! Components that this client is already aware of
    std::vector<ComponentSyncState> components_;
    //! Created/modified components
    std::set<std::pair<QString, QString> > dirty_components_;
    //! Pending removed components (typename, name)
    std::set<std::pair<QString, QString> > removed_components_;
    
    ComponentSyncState* GetOrCreateComponent(const QString& type_name, const QString& name)
    {
        // If we want to recreate the component and have a pending remove, remove the remove
        removed_components_.erase(std::make_pair<QString, QString>(type_name, name));
        ComponentSyncState* old = GetComponent(type_name, name);
        if (old)
            return old;
        ComponentSyncState newstate;
        newstate.type_name_ = type_name;
        newstate.name_ = name;
        components_.push_back(newstate);
        return &components_[components_.size()-1];
    }
    
    ComponentSyncState* GetComponent(const QString& type_name, const QString& name)
    {
        for (int i = 0; i < (int)components_.size(); ++i)
        {
            if ((components_[i].type_name_ == type_name) && (components_[i].name_ == name))
                return &components_[i];
        }
        return 0;
    }
    
    void RemoveComponent(const QString& type_name, const QString& name)
    {
        dirty_components_.erase(std::make_pair<QString, QString>(type_name, name));
        removed_components_.erase(std::make_pair<QString, QString>(type_name, name));
        for (int i = 0; i < (int)components_.size(); ++i)
        {
            if ((components_[i].type_name_ == type_name) && (components_[i].name_ == name))
                components_.erase(components_.begin() + i);
        }
    }
    
    void OnComponentChanged(const QString& type_name, const QString& name)
    {
        dirty_components_.insert(std::make_pair<QString, QString>(type_name, name));
        removed_components_.erase(std::make_pair<QString, QString>(type_name, name));
    }
    
    void OnComponentRemoved(const QString& type_name, const QString& name)
    {
        removed_components_.insert(std::make_pair<QString, QString>(type_name, name));
        dirty_components_.erase(std::make_pair<QString, QString>(type_name, name));
    }
    
    void AckDirty(const QString& type_name, const QString& name)
    {
        dirty_components_.erase(std::make_pair<QString, QString>(type_name, name));
    }
    
    void AckRemove(const QString& type_name, const QString& name)
    {
        removed_components_.erase(std::make_pair<QString, QString>(type_name, name));
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
    
    void OnComponentChanged(entity_id_t id, const QString& type_name, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentChanged(type_name, name);
    }
    
    void OnComponentRemoved(entity_id_t id, const QString& type_name, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentRemoved(type_name, name);
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
