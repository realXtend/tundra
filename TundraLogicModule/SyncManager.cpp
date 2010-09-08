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

#include <QDomDocument>

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
    
    connect(scene.get(), SIGNAL( ComponentChanged(Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentChanged(Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentAdded(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( ComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ),
        this, SLOT( OnComponentRemoved(Scene::Entity*, Foundation::ComponentInterface*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( EntityCreated(Scene::Entity*, AttributeChange::Type) ),
        this, SLOT( OnEntityCreated(Scene::Entity*, AttributeChange::Type) ));
    connect(scene.get(), SIGNAL( EntityRemoved(Scene::Entity*, AttributeChange::Type) ),
        this, SLOT( OnEntityRemoved(Scene::Entity*, AttributeChange::Type) ));
}

void SyncManager::NewUserConnected(KristalliProtocol::UserConnection* user)
{
    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    // Sync all non-local entities
    for(Scene::SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        std::vector<MessageConnection*> connections;
        connections.push_back(user->connection);
        Scene::EntityPtr entity = *iter;
        if (!entity->IsLocal())
            EntitySync(connections, entity, true, true);
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

void SyncManager::EntitySync(const std::vector<MessageConnection*>& connections, Scene::EntityPtr entity, bool createEntity, bool allComponents)
{
    QDomDocument temp_doc;
    QDomElement entity_elem = temp_doc.createElement("entity");
    
    bool no_components = true;
    const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
    for(unsigned i = 0; i < components.size(); ++i)
    {
        if ((components[i]->IsSerializable()) && (components[i]->GetNetworkSyncEnabled()))
        {
            // If not sending all components, send only those who are dirtied
            if ((allComponents) || (components[i]->GetChange() == AttributeChange::Local))
            {
                components[i]->SerializeTo(temp_doc, entity_elem);
                no_components = false;
            }
        }
    }
    
    // If there were no components, and we're not sending a CreateEntity message, do not send anything
    if ((no_components) && (!createEntity))
        return;
    
    QByteArray bytes = temp_doc.toByteArray();
    
    if (createEntity)
    {
        MsgCreateEntity msg;
        msg.entityID = entity->GetId();
        msg.componentData.resize(bytes.size());
        memcpy(&msg.componentData[0], bytes.data(), bytes.size());
        
        for (unsigned i = 0; i < connections.size(); ++i)
            connections[i]->Send(msg);
    }
    else
    {
        MsgUpdateComponents msg;
        msg.entityID = entity->GetId();
        msg.componentData.resize(bytes.size());
        memcpy(&msg.componentData[0], bytes.data(), bytes.size());
        
        for (unsigned i = 0; i < connections.size(); ++i)
            connections[i]->Send(msg);
    }
}

void SyncManager::OnComponentChanged(Foundation::ComponentInterface* comp, AttributeChange::Type change)
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

void SyncManager::OnComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
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

void SyncManager::OnComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp, AttributeChange::Type change)
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
    
    std::vector<MessageConnection*> connections = GetConnectionsToSyncTo();
    
    // Process first the created entities
    std::set<entity_id_t>::const_iterator i = createdEntities_.begin();
    while (i != createdEntities_.end())
    {
        Scene::EntityPtr entity = scene->GetEntity(*i);
        if (entity)
        {
            EntitySync(connections, entity, true, false);
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
            EntitySync(connections, entity, false, false);
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

void SyncManager::HandleCreateEntity(MessageConnection* source, const MsgCreateEntity& msg)
{
}

void SyncManager::HandleRemoveEntity(MessageConnection* source, const MsgRemoveEntity& msg)
{
}

void SyncManager::HandleUpdateComponents(MessageConnection* source, const MsgUpdateComponents& msg)
{
}

void SyncManager::HandleRemoveComponents(MessageConnection* source, const MsgRemoveComponents& msg)
{
}

void SyncManager::HandleEntityIDCollision(MessageConnection* source, const MsgEntityIDCollision& msg)
{
}


}
