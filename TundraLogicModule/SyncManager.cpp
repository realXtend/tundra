// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ComponentManager.h"
#include "CoreStringUtils.h"
#include "DebugOperatorNew.h"
#include "KristalliProtocolModule.h"
#include "SyncManager.h"
#include "SceneManager.h"
#include "Entity.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "TundraMessages.h"
#include "MsgCreateEntity.h"
#include "MsgCreateComponents.h"
#include "MsgUpdateComponents.h"
#include "MsgRemoveComponents.h"
#include "MsgRemoveEntity.h"
#include "MsgEntityIDCollision.h"
#include "MsgEntityAction.h"
#include "MsgAssetDeleted.h"
#include "MsgAssetDiscovery.h"
#include "EC_DynamicComponent.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "LoggingFunctions.h"

#include "SceneAPI.h"

#include <kNet.h>

#include <cstring>

#include "MemoryLeakCheck.h"

using namespace kNet;

// This define controls whether to echo scene changes back to the sending client. For some applications (ie. synced UI widgets)
// it is nice to have off, but there might be possible determinism issues if many clients are updating the same attribute at the same time
// Note: changes must definitely be sent also to sender if server can change the value sent by the client. Also when/if changes can be 
// rejected, an overriding update should be sent to the sending client
// #define ECHO_CHANGES_TO_SENDER

// This variable is used for the sender echo & interpolation stop check
kNet::MessageConnection* currentSender = 0;

