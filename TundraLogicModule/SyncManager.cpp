// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "KristalliProtocolModule.h"
#include "SyncManager.h"
#include "TundraLogicModule.h"
#include "MsgCreateEntity.h"
#include "MsgUpdateComponents.h"
#include "MsgRemoveComponents.h"
#include "MsgRemoveEntity.h"
#include "MsgEntityIDCollision.h"
#include "CoreStringUtils.h"

namespace TundraLogic
{

SyncManager::SyncManager(TundraLogicModule* owner, Foundation::Framework* fw) :
    owner_(owner),
    framework_(fw)
{
}

SyncManager::~SyncManager()
{
}

void SyncManager::RegisterToScene(Scene::ScenePtr scene)
{
    createdEntities_.clear();
    dirtyEntities_.clear();
    removedComponents_.clear();
    removedEntities_.clear();
    
    connect(scene.get(), SIGNAL( ComponentChanged(IComponent*, AttributeChange::Type) ),
        this, SLOT( OnComponentChanged(IComponent*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type) ),
        this, SLOT( OnComponentAdded(Scene::Entity*, IComponent*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type) ),
        this, SLOT( OnComponentRemoved(Scene::Entity*, IComponent*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( EntityCreated(Scene::Entity*, AttributeChange::Type) ),
        this, SLOT( OnEntityCreated(Scene::Entity*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( EntityRemoved(Scene::Entity*, AttributeChange::Type) ),
        this, SLOT( OnEntityRemoved(Scene::Entity*, AttributeChange::Type) ));
}

void SyncManager::NewUserConnected(KristalliProtocol::UserConnection* user)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    // Sync all non-local entities
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        std::vector<MessageConnection*> connections;
        connections.push_back(user->connection);
        Scene::EntityPtr entity = *iter;
        if (!entity->IsLocal())
            SerializeAndSendComponents(connections, entity, true, true);
    }
}

std::vector<MessageConnection*> SyncManager::GetConnectionsToSyncTo()
{
    std::vector<MessageConnection*> connections;
    if (owner_->IsServer())
    {
        KristalliProtocol::UserConnectionList& userConnections = owner_->ServerGetUserConnections();
        for (KristalliProtocol::UserConnectionList::iterator i = userConnections.begin(); i != userConnections.end(); ++i)
            connections.push_back(i->connection);
    }
    else
    {
        if (owner_->ClientGetConnection())
            connections.push_back(owner_->ClientGetConnection());
    }
    
    return connections;
}

void SyncManager::SerializeAndSendComponents(const std::vector<MessageConnection*>& connections, Scene::EntityPtr entity, bool createEntity, bool allComponents)
{
    if (!entity)
        return;
    
    const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
    bool no_components = true;
    
    // If there were no components, and we're not sending a CreateEntity message, do not send anything
    if ((no_components) && (!createEntity))
        return;
    
    if (createEntity)
    {
        MsgCreateEntity msg;
        msg.entityID = entity->GetId();
        for(uint i = 0; i < components.size(); ++i)
        {
            IComponent* component = components[i].get();
            
            if ((component->IsSerializable()) && (component->GetNetworkSyncEnabled()))
            {
                // If not sending all components, send only those who are dirtied
                if ((allComponents) || (component->GetChange() == AttributeChange::Local))
                {
                    MsgCreateEntity::S_components newComponent;
                    newComponent.componentTypeName = StringToBuffer(component->TypeName().toStdString());
                    newComponent.componentName = StringToBuffer(component->Name().toStdString());
                    newComponent.componentData.resize(64 * 1024);
                    DataSerializer dest((char*)&newComponent.componentData[0], newComponent.componentData.size());
                    component->SerializeToBinary(dest);
                    newComponent.componentData.resize(dest.BytesFilled());
                    msg.components.push_back(newComponent);
                }
            }
        }
        for (unsigned i = 0; i < connections.size(); ++i)
            connections[i]->Send(msg);
    }
    else
    {
        MsgUpdateComponents msg;
        msg.entityID = entity->GetId();
        for(uint i = 0; i < components.size(); ++i)
        {
            IComponent* component = components[i].get();
            
            if ((component->IsSerializable()) && (component->GetNetworkSyncEnabled()))
            {
                // If not sending all components, send only those who are dirtied
                if ((allComponents) || (component->GetChange() == AttributeChange::Local))
                {
                    MsgUpdateComponents::S_components newComponent;
                    newComponent.componentTypeName = StringToBuffer(component->TypeName().toStdString());
                    newComponent.componentName = StringToBuffer(component->Name().toStdString());
                    newComponent.componentData.resize(64 * 1024);
                    DataSerializer dest((char*)&newComponent.componentData[0], newComponent.componentData.size());
                    component->SerializeToBinary(dest);
                    newComponent.componentData.resize(dest.BytesFilled());
                    msg.components.push_back(newComponent);
                }
            }
        }
        // If there are no components to update, do not send
        if (msg.components.empty())
            return;
        
        for (unsigned i = 0; i < connections.size(); ++i)
            connections[i]->Send(msg);
    }
}

void SyncManager::OnComponentChanged(IComponent* comp, AttributeChange::Type change)
{
    if ((change != AttributeChange::Local) || (!comp->GetNetworkSyncEnabled()))
        return;
    Scene::Entity* entity = comp->GetParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    // If entity is already on created list, let creation sync handle the component
    if (createdEntities_.find(entity->GetId()) != createdEntities_.end())
        return;
    dirtyEntities_.insert(entity->GetId());
}

void SyncManager::OnComponentAdded(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if ((change != AttributeChange::Local) || (!comp->GetNetworkSyncEnabled()))
        return;
    if (entity->IsLocal())
        return;
    
    // If component was on the removed list, but was re-added, remove from removed list
    std::vector<RemovedComponent>& removed = removedComponents_[entity->GetId()];
    for (unsigned i = 0; i < removed.size(); ++i)
    {
        if ((removed[i].typename_ == comp->TypeName()) && (removed[i].name_ == comp->Name()))
            removed.erase(removed.begin() + i);
    }
    
    // If entity is already on created list, let creation sync handle the component
    if (createdEntities_.find(entity->GetId()) != createdEntities_.end())
        return;
    dirtyEntities_.insert(entity->GetId());
}

void SyncManager::OnComponentRemoved(Scene::Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if ((change != AttributeChange::Local) || (!comp->GetNetworkSyncEnabled()))
        return;
    if (entity->IsLocal())
        return;
    
    // If entity is on the created list, creation sync will send the proper set of components at the end of the frame, and we don't have to bother
    if (createdEntities_.find(entity->GetId()) != createdEntities_.end())
        return;
    // If entity is on the removed list, we don't have to bother
    if (removedEntities_.find(entity->GetId()) != removedEntities_.end())
        return;
    
    RemovedComponent removed;
    removed.typename_ = comp->TypeName();
    removed.name_ = comp->Name();
    
    removedComponents_[entity->GetId()].push_back(removed);
}

void SyncManager::OnEntityCreated(Scene::Entity* entity, AttributeChange::Type change)
{
    if ((change != AttributeChange::Local) || (entity->IsLocal()))
        return;
    
    // Put on the created list
    createdEntities_.insert(entity->GetId());
    
    // Remove from all other lists, if is there
    removedEntities_.erase(entity->GetId());
    dirtyEntities_.erase(entity->GetId());
    removedComponents_.erase(entity->GetId());
}

void SyncManager::OnEntityRemoved(Scene::Entity* entity, AttributeChange::Type change)
{
    if (change != AttributeChange::Local)
        return;
    if (entity->IsLocal())
        return;
    
    createdEntities_.erase(entity->GetId());
    dirtyEntities_.erase(entity->GetId());
    removedComponents_.erase(entity->GetId()); // Individual component removals do not matter at this point
    removedEntities_.insert(entity->GetId());
}

void SyncManager::Update()
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    std::vector<MessageConnection*> connections = GetConnectionsToSyncTo();
    
    // Process first the created entities
    std::set<entity_id_t>::const_iterator i = createdEntities_.begin();
    while (i != createdEntities_.end())
    {
        Scene::EntityPtr entity = scene->GetEntity(*i);
        if (entity)
        {
            SerializeAndSendComponents(connections, entity, true, false);
            entity->ResetChange();
        }
        ++i;
    }
    createdEntities_.clear();
    
    // Then the entity modifications
    i = dirtyEntities_.begin();
    while (i != dirtyEntities_.end())
    {
        Scene::EntityPtr entity = scene->GetEntity(*i);
        if (entity)
        {
            SerializeAndSendComponents(connections, entity, false, false);
            entity->ResetChange();
        }
        ++i;
    }
    dirtyEntities_.clear();
    
    // Then removed components
    std::map<entity_id_t, std::vector<RemovedComponent> >::const_iterator j = removedComponents_.begin();
    while (j != removedComponents_.end())
    {
        const std::vector<RemovedComponent>& removed = j->second;
        if (removed.size())
        {
            MsgRemoveComponents msg;
            msg.entityID = j->first;
            msg.components.resize(removed.size());
            for (unsigned k = 0; k < removed.size(); ++k)
            {
                msg.components[k].componentTypeName = StringToBuffer(removed[k].typename_.toStdString());
                msg.components[k].componentName = StringToBuffer(removed[k].name_.toStdString());
            }
            
            for (unsigned l = 0; l < connections.size(); ++l)
                connections[l]->Send(msg);
        }
        ++j;
    }
    removedComponents_.clear();
    
    // And finally removed entities
    i = removedEntities_.begin();
    while (i != removedEntities_.end())
    {
        MsgRemoveEntity msg;
        msg.entityID = *i;
        for (unsigned j = 0; j < connections.size(); ++j)
            connections[j]->Send(msg);
        ++i;
    }
    removedEntities_.clear();
}

bool SyncManager::ValidateAction(MessageConnection* source, unsigned messageID, entity_id_t entityID)
{
    if (entityID & Scene::LocalEntity)
    {
        TundraLogicModule::LogWarning("Received an entity sync message for a local entity. Disregarding.");
        return false;
    }
    
    // For now, always trust scene actions from server
    if (!owner_->IsServer())
        return true;
    
    // And for now, always also trust scene actions from clients, if they are known and authenticated
    KristalliProtocol::UserConnection* user = owner_->GetUserConnection(source);
    if ((!user) || (!user->authenticated))
        return false;
    
    return true;
}

void SyncManager::HandleCreateEntity(MessageConnection* source, const MsgCreateEntity& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.MessageID(), entityID))
        return;
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is Network. For server, the change type is Local, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Local : AttributeChange::Network;
    
