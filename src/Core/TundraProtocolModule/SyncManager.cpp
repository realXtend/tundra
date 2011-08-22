// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CoreStringUtils.h"
#include "DebugOperatorNew.h"
#include "KristalliProtocolModule.h"
#include "SyncManager.h"
#include "Scene.h"
#include "Entity.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "TundraMessages.h"
#include "MsgEntityAction.h"
#include "EC_DynamicComponent.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "Profiler.h"

#include "SceneAPI.h"

#include <kNet.h>

#include <cstring>

#include "MemoryLeakCheck.h"

using namespace kNet;

// This variable is used for the sender echo & interpolation stop check
kNet::MessageConnection* currentSender = 0;

namespace TundraLogic
{

void SyncManager::QueueMessage(kNet::MessageConnection* connection, kNet::message_id_t id, bool reliable, bool inOrder, kNet::DataSerializer& ds)
{
    kNet::NetworkMessage* msg = connection->StartNewMessage(id, ds.BytesFilled());
    memcpy(msg->data, ds.GetData(), ds.BytesFilled());
    msg->reliable = reliable;
    msg->inOrder = inOrder;
    connection->EndAndQueueMessage(msg);
}

void SyncManager::WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp)
{
    // Component identification
    ds.AddVLE<VLE8_16_32>(comp->Id());
    ds.AddVLE<VLE8_16_32>(comp->TypeId());
    ds.AddString(comp->Name().toStdString());
    
    // Create a nested dataserializer for the attributes, so we can survive unknown or incompatible components
    kNet::DataSerializer attrDs(attrDataBuffer_, 16 * 1024);
    
    // Static-structured attributes
    unsigned numStaticAttrs = comp->NumStaticAttributes();
    const AttributeVector& attrs = comp->Attributes();
    for (uint i = 0; i < numStaticAttrs; ++i)
        attrs[i]->ToBinary(attrDs);
    
    // Dynamic-structured attributes (use EOF to detect so do not need to send their amount)
    for (unsigned i = numStaticAttrs; i < attrs.size(); ++i)
    {
        if (attrs[i] && attrs[i]->IsDynamic())
        {
            attrDs.Add<u8>(i); // Index
            attrDs.Add<u8>(attrs[i]->TypeId());
            attrDs.AddString(attrs[i]->Name().toStdString());
            attrs[i]->ToBinary(attrDs);
        }
    }
    
    // Add the attribute array to the main serializer
    ds.AddArray<u8>((unsigned char*)attrDataBuffer_, attrDs.BytesFilled());
}

SyncManager::SyncManager(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    update_period_(1.0f / 30.0f),
    update_acc_(0.0)
{
    KristalliProtocol::KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocol::KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::message_id_t, const char*, size_t)));
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
    Scene* sceneptr = scene.get();
    
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
    connect(sceneptr, SIGNAL( ActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecTypeField) ),
        SLOT( OnActionTriggered(Entity *, const QString &, const QStringList &, EntityAction::ExecTypeField)));
}

void SyncManager::HandleKristalliMessage(kNet::MessageConnection* source, kNet::message_id_t id, const char* data, size_t numBytes)
{
    switch (id)
    {
    case cEntityActionMessage:
        {
            MsgEntityAction msg(data, numBytes);
            HandleEntityAction(source, msg);
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
    {
        LogWarning("SyncManager: Cannot handle new user connection message - No scene set!");
        return;
    }
    
    // Connect to actions sent to specifically to this user
    connect(user, SIGNAL(ActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)), this, SLOT(OnUserActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)));
    
    // Mark all entities in the sync state as new so we will send them
    user->syncState.Clear();
    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        if (entity->IsLocal())
            continue;
        entity_id_t id = entity->Id();
        user->syncState.MarkEntityDirty(id);
    }
}

