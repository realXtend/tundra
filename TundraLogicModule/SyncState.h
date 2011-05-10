// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TundraLogicModule_SyncState_h
#define incl_TundraLogicModule_SyncState_h

#include "IAttribute.h"
#include "UserConnection.h"
#include "Entity.h"
#include "TundraLogicModule.h"
#include "LoggingFunctions.h"

#include <QString>

#include <map>

namespace TundraLogic
{

/// State of component replication for a specific user
struct ComponentSyncState
{
    u32 typeId;
    QString name;
    // Note! These pointers are never dereferenced, and might be invalid. They are just for quick response to AttributeChanged signals.
    std::set<IAttribute*> dirty_static_attributes;
    std::set<QString> dirty_dynamic_attributes;
};

/// State of entity replication for a specific user
struct EntitySyncState
{
    /// Components that this client is already aware of
    std::vector<ComponentSyncState> components_;
    /// Created/modified components
    std::set<std::pair<uint, QString> > dirty_components_;
    /// Pending removed components (typenameid, name)
    std::set<std::pair<uint, QString> > removed_components_;
    
    ComponentSyncState* GetOrCreateComponent(u32 typeId, const QString &name)
    {
        // If we want to recreate the component and have a pending remove, remove the remove
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
        ComponentSyncState* old = GetComponent(typeId, name);
        if (old)
            return old;
        ComponentSyncState newstate;
        newstate.typeId = typeId;
        newstate.name = name;
        components_.push_back(newstate);
        return &components_[components_.size()-1];
    }
    
    ComponentSyncState* GetComponent(u32 typeId, const QString& name)
    {
        for(size_t i = 0; i < components_.size(); ++i)
        {
            if (components_[i].typeId == typeId && components_[i].name == name)
                return &components_[i];
        }
        return 0;
    }
    
    void RemoveComponent(u32 typeId, const QString& name)
    {
        dirty_components_.erase(std::make_pair<uint, QString>(typeId, name));
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
        for(size_t i = 0; i < components_.size(); ++i)
        {
            if (components_[i].typeId == typeId && components_[i].name == name)
                components_.erase(components_.begin() + i);
        }
    }
    
    void OnComponentAdded(u32 typeId, const QString& name)
    {
        dirty_components_.insert(std::make_pair<uint, QString>(typeId, name));
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
    }
    
    void OnAttributeChanged(u32 typeId, const QString& name, IAttribute* attribute)
    {
        dirty_components_.insert(std::make_pair<uint, QString>(typeId, name));
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
        // If client already has the component state, dirty the specific attribute
        if (attribute)
        {
            ComponentSyncState* compState = GetComponent(typeId, name);
            if (compState)
                compState->dirty_static_attributes.insert(attribute);
        }
    }
    
    void OnDynamicAttributeChanged(u32 typeId, const QString& name, const QString& attrName)
    {
        dirty_components_.insert(std::make_pair<uint, QString>(typeId, name));
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
        // If client already has the component state, dirty the specific attribute
        ComponentSyncState* compState = GetComponent(typeId, name);
        if (compState)
            compState->dirty_dynamic_attributes.insert(attrName);
    }
    
    void OnComponentRemoved(u32 typeId, const QString& name)
    {
        removed_components_.insert(std::make_pair<uint, QString>(typeId, name));
        dirty_components_.erase(std::make_pair<uint, QString>(typeId, name));
    }
    
    void AckDirty(u32 typeId, const QString& name)
    {
        dirty_components_.erase(std::make_pair<uint, QString>(typeId, name));
        ComponentSyncState* compState = GetComponent(typeId, name);
        if (compState)
        {
            compState->dirty_static_attributes.clear();
            compState->dirty_dynamic_attributes.clear();
        }
    }
    
    void AckRemove(u32 typeId, const QString& name)
    {
        removed_components_.erase(std::make_pair<uint, QString>(typeId, name));
    }
};

/// State of scene replication for a specific user
struct SceneSyncState : public ISyncState
{
    /// Entities that this client is already aware of
    std::map<entity_id_t, EntitySyncState> entities_;
    /// Created/modified entities
    std::set<entity_id_t> dirty_entities_;
    /// Pending removed entities
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
        if (removed_entities_.find(id) != removed_entities_.end())
        {
            // This is a problem because deletions are always processed after modifications, so a deletion for an old entity can actually occur after editing a new entity.
            LogWarning("Invoking buggy behavior: Update for ID " + QString::number(id).toStdString() + " to be sent, but that entity is also marked for deletion!");
        }
    }
    
    void OnEntityRemoved(entity_id_t id)
    {
        dirty_entities_.erase(id); // No need to update this entity to the network, since it will be deleted.
        removed_entities_.insert(id);
    }
    
    void OnAttributeChanged(entity_id_t id, u32 typeId, const QString& name, IAttribute* attribute)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnAttributeChanged(typeId, name, attribute);
    }
    
    void OnDynamicAttributeChanged(entity_id_t id, u32 typeId, const QString& name, const QString& attrName)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnDynamicAttributeChanged(typeId, name, attrName);
    }
    
    void OnComponentAdded(entity_id_t id, u32 typeId, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentAdded(typeId, name);
    }
    
    void OnComponentRemoved(entity_id_t id, u32 typeId, const QString& name)
    {
        OnEntityChanged(id);
        // If the entity does not exist in the user's syncstate yet, don't have to care
        // (full entitystate will be serialized once it's time)
        EntitySyncState* entitystate = GetEntity(id);
        if (!entitystate)
            return;
        entitystate->OnComponentRemoved(typeId, name);
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