    // Check for ID collision
    if (isServer)
    {
        if (scene->GetEntity(entityID))
        {
            entity_id_t newEntityID = scene->GetNextFreeId();
            // Send information to the creator that the ID was taken. The reserved ID will never get replicated to others
            MsgEntityIDCollision collisionMsg;
            collisionMsg.oldEntityID = entityID;
            collisionMsg.newEntityID = newEntityID;
            source->Send(collisionMsg);
            entityID = newEntityID;
        }
    }
    else
    {
        // If a client gets a entity that already exists, destroy it forcibly
        if (scene->GetEntity(entityID))
        {
            TundraLogicModule::LogDebug("Received entity creation from server for entity ID " + ToString<int>(entityID) + " that already exists. Removing the old entity.");
            scene->RemoveEntity(entityID, change);
        }
    }
    
    Scene::EntityPtr entity = scene->CreateEntity(entityID);
    if (!entity)
    {
        TundraLogicModule::LogWarning("Scene refused to create entity " + ToString<int>(entityID));
        return;
    }
    
    // Read the components
    for (uint i = 0; i < msg.components.size(); ++i)
    {
        QString type_name = QString::fromStdString(BufferToString(msg.components[i].componentTypeName));
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
        if (new_comp)
        {
            if (msg.components[i].componentData.size())
            {
                DataDeserializer source((const char*)&msg.components[i].componentData[0], msg.components[i].componentData.size());
                try
                {
                    new_comp->DeserializeFromBinary(source, change);
                }
                catch (...)
                {
                    TundraLogicModule::LogError("Error while deserializing component " + type_name.toStdString());
                }
            }
        }
        else
            TundraLogicModule::LogWarning("Could not create component " + type_name.toStdString());
    }
    