void SyncManager::OnAttributeChanged(IComponent* comp, IAttribute* attr, AttributeChange::Type change)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;

    bool isServer = owner_->IsServer();
    
    // Client: Check for stopping interpolation, if we change a currently interpolating variable ourselves
    if (!isServer)
    {
        ScenePtr scene = scene_.lock();
        if ((scene) && (!scene->IsInterpolating()) && (!currentSender))
        {
            if ((attr->Metadata()) && (attr->Metadata()->interpolation == AttributeMetadata::Interpolate))
                // Note: it does not matter if the attribute was not actually interpolating
                scene->EndAttributeInterpolation(attr);
        }
    }
    
    if ((change != AttributeChange::Replicate) || (comp->IsLocal()))
        return;
    Entity* entity = comp->ParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    
    if (isServer)
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            // Do not echo the attribute change back to the sender
            if ((*i)->connection == currentSender)
                continue;
            (*i)->syncState.MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
        }
    }
    else
    {
        server_syncstate_.MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
    }
    
    // This attribute changing might in turn cause other attributes to change on the server, and these must be echoed to all, so reset sender now
    currentSender = 0;
}

void SyncManager::OnComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    assert(entity && comp);
    if (!entity || !comp)
        return;

    if ((change != AttributeChange::Replicate) || (comp->IsLocal()))
        return;
    if (entity->IsLocal())
        return;
    
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            (*i)->syncState.MarkComponentDirty(entity->Id(), comp->Id());
    }
    else
    {
        server_syncstate_.MarkComponentDirty(entity->Id(), comp->Id());
    }
}

void SyncManager::OnComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    assert(entity && comp);
    if (!entity || !comp)
        return;
    if ((change != AttributeChange::Replicate) || (comp->IsLocal()))
        return;
    if (entity->IsLocal())
        return;
    
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            (*i)->syncState.MarkComponentRemoved(entity->Id(), comp->Id());
    }
    else
    {
        server_syncstate_.MarkComponentRemoved(entity->Id(), comp->Id());
    }
}

void SyncManager::OnEntityCreated(Entity* entity, AttributeChange::Type change)
{
    assert(entity);
    if (!entity)
        return;
    if ((change != AttributeChange::Replicate) || (entity->IsLocal()))
        return;

    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            (*i)->syncState.MarkEntityDirty(entity->Id());
            if ((*i)->syncState.entities[entity->Id()].removed)
            {
                LogWarning("An entity with ID " + QString::number(entity->Id()) + " is queued to be deleted, but a new entity \"" + 
                    entity->Name() + "\" is to be added to the scene!");
            }
        }
    }
    else
    {
        server_syncstate_.MarkEntityDirty(entity->Id());
    }
}

void SyncManager::OnEntityRemoved(Entity* entity, AttributeChange::Type change)
{
    assert(entity);
    if (!entity)
        return;
    if (change != AttributeChange::Replicate)
        return;
    if (entity->IsLocal())
        return;
    
    if (owner_->IsServer())
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            (*i)->syncState.MarkEntityRemoved(entity->Id());
    }
    else
    {
        server_syncstate_.MarkEntityRemoved(entity->Id());
    }
}

void SyncManager::OnActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type)
{
    //Scene* scene = scene_.lock().get();
    //assert(scene);

    // If we are the server and the local script on this machine has requested a script to be executed on the server, it
    // means we just execute the action locally here, without sending to network.
    bool isServer = owner_->IsServer();
    if (isServer && (type & EntityAction::Server) != 0)
    {
        //LogInfo("EntityAction " + action. + " type Server on server.");
        entity->Exec(EntityAction::Local, action, params);
    }

    // Craft EntityAction message.
    MsgEntityAction msg;
    msg.entityId = entity->Id();
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
        //LogInfo("Tundra client sending EntityAction " + action + " type " + ToString(type));
        msg.executionType = (u8)(type & ~EntityAction::Local);
        owner_->GetClient()->GetConnection()->Send(msg);
    }

    if (isServer && (type & EntityAction::Peers) != 0)
    {
        msg.executionType = (u8)EntityAction::Local; // Propagate as local actions.
        foreach(UserConnectionPtr c, owner_->GetKristalliModule()->GetUserConnections())
        {
            if (c->properties["authenticated"] == "true" && c->connection)
            {
                //LogInfo("peer " + action);
                c->connection->Send(msg);
            }
        }
    }
}