namespace TundraLogic
{

SyncManager::SyncManager(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    update_period_(1.0f / 30.0f),
    update_acc_(0.0)
{
    // Connect to asset uploads to be able to post discovery messages
    connect(framework_->Asset(), SIGNAL(AssetUploaded(const QString &)), this, SLOT(OnAssetUploaded(const QString &)));
    // Connect to asset deletes to be able to post delete messages
    connect(framework_->Asset(), SIGNAL(AssetDeleted(const QString &)), this, SLOT(OnAssetDeleted(const QString &)));
}

SyncManager::~SyncManager()
{
}

void SyncManager::SetUpdatePeriod(float period)
{
    // Allow max 100fps
    if (period < 0.01f)
        period = 0.01f;
    update_period_ = period;
}

void SyncManager::RegisterToScene(ScenePtr scene)
{
    // Disconnect from previous scene if not expired
    ScenePtr previous = scene_.lock();
    if (previous)
    {
        disconnect(this);
        server_syncstate_.Clear();
    }
    
    scene_.reset();
    
    if (!scene)
    {
        ::LogError("Null scene, cannot replicate");
        return;
    }
    
    scene_ = scene;
    SceneManager* sceneptr = scene.get();
    
    connect(sceneptr, SIGNAL( AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( AttributeAdded(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( AttributeRemoved(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( ComponentAdded(Entity*, IComponent*, AttributeChange::Type) ),
        SLOT( OnComponentAdded(Entity*, IComponent*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( ComponentRemoved(Entity*, IComponent*, AttributeChange::Type) ),
        SLOT( OnComponentRemoved(Entity*, IComponent*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( EntityCreated(Entity*, AttributeChange::Type) ),
        SLOT( OnEntityCreated(Entity*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( EntityRemoved(Entity*, AttributeChange::Type) ),
        SLOT( OnEntityRemoved(Entity*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionType) ),
        SLOT( OnActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecutionType)));
}

///\todo EventManager regression. -jj.
/*
void SyncManager::HandleKristalliEvent(event_id_t event_id, IEventData* data)
{
    if (event_id == KristalliProtocol::Events::NETMESSAGE_IN)
    {
        KristalliProtocol::Events::KristalliNetMessageIn* eventData = checked_static_cast<KristalliProtocol::Events::KristalliNetMessageIn*>(data);
        HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
    }
}
*/
void SyncManager::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
{
    switch (id)
    {
    case cCreateEntityMessage:
        {
            MsgCreateEntity msg(data, numBytes);
            HandleCreateEntity(source, msg);
        }
        break;
    case cRemoveEntityMessage:
        {
            MsgRemoveEntity msg(data, numBytes);
            HandleRemoveEntity(source, msg);
        }
        break;
    case cCreateComponentsMessage:
        {
            MsgCreateComponents msg(data, numBytes);
            HandleCreateComponents(source, msg);
        }
        break;
    case cUpdateComponentsMessage:
        {
            MsgUpdateComponents msg(data, numBytes);
            HandleUpdateComponents(source, msg);
        }
        break;
    case cRemoveComponentsMessage:
        {
            MsgRemoveComponents msg(data, numBytes);
            HandleRemoveComponents(source, msg);
        }
        break;
    case cEntityIDCollisionMessage:
        {
            MsgEntityIDCollision msg(data, numBytes);
            HandleEntityIDCollision(source, msg);
        }
        break;
    case cEntityActionMessage:
        {
            MsgEntityAction msg(data, numBytes);
            HandleEntityAction(source, msg);
        }
        break;
    case cAssetDiscoveryMessage:
        {
            MsgAssetDiscovery msg(data, numBytes);
            HandleAssetDiscovery(source, msg);
        }
        break;
    case cAssetDeletedMessage:
        {
            MsgAssetDeleted msg(data, numBytes);
            HandleAssetDeleted(source, msg);
        }
        break;
    }
    
    currentSender = 0;
}

void SyncManager::NewUserConnected(UserConnection* user)
{
    PROFILE(SyncManager_NewUserConnected);

    ScenePtr scene = scene_.lock();
    if (!scene)
        return;
    
    // Connect to actions sent to specifically to this user
    connect(user, SIGNAL(ActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)), this, SLOT(OnUserActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)));
    
    // If user does not have replication state, create it, then mark all non-local entities dirty
    // so we will send them during the coming updates
    if (!user->syncState)
        user->syncState = boost::shared_ptr<ISyncState>(new SceneSyncState());
    
    SceneSyncState* state = checked_static_cast<SceneSyncState*>(user->syncState.get());
    
    for(SceneManager::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        entity_id_t id = entity->GetId();
        // If we cross over to local entities (ID range 0x80000000 - 0xffffffff), break
        if (id & LocalEntity)
            break;
        state->OnEntityChanged(id);
    }
}

void SyncManager::OnAttributeChanged(IComponent* comp, IAttribute* attr, AttributeChange::Type change)
{
    bool isServer = owner_->IsServer();
    
    // Client: Check for stopping interpolation, if we change a currently interpolating variable ourselves
    if (!isServer)
    {
        ScenePtr scene = scene_.lock();
        if ((scene) && (!scene->IsInterpolating()) && (!currentSender))
        {
            if ((attr->HasMetadata()) && (attr->GetMetadata()->interpolation == AttributeMetadata::Interpolate))
                // Note: it does not matter if the attribute was not actually interpolating
                scene->EndAttributeInterpolation(attr);
        }
    }
    
    if ((change != AttributeChange::Replicate) || (!comp->GetNetworkSyncEnabled()))
        return;
    Entity* entity = comp->GetParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    bool dynamic = comp->HasDynamicStructure();
    
    if (isServer)
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
#ifndef ECHO_CHANGES_TO_SENDER
            if ((*i)->connection == currentSender)
                continue;
#endif
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
            {
                if (!dynamic)
                    state->OnAttributeChanged(entity->GetId(), comp->TypeNameHash(), comp->Name(), attr);
                else
                    // Note: this may be an add, change or remove. We inspect closer when it's time to send the update message.
                    state->OnDynamicAttributeChanged(entity->GetId(), comp->TypeNameHash(), comp->Name(), QString::fromStdString(attr->GetNameString()));
            }
        }
    }
    else
    {
        SceneSyncState* state = &server_syncstate_;
        if (!dynamic)
            state->OnAttributeChanged(entity->GetId(), comp->TypeNameHash(), comp->Name(), attr);
        else
            state->OnDynamicAttributeChanged(entity->GetId(), comp->TypeNameHash(), comp->Name(), QString::fromStdString(attr->GetNameString()));
    }
    
    // This attribute changing might in turn cause other attributes to change on the server, and these must be echoed to all, so reset sender now
    currentSender = 0;
}

void SyncManager::OnComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if ((change != AttributeChange::Replicate) || (!comp->GetNetworkSyncEnabled()))
        return;
    if (entity->IsLocal())
        return;
    
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
                state->OnComponentAdded(entity->GetId(), comp->TypeNameHash(), comp->Name());
        }
    }
    else
    {
        SceneSyncState* state = &server_syncstate_;
        state->OnComponentAdded(entity->GetId(), comp->TypeNameHash(), comp->Name());
    }
}

void SyncManager::OnComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    if ((change != AttributeChange::Replicate) || (!comp->GetNetworkSyncEnabled()))
        return;
    if (entity->IsLocal())
        return;
    
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
                state->OnComponentRemoved(entity->GetId(), comp->TypeNameHash(), comp->Name());
        }
    }
    else
    {
        SceneSyncState* state = &server_syncstate_;
        state->OnComponentRemoved(entity->GetId(), comp->TypeNameHash(), comp->Name());
    }
}

void SyncManager::OnEntityCreated(Entity* entity, AttributeChange::Type change)
{
    if ((change != AttributeChange::Replicate) || (entity->IsLocal()))
        return;

    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
            {
                if (state->removed_entities_.find(entity->GetId()) != state->removed_entities_.end())
                {
                    LogWarning("An entity with ID " + QString::number(entity->GetId()).toStdString() + " is queued to be deleted, but a new entity \"" + 
                        entity->GetName().toStdString() + "\" is to be added to the scene!");
                }
                state->OnEntityChanged(entity->GetId());
            }
        }
    }
    else
    {
        SceneSyncState* state = &server_syncstate_;
        state->OnEntityChanged(entity->GetId());
    }
}

void SyncManager::OnEntityRemoved(Entity* entity, AttributeChange::Type change)
{
    if (change != AttributeChange::Replicate)
        return;
    if (entity->IsLocal())
        return;
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
                state->OnEntityRemoved(entity->GetId());
        }
    }
    else
    {
        SceneSyncState* state = &server_syncstate_;
        state->OnEntityRemoved(entity->GetId());
    }
}