    // Then emit the entity/componentchanges
    scene->EmitEntityCreated(entity, change);
    // Kind of a "hack", call OnChanged to the components only after all components have been loaded
    // This allows to resolve component references to the same entity (for example to the Placeable) at this point
    const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
    for(uint i = 0; i < components.size(); ++i)
        components[i]->ComponentChanged(change);
    // Finally, if changetype is Network, reset it so that it won't show "dirty" status which may be confusing
    if (change == AttributeChange::Network)
        entity->ResetChange();
}

void SyncManager::HandleRemoveEntity(MessageConnection* source, const MsgRemoveEntity& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.MessageID(), entityID))
        return;
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is Network. For server, the change type is Local, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Local : AttributeChange::Network;
    
    scene->RemoveEntity(entityID, change);
}

void SyncManager::HandleUpdateComponents(MessageConnection* source, const MsgUpdateComponents& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.MessageID(), entityID))
        return;
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is Network. For server, the change type is Local, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Local : AttributeChange::Network;
    
    // See if we can find the entity. If not, create it (!!!)
    // \todo When we move to UDP, we will need to implement a smarter protocol that guarantees correct latest state
    // With TCP everything stays automatically in order nicely
    Scene::EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        TundraLogicModule::LogWarning("Entity " + ToString<int>(entityID) + " not found for UpdateComponents message, creating it now");
        entity = scene->CreateEntity(entityID);
        if (!entity)
        {
            TundraLogicModule::LogWarning("Scene refused to create entity " + ToString<int>(entityID));
            return;
        }
    }
    
    // Read the components
    for (uint i = 0; i < msg.components.size(); ++i)
    {
        QString type_name = QString::fromStdString(BufferToString(msg.components[i].componentTypeName));
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr new_comp = entity->GetOrCreateComponent(type_name, name);
        if (new_comp)
        {
            if (msg.components[i].componentData.size())
            {
                DataDeserializer source((const char*)&msg.components[i].componentData[0], msg.components[i].componentData.size());
                try
                {
                    new_comp->DeserializeFromBinary(source, change);
                }
                catch (...)
                {
                    TundraLogicModule::LogError("Error while deserializing component " + type_name.toStdString());
                }
            }
        }
        else
            TundraLogicModule::LogWarning("Could not create component " + type_name.toStdString());
    }
    
    // Then emit the componentchanges
    // Kind of a "hack", call OnChanged to the components only after all components have been loaded
    // This allows to resolve component references to the same entity (for example to the Placeable) at this point
    const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
    for(uint i = 0; i < components.size(); ++i)
        components[i]->ComponentChanged(change);
    // Finally, if changetype is Network, reset it so that it won't show "dirty" status which may be confusing
    if (change == AttributeChange::Network)
        entity->ResetChange();
}