void SyncManager::OnUserActionTriggered(UserConnection* user, Entity *entity, const QString &action, const QStringList &params)
{
    assert(user && entity);
    if (!entity || !user)
        return;
    bool isServer = owner_->IsServer();
    if (!isServer)
        return; // Should never happen
    if (user->properties["authenticated"] != "true")
        return; // Not yet authenticated, do not receive actions
    
    // Craft EntityAction message.
    MsgEntityAction msg;
    msg.entityId = entity->Id();
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
            ProcessSyncState((*i)->connection, (*i)->syncState);
    }
    else
    {
        // If we are client, process just the server sync state
        kNet::MessageConnection* connection = owner_->GetKristalliModule()->GetMessageConnection();
        if (connection)
            ProcessSyncState(connection, server_syncstate_);
    }
}

void SyncManager::ProcessSyncState(kNet::MessageConnection* destination, SceneSyncState& state)
{
    PROFILE(SyncManager_ProcessSyncState);
    
    ScenePtr scene = scene_.lock();
    
    int numMessagesSent = 0;
    std::set<entity_id_t> newDeletedEntities;
    
    // Process the state's dirty entity queue.
    /// \todo Limit and prioritize the data sent. For now the whole queue is processed, regardless of whether the connection is being saturated.
    while (!state.dirtyQueue.empty())
    {
        EntitySyncState& entityState = *state.dirtyQueue.front();
        state.dirtyQueue.pop_front();
        entityState.isInQueue = false;
        
        EntityPtr entity = scene->GetEntity(entityState.id);
        bool removeState = false;
        if (!entity)
        {
            entityState.isNew = false;
            removeState = true;
        }
        else
        {
            // Make sure we don't send data for local entities
            if (entity->IsLocal())
                continue;
        }
        
        // Remove entity
        if (entityState.removed)
        {
            // If we have both new & removed flags on the entity, send the delete now, but queue for creation on a later frame
            if (entityState.isNew)
            {
                LogWarning("Entity " + QString::number(entityState.id) + " queued for both deletion and creation. Sending deletion first and creation later.");
                newDeletedEntities.insert(entityState.id);
                // The delete has been processed. Do not remember it anymore.
                entityState.removed = false;
            }
            else
                removeState = true;
            
            kNet::DataSerializer ds(removeEntityBuffer_, 1024);
            ds.AddVLE<VLE8_16_32>(entityState.id);
            QueueMessage(destination, cRemoveEntityMessage, true, true, ds);
            ++numMessagesSent;
        }
        // New entity
        else if (entityState.isNew)
        {
            kNet::DataSerializer ds(createEntityBuffer_, 64 * 1024);
            
            // Entity identification and temporary flag
            ds.AddVLE<VLE8_16_32>(entityState.id);
            ds.Add<bit>(entity->IsTemporary() ? 1 : 0);
            
            const Entity::ComponentMap& components = entity->Components();
            // Count the amount of replicated components
            uint numReplicatedComponents = 0;
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                if (i->second->IsReplicated()) ++numReplicatedComponents;
            ds.AddVLE<VLE8_16_32>(numReplicatedComponents);
            
            // Serialize each replicated component
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            {
                ComponentPtr comp = i->second;
                if (!comp->IsReplicated())
                    continue;
                WriteComponentFullUpdate(ds, comp);
            }
            
            QueueMessage(destination, cCreateEntityMessage, true, true, ds);
            ++numMessagesSent;
            
            // The create has been processed fully. Clear dirty flags.
            entityState.DirtyProcessed();
        }
        else if (entity)
        {
            // Components or attributes have been added, changed, or removed. Prepare the dataserializers
            kNet::DataSerializer removeCompsDs(removeCompsBuffer_, 1024);
            kNet::DataSerializer removeAttrsDs(removeAttrsBuffer_, 1024);
            kNet::DataSerializer createCompsDs(createCompsBuffer_, 64 * 1024);
            kNet::DataSerializer createAttrsDs(createAttrsBuffer_, 16 * 1024);
            kNet::DataSerializer editAttrsDs(editAttrsBuffer_, 64 * 1024);
            
            while (!entityState.dirtyQueue.empty())
            {
                ComponentSyncState& compState = *entityState.dirtyQueue.front();
                entityState.dirtyQueue.pop_front();
                compState.isInQueue = false;
                
                ComponentPtr comp = entity->GetComponentById(compState.id);
                bool removeState = false;
                if (!comp)
                {
                    compState.isNew = false;
                    removeState = true;
                }
                else
                {
                    // Make sure we don't send data for local components
                    if (comp->IsLocal())
                        continue;
                }
                
                // Remove component
                if (compState.removed)
                {
                    removeState = true;
                    
                    // If first component, write the entity ID first
                    if (!removeCompsDs.BytesFilled())
                        removeCompsDs.AddVLE<VLE8_16_32>(entityState.id);
                    // Then add component ID
                    removeCompsDs.AddVLE<VLE8_16_32>(compState.id);
                }
                // New component
                else if (compState.isNew)
                {
                    // If first component, write the entity ID first
                    if (!createCompsDs.BytesFilled())
                        createCompsDs.AddVLE<VLE8_16_32>(entityState.id);
                    // Then add the component data
                    WriteComponentFullUpdate(createCompsDs, comp);
                }
                // Added/removed/edited attributes
                else if (comp)
                {
                    const AttributeVector& attrs = comp->Attributes();
                    
                    for (std::map<u8, bool>::iterator i = compState.newAndRemovedAttributes.begin(); i != compState.newAndRemovedAttributes.end(); ++i)
                    {
                        u8 attrIndex = i->first;
                        // Clear the corresponding dirty flags, so that we don't redundantly send attribute edited data.
                        compState.dirtyAttributes[attrIndex >> 3] &= ~(1 << (attrIndex & 7));
                        
                        if (i->second)
                        {
                            // Create attribute. Make sure it exists and is dynamic.
                            if (attrIndex >= attrs.size() || !attrs[attrIndex])
                                ::LogError("CreateAttribute for nonexisting attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in entity " + entity->ToString() + ". Discarding.");
                            else if (!attrs[attrIndex]->IsDynamic())
                                ::LogError("CreateAttribute for a static attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in entity " + entity->ToString() + ". Discarding.");
                            else
                            {
                                // If first attribute, write the entity ID first
                                if (!createAttrsDs.BytesFilled())
                                    createAttrsDs.AddVLE<VLE8_16_32>(entityState.id);
                                
                                IAttribute* attr = attrs[attrIndex];
                                createAttrsDs.AddVLE<VLE8_16_32>(compState.id);
                                createAttrsDs.Add<u8>(attrIndex); // Index
                                createAttrsDs.Add<u8>(attr->TypeId());
                                createAttrsDs.AddString(attr->Name().toStdString());
                                attr->ToBinary(createAttrsDs);
                            }
                        }
                        else
                        {
                            // Remove attribute
                            // If first attribute, write the entity ID first
                            if (!removeAttrsDs.BytesFilled())
                                removeAttrsDs.AddVLE<VLE8_16_32>(entityState.id);
                            removeAttrsDs.AddVLE<VLE8_16_32>(compState.id);
                            removeAttrsDs.Add<u8>(attrIndex);
                        }
                    }
                    compState.newAndRemovedAttributes.clear();
                    
                    // Now, if remaining dirty bits exist, they must be sent in the edit attributes message. These are the majority of our network data.
                    changedAttributes_.clear();
                    unsigned numBytes = (attrs.size() + 7) >> 3;
                    for (unsigned i = 0; i < numBytes; ++i)
                    {
                        u8 byte = compState.dirtyAttributes[i];
                        if (byte)
                        {
                            for (unsigned j = 0; j < 8; ++j)
                            {
                                if (byte & (1 << j))
                                {
                                    u8 attrIndex = i * 8 + j;
                                    if (attrIndex < attrs.size() && attrs[attrIndex])
                                        changedAttributes_.push_back(attrIndex);
                                    else
                                        ::LogError("Attribute change for a nonexisting attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in entity " + entity->ToString() + ". Discarding.");
                                }
                            }
                        }
                    }
                    if (changedAttributes_.size())
                    {
                        // If first component for which attribute changes are sent, write the entity ID first
                        if (!editAttrsDs.BytesFilled())
                            editAttrsDs.AddVLE<VLE8_16_32>(entityState.id);
                        editAttrsDs.AddVLE<VLE8_16_32>(compState.id);
                        
                        // There are changed attributes. Check if it is more optimal to send attribute indices, or the whole bitmask
                        unsigned bitsMethod1 = changedAttributes_.size() * 8 + 8;
                        unsigned bitsMethod2 = attrs.size();
                        if (bitsMethod1 <= bitsMethod2)
                        {
                            editAttrsDs.Add<bit>(0);
                            editAttrsDs.Add<u8>(changedAttributes_.size());
                            for (unsigned i = 0; i < changedAttributes_.size(); ++i)
                            {
                                editAttrsDs.Add<u8>(changedAttributes_[i]);
                                attrs[changedAttributes_[i]]->ToBinary(editAttrsDs);
                            }
                        }
                        else
                        {
                            editAttrsDs.Add<bit>(1);
                            for (unsigned i = 0; i < attrs.size(); ++i)
                            {
                                if (compState.dirtyAttributes[i >> 3] & (1 << (i & 7)))
                                {
                                    editAttrsDs.Add<bit>(1);
                                    attrs[i]->ToBinary(editAttrsDs);
                                }
                                else
                                    editAttrsDs.Add<bit>(0);
                            }
                        }
                        
                        // Now zero out all remaining dirty bits
                        for (unsigned i = 0; i < numBytes; ++i)
                            compState.dirtyAttributes[i] = 0;
                    }
                }
                
                if (removeState)
                    entityState.components.erase(compState.id);
            }
            
            // Send the messages which have data
            if (removeCompsDs.BytesFilled())
            {
                QueueMessage(destination, cRemoveComponentsMessage, true, true, removeCompsDs);
                ++numMessagesSent;
            }
            if (removeAttrsDs.BytesFilled())
            {
                QueueMessage(destination, cRemoveAttributesMessage, true, true, removeAttrsDs);
                ++numMessagesSent;
            }
            if (createCompsDs.BytesFilled())
            {
                QueueMessage(destination, cCreateComponentsMessage, true, true, createCompsDs);
                ++numMessagesSent;
            }
            if (createAttrsDs.BytesFilled())
            {
                QueueMessage(destination, cCreateAttributesMessage, true, true, createAttrsDs);
                ++numMessagesSent;
            }
            if (editAttrsDs.BytesFilled())
            {
                QueueMessage(destination, cEditAttributesMessage, true, true, editAttrsDs);
                ++numMessagesSent;
            }
            
            // The entity has been processed fully. Clear dirty flags.
            entityState.DirtyProcessed();
        }
        
        if (removeState)
            state.entities.erase(entityState.id);
    }
    //if (num_messages_sent)
    //    LogInfo("Sent " + ToString<int>(num_messages_sent) + " scenesync messages");
}