void SyncManager::OnActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecutionType type)
{
    //SceneManager* scene = scene_.lock().get();
    //assert(scene);

    // If we are the server and the local script on this machine has requested a script to be executed on the server, it
    // means we just execute the action locally here, without sending to network.
    bool isServer = owner_->IsServer();
    if (isServer && (type & EntityAction::Server) != 0)
    {
        //LogInfo("EntityAction " + action.toStdString() + " type Server on server.");
        entity->Exec(EntityAction::Local, action, params);
    }

    // Craft EntityAction message.
    MsgEntityAction msg;
    msg.entityId = entity->GetId();
    // msg.executionType will be set below depending are we server or client.
    msg.name = StringToBuffer(action.toStdString());
    for(int i = 0; i < params.size(); ++i)
    {
        MsgEntityAction::S_parameters p = { StringToBuffer(params[i].toStdString()) };
        msg.parameters.push_back(p);
    }

    if (!isServer && ((type & EntityAction::Server) != 0 || (type & EntityAction::Peers) != 0) && owner_->GetClient()->GetConnection())
    {
        // send without Local flag
        //LogInfo("Tundra client sending EntityAction " + action.toStdString() + " type " + ToString(type));
        msg.executionType = (u8)(type & ~EntityAction::Local);
        owner_->GetClient()->GetConnection()->Send(msg);
    }

    if (isServer && (type & EntityAction::Peers) != 0)
    {
        msg.executionType = (u8)EntityAction::Local; // Propagate as local actions.
        foreach(UserConnection* c, owner_->GetKristalliModule()->GetUserConnections())
        {
            if (c->properties["authenticated"] == "true" && c->connection)
            {
                //LogInfo("peer " + action.toStdString());
                c->connection->Send(msg);
            }
        }
    }
}

void SyncManager::OnUserActionTriggered(UserConnection* user, Entity *entity, const QString &action, const QStringList &params)
{
    bool isServer = owner_->IsServer();
    if (!isServer)
        return; // Should never happen
    if ((!entity) || (!user))
        return;
    if (user->properties["authenticated"] != "true")
        return; // Not yet authenticated, do not receive actions
    
    // Craft EntityAction message.
    MsgEntityAction msg;
    msg.entityId = entity->GetId();
    msg.name = StringToBuffer(action.toStdString());
    msg.executionType = (u8)EntityAction::Local; // Propagate as local action.
    for(int i = 0; i < params.size(); ++i)
    {
        MsgEntityAction::S_parameters p = { StringToBuffer(params[i].toStdString()) };
        msg.parameters.push_back(p);
    }
    user->connection->Send(msg);
}

void SyncManager::Update(f64 frametime)
{
    PROFILE(SyncManager_Update);
    
    update_acc_ += (float)frametime;
    if (update_acc_ < update_period_)
        return;
    // If multiple updates passed, update still just once
    while(update_acc_ >= update_period_)
        update_acc_ -= update_period_;
    
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;
    
    if (owner_->IsServer())
    {
        // If we are server, process all users
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            SceneSyncState* state = checked_static_cast<SceneSyncState*>((*i)->syncState.get());
            if (state)
                ProcessSyncState((*i)->connection, state);
        }
    }
    else
    {
        // If we are client, process just the server sync state
        kNet::MessageConnection* connection = owner_->GetKristalliModule()->GetMessageConnection();
        if (connection)
            ProcessSyncState(connection, &server_syncstate_);
    }
}