void SyncManager::HandleRemoveComponents(MessageConnection* source, const MsgRemoveComponents& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.MessageID(), entityID))
        return;
        
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is Network. For server, the change type is Local, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Local : AttributeChange::Network;
    
    Scene::EntityPtr entity = scene->GetEntity(msg.entityID);
    if (!entity)
        return;
    
    for (unsigned i = 0; i < msg.components.size(); ++i)
    {
        QString componentName = QString::fromStdString(BufferToString(msg.components[i].componentName));
        QString componentTypeName = QString::fromStdString(BufferToString(msg.components[i].componentTypeName));
        
        ComponentPtr comp = entity->GetComponent(componentTypeName, componentName);
        if (comp)
        {
            entity->RemoveComponent(comp, change);
            comp.reset();
        }
    }
}

void SyncManager::HandleEntityIDCollision(MessageConnection* source, const MsgEntityIDCollision& msg)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (!scene)
        return;
    
    if (owner_->IsServer())
    {
        TundraLogicModule::LogWarning("Received a EntityIDCollision from a client, disregarding.");
        return;
    }
    
    TundraLogicModule::LogDebug("An entity ID collision occurred. Entity " + ToString<int>(msg.oldEntityID) + " became " + ToString<int>(msg.newEntityID));
    scene->ChangeEntityId(msg.oldEntityID, msg.newEntityID);
}

}