bool SyncManager::ValidateAction(kNet::MessageConnection* source, unsigned messageID, entity_id_t entityID)
{
    assert(source);
    
    // For now, always trust scene actions from server
    if (!owner_->IsServer())
        return true;
    
    // And for now, always also trust scene actions from clients, if they are known and authenticated
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if ((!user) || (user->properties["authenticated"] != "true"))
        return false;
    
    return true;
}

/*
void SyncManager::HandleCreateEntity(kNet::MessageConnection* source, const MsgCreateEntity& msg)
{
    assert(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
    {
        LogWarning("SyncManager: Ignoring received MsgCreateEntity as no scene exists!");
        return;
    }
    
    entity_id_t entityID = msg.entityID;
    if (!ValidateAction(source, msg.messageID, entityID))
        return;

    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!scene->AllowModifyEntity(user, 0)) //should be 'ModifyScene', but ModifyEntity is now the signal that covers all
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
            entity_id_t newEntityID = scene->NextFreeId();
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
        u32 typeId = msg.components[i].componentTypeHash; ///\todo VLE this!
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(typeId, name, change);
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
                    ::LogError("Error while deserializing component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"! (typeID: " + QString::number(typeId) + ")");
                }
                
                // Reflect changes back to syncstate
                EntitySyncState* entitystate = state->GetOrCreateEntity(entityID);
                ComponentSyncState* componentstate = entitystate->GetOrCreateComponent(typeId, name);
                UNREFERENCED_PARAM(componentstate);
            }
        }
        else
            LogWarning("Could not create component with type " + framework_->Scene()->GetComponentTypeName(typeId));
    }

    // Emit the entity/componentchanges last, to signal only a coherent state of the whole entity
    scene->EmitEntityCreated(entity.get(), change);
    const Entity::ComponentVector &components = entity->Components();
    for(uint i = 0; i < components.size(); ++i)
        components[i]->ComponentChanged(change);
}

void SyncManager::HandleRemoveEntity(kNet::MessageConnection* source, const MsgRemoveEntity& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
    {
        LogWarning("SyncManager: Ignoring received MsgRemoveEntity as no scene exists!");
        return;
    }
    
    entity_id_t entityID = msg.entityID;

    if (!ValidateAction(source, msg.messageID, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!scene->AllowModifyEntity(user, entity.get()))
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
    {
        LogWarning("SyncManager: Ignoring received MsgCreateComponents as no scene exists!");
        return;
    }
    
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
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!entity)
    {
        if (!scene->AllowModifyEntity(user, 0)) //to check if creating entities is allowed (for this user)
            return;

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

    if (!scene->AllowModifyEntity(user, entity.get()))
        return;
    
    // Read the components. These are not deltaserialized.
    std::vector<ComponentPtr> actually_changed_components;
    
    for(uint i = 0; i < msg.components.size(); ++i)
    {
        u32 typeId = msg.components[i].componentTypeHash; ///\todo VLE this!
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(typeId, name, change);
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
                    ::LogError("Error while deserializing component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"! (typeID: " + QString::number(typeId) + ")");
                }
                
                // Reflect changes back to syncstate
                EntitySyncState* entitystate = state->GetOrCreateEntity(entityID);
                ComponentSyncState* componentstate = entitystate->GetOrCreateComponent(typeId, name);
                UNREFERENCED_PARAM(componentstate);
            }
        }
        else
            LogWarning("Could not create component with type " + framework_->Scene()->GetComponentTypeName(typeId));
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
    {
        LogWarning("SyncManager: Ignoring received MsgUpdateComponents as no scene exists!");
        return;
    }
    
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
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!entity)
    {
        if (!scene->AllowModifyEntity(user, 0)) //to check if creating entities is allowed (for this user)
            return;

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

    if (!scene->AllowModifyEntity(user, entity.get()))
        return;
    
    std::map<IComponent*, std::vector<bool> > partially_changed_static_components;
    std::map<IComponent*, std::vector<QString> > partially_changed_dynamic_components;
    
    // Read the static structured components
    for(uint i = 0; i < msg.components.size(); ++i)
    {
        u32 typeId = msg.components[i].componentTypeHash; ///\todo VLE this!
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(typeId, name, change);
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
                    const AttributeVector& attributes = component->Attributes();
                    try
                    {
                        // Deserialize changed attributes (1 bit) with no signals first
                        for(uint i = 0; i < attributes.size(); ++i)
                        {
                            if (source.Read<bit>())
                            {
                                // If attribute supports interpolation, queue interpolation instead
                                bool interpolate = false;
                                if ((!isServer) && (attributes[i]->Metadata()) && (attributes[i]->Metadata()->interpolation == AttributeMetadata::Interpolate))
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
                        ::LogError("Error while delta-deserializing component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
                    }
                }
            }
        }
        else
            LogWarning("Could not create component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
    }
    
    // Read the dynamic structured components. For now, they have to be DynamicComponents.
    for(uint i = 0; i < msg.dynamiccomponents.size(); ++i)
    {
        u32 typeId = msg.dynamiccomponents[i].componentTypeHash; ///\todo VLE this!
        QString name = QString::fromStdString(BufferToString(msg.dynamiccomponents[i].componentName));
        ComponentPtr component = entity->GetOrCreateComponent(typeId, name, change);
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
                            if (attr->TypeName() != attrTypeName)
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
                            LogWarning("Could not create attribute type " + attrTypeName + " to dynamic component");
                    }
                }
            }
        }
        else
            LogWarning("Could not create component \"" + framework_->Scene()->GetComponentTypeName(typeId) + "\"!");
    }
    
    // Signal static components
    std::map<IComponent*, std::vector<bool> >::iterator i = partially_changed_static_components.begin();
    while(i != partially_changed_static_components.end())
    {
        // Crazy logic might have deleted the component, so we get the corresponding shared ptr from the entity to be sure
        ComponentPtr compShared = i->first->shared_from_this();
        if (compShared)
        {
            const AttributeVector& attributes = compShared->Attributes();
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
    {
        LogWarning("SyncManager: Ignoring received MsgRemoveComponents as no scene exists!");
        return;
    }
    
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

    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!scene->AllowModifyEntity(user, entity.get()))
        return;
    
    for(unsigned i = 0; i < msg.components.size(); ++i)
    {
        u32 typeId = msg.components[i].componentTypeHash; ///\todo VLE this!
        QString name = QString::fromStdString(BufferToString(msg.components[i].componentName));
        
        ComponentPtr comp = entity->GetComponent(typeId, name);
        if (comp)
        {
            entity->RemoveComponent(comp, change);
            comp.reset();
        }
        
        // Reflect changes back to syncstate
        EntitySyncState* entitystate = state->GetEntity(entityID);
        if (entitystate)
            entitystate->RemoveComponent(typeId, name);
    }
}

void SyncManager::HandleEntityIDCollision(kNet::MessageConnection* source, const MsgEntityIDCollision& msg)
{
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
    {
        LogWarning("SyncManager: Ignoring received MsgEntityIDCollision as no scene exists!");
        return;
    }
    
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
*/