void SyncManager::ProcessSyncState(kNet::MessageConnection* destination, SceneSyncState* state)
{
    PROFILE(SyncManager_ProcessSyncState);
    
    ScenePtr scene = scene_.lock();
    
    int num_messages_sent = 0;
    
    /// \todo Always sends everything that is dirty/removed. No priorization or limiting of sent data size yet.
    
    // Process dirty entities (added/updated/removed components)
    std::set<entity_id_t> dirty = state->dirty_entities_;
    for(std::set<entity_id_t>::iterator i = dirty.begin(); i != dirty.end(); ++i)
    {
        EntityPtr entity = scene->GetEntity(*i);
        if (!entity)
            continue;

        if (state->removed_entities_.find(*i) != state->removed_entities_.end())
        {
            LogWarning("Potentially buggy behavior! Sending entity update for ID " + QString::number(*i).toStdString() + ", name: " + entity->GetName().toStdString()
                + " but the entity with that ID is queued for deletion later!");
        }

        const Entity::ComponentVector &components =  entity->Components();
        EntitySyncState* entitystate = state->GetEntity(*i);
        // No record in entitystate -> newly created entity, send full state
        if (!entitystate)
        {
            entitystate = state->GetOrCreateEntity(*i);
            MsgCreateEntity msg;
            msg.entityID = entity->GetId();
            for(uint j = 0; j < components.size(); ++j)
            {
                ComponentPtr component = components[j];
                
                if (component->GetNetworkSyncEnabled())
                {
                    // Create componentstate so we can start tracking individual attributes
                    ComponentSyncState* componentstate = entitystate->GetOrCreateComponent(component->TypeNameHash(), component->Name());
                    UNREFERENCED_PARAM(componentstate);
                    MsgCreateEntity::S_components newComponent;
                    newComponent.componentTypeHash = component->TypeNameHash();
                    newComponent.componentName = StringToBuffer(component->Name().toStdString());
                    newComponent.componentData.resize(64 * 1024);
                    DataSerializer dest((char*)&newComponent.componentData[0], newComponent.componentData.size());
                    component->SerializeToBinary(dest);
                    newComponent.componentData.resize(dest.BytesFilled());
                    msg.components.push_back(newComponent);
                }
                
                entitystate->AckDirty(component->TypeNameHash(), component->Name());
            }
            destination->Send(msg);
            ++num_messages_sent;
        }
        else
        {
            // Existing entitystate, check created & modified components
            /// \todo Renaming an existing component, that already has been replicated to client, leads to duplication.
            /// So it's not currently supported sensibly.
            {
                std::set<std::pair<uint, QString> > dirtycomps = entitystate->dirty_components_;
                MsgCreateComponents createMsg;
                createMsg.entityID = entity->GetId();
                MsgUpdateComponents updateMsg;
                updateMsg.entityID = entity->GetId();
                
                for(std::set<std::pair<uint, QString> >::iterator j = dirtycomps.begin(); j != dirtycomps.end(); ++j)
                {
                    ComponentPtr component = entity->GetComponent(j->first, j->second);
                    if (component && component->GetNetworkSyncEnabled())
                    {
                        ComponentSyncState* componentstate = entitystate->GetComponent(component->TypeNameHash(), component->Name());
                        // New component
                        if (!componentstate)
                        {
                            // Create componentstate so we can start tracking individual attributes
                            componentstate = entitystate->GetOrCreateComponent(component->TypeNameHash(), component->Name());
                            
                            MsgCreateComponents::S_components newComponent;
                            newComponent.componentTypeHash = component->TypeNameHash();
                            newComponent.componentName = StringToBuffer(component->Name().toStdString());
                            newComponent.componentData.resize(64 * 1024);
                            DataSerializer dest((char*)&newComponent.componentData[0], newComponent.componentData.size());
                            component->SerializeToBinary(dest);
                            newComponent.componentData.resize(dest.BytesFilled());
                            createMsg.components.push_back(newComponent);
                        }
                        else
                        {
                            // Existing data, serialize changed attributes only
                            // Static structure component
                            if (!component->HasDynamicStructure())
                            {
                                MsgUpdateComponents::S_components updComponent;
                                updComponent.componentTypeHash = component->TypeNameHash();
                                updComponent.componentName = StringToBuffer(component->Name().toStdString());
                                updComponent.componentData.resize(64 * 1024);
                                DataSerializer dest((char*)&updComponent.componentData[0], updComponent.componentData.size());
                                bool has_changes = false;
                                // Otherwise, we assume the attribute structure is static in the component, and we check which attributes are in the dirty list
                                const AttributeVector& attributes = component->GetAttributes();
                                for(uint k = 0; k < attributes.size(); k++)
                                {
                                    if (componentstate->dirty_static_attributes_.find(attributes[k]) != componentstate->dirty_static_attributes_.end())
                                    {
                                        dest.Add<bit>(1);
                                        attributes[k]->ToBinary(dest);
                                        has_changes = true;
                                    }
                                    else
                                        dest.Add<bit>(0);
                                }
                                if (has_changes)
                                {
                                    updComponent.componentData.resize(dest.BytesFilled());
                                    updateMsg.components.push_back(updComponent);
                                }
                            }
                            // Existing data, dynamically structured component
                            else
                            {
                                MsgUpdateComponents::S_dynamiccomponents updComponent;
                                updComponent.componentTypeHash = component->TypeNameHash();
                                updComponent.componentName = StringToBuffer(component->Name().toStdString());
                                bool has_changes = false;
                                const std::set<QString>& dirtyAttrs = componentstate->dirty_dynamic_attributes_;
                                std::set<QString>::const_iterator k = dirtyAttrs.begin();
                                while(k != dirtyAttrs.end())
                                {
                                    has_changes = true;
                                    MsgUpdateComponents::S_dynamiccomponents::S_attributes updAttribute;
                                    // Check if the attribute is changed or removed
                                    IAttribute* attribute = component->GetAttribute(*k);
                                    if (attribute)
                                    {
                                        updAttribute.attributeName = StringToBuffer((*k).toStdString());
                                        updAttribute.attributeType = StringToBuffer(attribute->TypeName());
                                        updAttribute.attributeData.resize(64 * 1024);
                                        DataSerializer dest((char*)&updAttribute.attributeData[0], updAttribute.attributeData.size());
                                        attribute->ToBinary(dest);
                                        updAttribute.attributeData.resize(dest.BytesFilled());
                                    }
                                    else
                                    {
                                        // Removed attribute: empty typename & data
                                        updAttribute.attributeName = StringToBuffer((*k).toStdString());
                                    }
                                    
                                    updComponent.attributes.push_back(updAttribute);
                                    ++k;
                                }
                                if (has_changes)
                                    updateMsg.dynamiccomponents.push_back(updComponent);
                            }
                        }
                    }
                    entitystate->AckDirty(j->first, j->second);
                }
                
                // Send message(s) only if there were components
                if (createMsg.components.size())
                {
                    destination->Send(createMsg);
                    ++num_messages_sent;
                }
                if (updateMsg.components.size() || updateMsg.dynamiccomponents.size())
                {
                    destination->Send(updateMsg);
                    ++num_messages_sent;
                }
            }
            
            // Check removed components
            {
                std::set<std::pair<uint, QString> > removedcomps = entitystate->removed_components_;
                MsgRemoveComponents removeMsg;
                removeMsg.entityID = entity->GetId();
                
                for(std::set<std::pair<uint, QString> >::iterator j = removedcomps.begin(); j != removedcomps.end(); ++j)
                {
                    MsgRemoveComponents::S_components remComponent;
                    remComponent.componentTypeHash = j->first;
                    remComponent.componentName = StringToBuffer(j->second.toStdString());
                    removeMsg.components.push_back(remComponent);
                    
                    entitystate->RemoveComponent(j->first, j->second);
                    entitystate->AckRemove(j->first, j->second);
                }
                
                if (removeMsg.components.size())
                {
                    destination->Send(removeMsg);
                    ++num_messages_sent;
                }
            }
        }
        
        state->AckDirty(*i);
        
    }

    // Process removed entities
    std::set<entity_id_t> removed = state->removed_entities_;
    for(std::set<entity_id_t>::iterator i = removed.begin(); i != removed.end(); ++i)
    {
        MsgRemoveEntity msg;
        msg.entityID = *i;
        destination->Send(msg);
        state->RemoveEntity(*i);
        state->AckRemove(*i);
        ++num_messages_sent;
        
    }
    
    //if (num_messages_sent)
    //    LogInfo("Sent " + ToString<int>(num_messages_sent) + " scenesync messages");
}