void SyncManager::HandleEntityAction(kNet::MessageConnection* source, MsgEntityAction& msg)
{
    bool isServer = owner_->IsServer();
    
    ScenePtr scene = GetRegisteredScene();
    if (!scene)
    {
        LogWarning("SyncManager: Ignoring received MsgEntityAction as no scene exists!");
        return;
    }
    
    entity_id_t entityId = msg.entityId;
    EntityPtr entity = scene->GetEntity(entityId);
    if (!entity)
    {
        LogWarning("Entity " + ToString<int>(entityId) + " not found for EntityAction message.");
        return; ///\todo Are we ok to return here? Perhaps we should replicate this action to peers if they might have an entity with that id in the scene?
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

    EntityAction::ExecType type = (EntityAction::ExecType)(msg.executionType);

    bool handled = false;

    if ((type & EntityAction::Local) != 0 || (isServer && (type & EntityAction::Server) != 0))
    {
        entity->Exec(EntityAction::Local, action, params); // Execute the action locally, so that it doesn't immediately propagate back to network for sending.
        handled = true;
    }

    // If execution type is Peers, replicate to all peers but the sender.
    if (isServer && (type & EntityAction::Peers) != 0)
    {
        msg.executionType = (u8)EntityAction::Local;
        foreach(UserConnectionPtr userConn, owner_->GetKristalliModule()->GetUserConnections())
            if (userConn->connection != source) // The EC action will not be sent to the machine that originated the request to send an action to all peers.
                userConn->connection->Send(msg);
        handled = true;
    }
    
    if (!handled)
        LogWarning("SyncManager: Received MsgEntityAction message \"" + action + "\", but it went unhandled because of its type=" + QString::number(type));

    // Clear the action sender after action handling
    Server *server = owner_->GetServer().get();
    if (server)
        server->SetActionSender(0);
}

SceneSyncState* SyncManager::GetSceneSyncState(kNet::MessageConnection* connection)
{
    if (!owner_->IsServer())
        return &server_syncstate_;
    
    UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
    {
        if ((*i)->connection == connection)
            return &(*i)->syncState;
    }
    return 0;
}

}