bool SyncManager::ValidateAction(kNet::MessageConnection* source, unsigned messageID, entity_id_t entityID)
{
    if (entityID & LocalEntity)
    {
        LogWarning("Received an entity sync message for a local entity. Disregarding.");
        return false;
    }
    
    // For now, always trust scene actions from server
    if (!owner_->IsServer())
        return true;
    
    // And for now, always also trust scene actions from clients, if they are known and authenticated
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if ((!user) || (user->properties["authenticated"] != "true"))
        return false;
    
    return true;
}

void SyncManager::HandleCreateEntity(kNet::MessageConnection* source, const MsgCreateEntity& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.messageID, entityID))
        return;
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    if (!state)
    {
        LogWarning("Null syncstate for connection! Disregarding CreateEntity message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
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
            LogWarning("Received entity creation from server for entity ID " + ToString<int>(entityID) + " that already exists. Removing the old entity.");
            scene->RemoveEntity(entityID, change);
        }
    }
    
    EntityPtr entity = scene->CreateEntity(entityID);
    if (!entity)
    {
        LogWarning("Scene refused to create entity " + ToString<int>(entityID));
        return;
    }
    
    // Reflect changes back to syncstate
    state->GetOrCreateEntity(entityID);
    
    // Read the components
    for(uint i = 0; i < msg.components.size(); ++i)
    {
        uint type_hash = msg.components[i].componentTypeHash;
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(type_hash, name, change);
        if (component)
        {
            if (msg.components[i].componentData.size())
            {
                DataDeserializer source((const char*)&msg.components[i].componentData[0], msg.components[i].componentData.size());
                try
                {
                    // Deserialize in disconnected mode, then send the notifications later
                    component->DeserializeFromBinary(source, AttributeChange::Disconnected);
                }
                catch(...)
                {
                    ::LogError("Error while deserializing component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
                }
                
                // Reflect changes back to syncstate
                EntitySyncState* entitystate = state->GetOrCreateEntity(entityID);
                ComponentSyncState* componentstate = entitystate->GetOrCreateComponent(type_hash, name);
                UNREFERENCED_PARAM(componentstate);
            }
        }
        else
            LogWarning("Could not create component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
    }
 
    // Emit the entity/componentchanges last, to signal only a coherent state of the whole entity
    scene->EmitEntityCreated(entity, change);
    const Entity::ComponentVector &components = entity->Components();
    for(uint i = 0; i < components.size(); ++i)
        components[i]->ComponentChanged(change);
}

void SyncManager::HandleRemoveEntity(kNet::MessageConnection* source, const MsgRemoveEntity& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;

    if (!ValidateAction(source, msg.messageID, entityID))
        return;
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    if (!state)
    {
        LogWarning("Null syncstate for connection! Disregarding RemoveEntity message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;

    scene->RemoveEntity(entityID, change);
    
    // Reflect changes back to syncstate
    state->RemoveEntity(entityID);
}


void SyncManager::HandleCreateComponents(kNet::MessageConnection* source, const MsgCreateComponents& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.messageID, entityID))
        return;
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    if (!state)
    {
        LogWarning("Null syncstate for connection! Disregarding CreateComponents message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    // See if we can find the entity. If not, create it, should not happen, but we handle it anyway (!!!)
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + ToString<int>(entityID) + " not found for CreateComponents message, creating it now");
        entity = scene->CreateEntity(entityID);
        if (!entity)
        {
            LogWarning("Scene refused to create entity " + ToString<int>(entityID));
            return;
        }
        
        // Reflect changes back to syncstate
        state->GetOrCreateEntity(entityID);
    }
    
    // Read the components. These are not deltaserialized.
    std::vector<ComponentPtr> actually_changed_components;
    
    for(uint i = 0; i < msg.components.size(); ++i)
    {
        uint type_hash = msg.components[i].componentTypeHash;
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(type_hash, name, change);
        if (component)
        {
            if (msg.components[i].componentData.size())
            {
                DataDeserializer source((const char*)&msg.components[i].componentData[0], msg.components[i].componentData.size());
                
                try
                {
                    // Deserialize with no signals first
                    component->DeserializeFromBinary(source, AttributeChange::Disconnected);
                    actually_changed_components.push_back(component);
                }
                catch(...)
                {
                    ::LogError("Error while deserializing component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
                }
                
                // Reflect changes back to syncstate
                EntitySyncState* entitystate = state->GetOrCreateEntity(entityID);
                ComponentSyncState* componentstate = entitystate->GetOrCreateComponent(type_hash, name);
                UNREFERENCED_PARAM(componentstate);
            }
        }
        else
            LogWarning("Could not create component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
    }
    
    // Signal the component changes last
    if (actually_changed_components.size())
    {
        for(uint i = 0; i < actually_changed_components.size(); ++i)
            actually_changed_components[i]->ComponentChanged(change);
    }
}

void SyncManager::HandleUpdateComponents(kNet::MessageConnection* source, const MsgUpdateComponents& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.messageID, entityID))
        return;
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    if (!state)
    {
        LogWarning("Null syncstate for connection! Disregarding UpdateComponents message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    // See if we can find the entity. If not, create it, should not happen, but we handle it anyway (!!!)
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + ToString<int>(entityID) + " not found for UpdateComponents message, creating it now");
        entity = scene->CreateEntity(entityID);
        if (!entity)
        {
            LogWarning("Scene refused to create entity " + ToString<int>(entityID));
            return;
        }
        
        // Reflect changes back to syncstate
        state->GetOrCreateEntity(entityID);
    }
    
    std::map<IComponent*, std::vector<bool> > partially_changed_static_components;
    std::map<IComponent*, std::vector<QString> > partially_changed_dynamic_components;
    
    // Read the static structured components
    for(uint i = 0; i < msg.components.size(); ++i)
    {
        uint type_hash = msg.components[i].componentTypeHash;
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(type_hash, name, change);
        if (component)
        {
            if (msg.components[i].componentData.size())
            {
                DataDeserializer source((const char*)&msg.components[i].componentData[0], msg.components[i].componentData.size());
                
                if (component->HasDynamicStructure())
                    LogWarning("Received static structure update for a dynamic structured component");
                else
                {
                    std::vector<bool> actually_changed_attributes;
                    const AttributeVector& attributes = component->GetAttributes();
                    try
                    {
                        // Deserialize changed attributes (1 bit) with no signals first
                        for(uint i = 0; i < attributes.size(); ++i)
                        {
                            if (source.Read<bit>())
                            {
                                // If attribute supports interpolation, queue interpolation instead
                                bool interpolate = false;
                                if ((!isServer) && (attributes[i]->HasMetadata()) && (attributes[i]->GetMetadata()->interpolation == AttributeMetadata::Interpolate))
                                    interpolate = true;
                                
                                if (!interpolate)
                                {
                                    attributes[i]->FromBinary(source, AttributeChange::Disconnected);
                                    actually_changed_attributes.push_back(true);
                                }
                                else
                                {
                                    IAttribute* endValue = attributes[i]->Clone();
                                    endValue->FromBinary(source, AttributeChange::Disconnected);
                                    /// \todo server's tickrate might not be same as ours. Should perhaps sync it upon join
                                    // Allow a slightly longer interval than the actual tickrate, for possible packet jitter
                                    scene->StartAttributeInterpolation(attributes[i], endValue, update_period_ * 1.35f);
                                    // Do not signal attribute change at this point at all
                                    actually_changed_attributes.push_back(false);
                                }
                            }
                            else
                                actually_changed_attributes.push_back(false);
                        }
                        partially_changed_static_components[component.get()] = actually_changed_attributes;
                    }
                    catch(...)
                    {
                        ::LogError("Error while delta-deserializing component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
                    }
                }
            }
        }
        else
            LogWarning("Could not create component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
    }
    
    // Read the dynamic structured components. For now, they have to be DynamicComponents.
    for(uint i = 0; i < msg.dynamiccomponents.size(); ++i)
    {
        uint type_hash = msg.dynamiccomponents[i].componentTypeHash;
        QString name = QString::fromStdString(BufferToString(msg.dynamiccomponents[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(type_hash, name, change);
        if (component)
        {
            EC_DynamicComponent* dynComp = dynamic_cast<EC_DynamicComponent*>(component.get());
            if (!dynComp)
                LogWarning("Received dynamic attribute update for a component that is not EC_DynamicComponent");
            else
            {
                for(uint j = 0; j < msg.dynamiccomponents[i].attributes.size(); ++j)
                {
                    QString attrName = QString::fromStdString(BufferToString(msg.dynamiccomponents[i].attributes[j].attributeName));
                    QString attrTypeName = QString::fromStdString(BufferToString(msg.dynamiccomponents[i].attributes[j].attributeType));
                    // Empty typename: delete attribute
                    if (!attrTypeName.size())
                    {
                        // Note: signal remove immediately, as nothing should depend on batched attribute remove
                        dynComp->RemoveAttribute(attrName, change);
                    }
                    // Otherwise add or modify attribute
                    else
                    {
                        IAttribute* attr = component->GetAttribute(attrName);
                        if (attr)
                        {
                            // If wrong type of attribute, delete and recreate
                            if (attr->TypeName() != attrTypeName.toStdString())
                            {
                                dynComp->RemoveAttribute(attrName, AttributeChange::Disconnected);
                                attr = dynComp->CreateAttribute(attrTypeName, attrName, AttributeChange::Disconnected);
                            }
                        }
                        else
                            attr = dynComp->CreateAttribute(attrTypeName, attrName, AttributeChange::Disconnected);
                        
                        if (attr)
                        {
                            if (msg.dynamiccomponents[i].attributes[j].attributeData.size())
                            {
                                DataDeserializer source((const char*)&msg.dynamiccomponents[i].attributes[j].attributeData[0], 
                                    msg.dynamiccomponents[i].attributes[j].attributeData.size());
                                attr->FromBinary(source, AttributeChange::Disconnected);
                            }
                            partially_changed_dynamic_components[dynComp].push_back(attrName);
                        }
                        else
                            LogWarning("Could not create attribute type " + attrTypeName.toStdString() + " to dynamic component");
                    }
                }
            }
        }
        else
            LogWarning("Could not create component " + framework_->GetComponentManager()->GetComponentTypeName(type_hash).toStdString());
    }
    
    // Signal static components
    std::map<IComponent*, std::vector<bool> >::iterator i = partially_changed_static_components.begin();
    while(i != partially_changed_static_components.end())
    {
        // Crazy logic might have deleted the component, so we get the corresponding shared ptr from the entity to be sure
        ComponentPtr compShared = i->first->shared_from_this();
        if (compShared)
        {
            const AttributeVector& attributes = compShared->GetAttributes();
            for(uint j = 0; (j < attributes.size()) && (j < i->second.size()); ++j)
                if (i->second[j])
                {
                    currentSender = source;
                    compShared->EmitAttributeChanged(attributes[j], change);
                }
        }
        ++i;
    }
    
    // Signal dynamic components
    std::map<IComponent*, std::vector<QString> >::iterator j = partially_changed_dynamic_components.begin();
    while(j != partially_changed_dynamic_components.end())
    {
        // Crazy logic might have deleted the component, so we get the corresponding shared ptr from the entity to be sure
        ComponentPtr compShared = j->first->shared_from_this();
        if (compShared)
        {
            for(uint k = 0; k < j->second.size(); ++k)
            {
                IAttribute* attr = compShared->GetAttribute(j->second[k]);
                if (attr)
                {
                    currentSender = source;
                    compShared->EmitAttributeChanged(attr, change);
                }
            }
        }
        ++j;
    }
}

void SyncManager::HandleRemoveComponents(kNet::MessageConnection* source, const MsgRemoveComponents& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.messageID, entityID))
        return;
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    if (!state)
    {
        LogWarning("Null syncstate for connection! Disregarding RemoveComponents message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    EntityPtr entity = scene->GetEntity(msg.entityID);
    if (!entity)
        return;
    
    for(unsigned i = 0; i < msg.components.size(); ++i)
    {
        uint type_hash = msg.components[i].componentTypeHash;
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        
        ComponentPtr comp = entity->GetComponent(type_hash, name);
        if (comp)
        {
            entity->RemoveComponent(comp, change);
            comp.reset();
        }
        
        // Reflect changes back to syncstate
        EntitySyncState* entitystate = state->GetEntity(entityID);
        if (entitystate)
            entitystate->RemoveComponent(type_hash, name);
    }
}

void SyncManager::HandleEntityIDCollision(kNet::MessageConnection* source, const MsgEntityIDCollision& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    if (owner_->IsServer())
    {
        LogWarning("Received EntityIDCollision from a client, disregarding.");
        return;
    }
    
    LogDebug("An entity ID collision occurred. Entity " + ToString<int>(msg.oldEntityID) + " became " + ToString<int>(msg.newEntityID));
    scene->ChangeEntityId(msg.oldEntityID, msg.newEntityID);
    
    // Do the change also in server scene replication state
    SceneSyncState* state = GetSceneSyncState(source);
    if (state)
    {
        state->entities_[msg.newEntityID] = state->entities_[msg.oldEntityID];
        state->RemoveEntity(msg.oldEntityID);
    }
}

void SyncManager::HandleEntityAction(kNet::MessageConnection* source, MsgEntityAction& msg)
{
    bool isServer = owner_->IsServer();
    
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
        return;
    
    entity_id_t entityId = msg.entityId;
    EntityPtr entity = scene->GetEntity(entityId);
    if (!entity)
    {
        LogWarning("Entity " + ToString<int>(entityId) + " not found for EntityAction message.");
        return;
    }

    // If we are server, get the user who sent the action, so it can be queried
    if (isServer)
    {
        Server* server = owner_->GetServer().get();
        if (server)
        {
            UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
            server->SetActionSender(user);
        }
    }
    
    QString action = BufferToString(msg.name).c_str();
    QStringList params;
    for(uint i = 0; i < msg.parameters.size(); ++i)
        params << BufferToString(msg.parameters[i].parameter).c_str();

    EntityAction::ExecutionType type = (EntityAction::ExecutionType)(msg.executionType);

    if ((type & EntityAction::Local) != 0 || (isServer && (type & EntityAction::Server) != 0))
        entity->Exec(EntityAction::Local, action, params); // Execute the action locally, so that it doesn't immediately propagate back to network for sending.

    // If execution type is Peers, replicate to all peers but the sender.
    if (isServer && type & EntityAction::Peers)
    {
        msg.executionType = (u8)EntityAction::Local;
        foreach(UserConnection* userConn, owner_->GetKristalliModule()->GetUserConnections())
            if (userConn->connection != source) // The EC action will not be sent to the machine that originated the request to send an action to all peers.
                userConn->connection->Send(msg);
    }
    
    // Clear the action sender after action handling
    if (isServer)
    {
        Server* server = owner_->GetServer().get();
        if (server)
            server->SetActionSender(0);
    }
}

void SyncManager::HandleAssetDiscovery(kNet::MessageConnection* source, MsgAssetDiscovery& msg)
{
    // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
    bool isServer = owner_->IsServer();
    if (isServer)
    {
        foreach(UserConnection* userConn, owner_->GetKristalliModule()->GetUserConnections())
        {
            if (userConn->connection != source)
                userConn->connection->Send(msg);
        }
    }
    
    // Then do an asset request of the ref, and post a discovery notification
    QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
    QString assetType = QString::fromStdString(BufferToString(msg.assetType));
    framework_->Asset()->RequestAsset(assetRef, assetType);
    framework_->Asset()->EmitAssetDiscovered(assetRef, assetType);
}

void SyncManager::HandleAssetDeleted(kNet::MessageConnection* source, MsgAssetDeleted& msg)
{
    // If we are server, the message had to come from a client, and we replicate it to everyone except the sender
    bool isServer = owner_->IsServer();
    if (isServer)
    {
        foreach(UserConnection* userConn, owner_->GetKristalliModule()->GetUserConnections())
        {
            if (userConn->connection != source)
                userConn->connection->Send(msg);
        }
    }
    
    // Do not actually unload the asset, but make sure the asset storage(s) update their refs in case they had it
    QString assetRef = QString::fromStdString(BufferToString(msg.assetRef));
    framework_->Asset()->EmitAssetDeleted(assetRef);
}

SceneSyncState* SyncManager::GetSceneSyncState(kNet::MessageConnection* connection)
{
    if (!owner_->IsServer())
        return &server_syncstate_;
    
    UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
    {
        if ((*i)->connection == connection)
            return checked_static_cast<SceneSyncState*>((*i)->syncState.get());
    }
    return 0;
}

void SyncManager::OnAssetUploaded(const QString& assetRef)
{
    // Check whether the asset upload needs to be replicated
    AssetAPI::AssetRefType type = framework_->Asset()->ParseAssetRef(assetRef);
    if ((type == AssetAPI::AssetRefInvalid) || (type == AssetAPI::AssetRefLocalPath) || (type == AssetAPI::AssetRefLocalUrl))
        return;
    
    bool isServer = owner_->IsServer();
    
    MsgAssetDiscovery msg;
    msg.assetRef = StringToBuffer(assetRef.toStdString());
    /// \todo Would need the assettype as well
    
    // If we are server, send to everyone
    if (isServer)
    {
        foreach(UserConnection* userConn, owner_->GetKristalliModule()->GetUserConnections())
            userConn->connection->Send(msg);
    }
    // If we are client, send to server
    else
    {
        kNet::MessageConnection* connection = owner_->GetClient()->GetConnection();
        if (connection)
            connection->Send(msg);
    }
}

void SyncManager::OnAssetDeleted(const QString& assetRef)
{
    // Check whether the asset delete needs to be replicated
    AssetAPI::AssetRefType type = framework_->Asset()->ParseAssetRef(assetRef);
    if ((type == AssetAPI::AssetRefInvalid) || (type == AssetAPI::AssetRefLocalPath) || (type == AssetAPI::AssetRefLocalUrl))
        return;
    
    bool isServer = owner_->IsServer();
    
    MsgAssetDeleted msg;
    msg.assetRef = StringToBuffer(assetRef.toStdString());
    
    // If we are server, send to everyone
    if (isServer)
    {
        foreach(UserConnection* userConn, owner_->GetKristalliModule()->GetUserConnections())
            userConn->connection->Send(msg);
    }
    // If we are client, send to server
    else
    {
        kNet::MessageConnection* connection = owner_->GetClient()->GetConnection();
        if (connection)
            connection->Send(msg);
    }
}


}
