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

// This variable is used for the interpolation stop check
kNet::MessageConnection* currentSender = 0;

namespace TundraLogic
{

void SyncManager::QueueMessage(kNet::MessageConnection* connection, kNet::message_id_t id, bool reliable, bool inOrder, kNet::DataSerializer& ds)
{
    //std::cout << "Queuing message " << id << " size " << ds.BytesFilled() << std::endl;
    kNet::NetworkMessage* msg = connection->StartNewMessage(id, ds.BytesFilled());
    memcpy(msg->data, ds.GetData(), ds.BytesFilled());
    msg->reliable = reliable;
    msg->inOrder = inOrder;
    msg->priority = 100; // Fixed priority as in those defined with xml
    connection->EndAndQueueMessage(msg);
}

void SyncManager::WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp)
{
    //std::cout << "Writing component fullupdate id " << comp->Id() << " typeid " << comp->TypeId() << std::endl;
    // Component identification
    ds.AddVLE<kNet::VLE8_16_32>(comp->Id() & UniqueIdGenerator::LAST_REPLICATED_ID);
    ds.AddVLE<kNet::VLE8_16_32>(comp->TypeId());
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
    ds.AddVLE<kNet::VLE8_16_32>(attrDs.BytesFilled());
    ds.AddArray<u8>((unsigned char*)attrDataBuffer_, attrDs.BytesFilled());
}

SyncManager::SyncManager(TundraLogicModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    updatePeriod_(1.0f / 30.0f),
    updateAcc_(0.0)
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
    updatePeriod_ = period;
}

SceneSyncState* SyncManager::SceneState(int connectionId)
{
    if (!owner_->IsServer())
        return 0;
    return SceneState(owner_->GetServer()->GetUserConnection(connectionId));
}

SceneSyncState* SyncManager::SceneState(UserConnection *connection)
{
    if (!owner_->IsServer())
        return 0;
    if (!connection)
        return 0;
    return connection->syncState.get();
}

void SyncManager::RegisterToScene(ScenePtr scene)
{
    // Disconnect from previous scene if not expired
    ScenePtr previous = scene_.lock();
    if (previous)
    {
        disconnect(previous.get(), 0, this, 0);
        server_syncstate_.Clear();
    }
    
    scene_.reset();
    
    if (!scene)
    {
        LogError("Null scene, cannot replicate");
        return;
    }
    
    scene_ = scene;
    Scene* sceneptr = scene.get();
    
    connect(sceneptr, SIGNAL( AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeChanged(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( AttributeAdded(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeAdded(IComponent*, IAttribute*, AttributeChange::Type) ));
    connect(sceneptr, SIGNAL( AttributeRemoved(IComponent*, IAttribute*, AttributeChange::Type) ),
        SLOT( OnAttributeRemoved(IComponent*, IAttribute*, AttributeChange::Type) ));
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
    // std::cout << "Handling message " << id << " size " << numBytes << std::endl;
    try
    {
        switch (id)
        {
        case cCreateEntityMessage:
            HandleCreateEntity(source, data, numBytes);
            break;
        case cCreateComponentsMessage:
            HandleCreateComponents(source, data, numBytes);
            break;
        case cCreateAttributesMessage:
            HandleCreateAttributes(source, data, numBytes);
            break;
        case cEditAttributesMessage:
            HandleEditAttributes(source, data, numBytes);
            break;
        case cRemoveAttributesMessage:
            HandleRemoveAttributes(source, data, numBytes);
            break;
        case cRemoveComponentsMessage:
            HandleRemoveComponents(source, data, numBytes);
            break;
        case cRemoveEntityMessage:
            HandleRemoveEntity(source, data, numBytes);
            break;
        case cCreateEntityReplyMessage:
            HandleCreateEntityReply(source, data, numBytes);
            break;
        case cCreateComponentsReplyMessage:
            HandleCreateComponentsReply(source, data, numBytes);
            break;
        case cEntityActionMessage:
            {
                MsgEntityAction msg(data, numBytes);
                HandleEntityAction(source, msg);
            }
            break;
        }
    }
    catch (kNet::NetException& e)
    {
        LogError("Exception while handling scene sync network message " + QString::number(id) + ": " + QString(e.what()));
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
    user->syncState = boost::shared_ptr<SceneSyncState>(new SceneSyncState(user->GetConnectionID(), owner_->IsServer()));
    user->syncState->SetParentScene(scene_);

    if (owner_->IsServer())
        emit SceneStateCreated(user, user->syncState.get());

    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        if (entity->IsLocal())
            continue;
        entity_id_t id = entity->Id();
        user->syncState->MarkEntityDirty(id);
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
            if ((*i)->syncState) (*i)->syncState->MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
    }
    else
    {
        server_syncstate_.MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
    }
}

void SyncManager::OnAttributeAdded(IComponent* comp, IAttribute* attr, AttributeChange::Type change)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;

    bool isServer = owner_->IsServer();
    
    // We do not allow to create attributes in local or disconnected signaling mode in a replicated component.
    // Always replicate the creation, because the client & server must have their attribute count in sync to 
    // be able to send attribute bitmasks
    if (comp->IsLocal())
        return;
    Entity* entity = comp->ParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    
    if (isServer)
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            if ((*i)->syncState) (*i)->syncState->MarkAttributeCreated(entity->Id(), comp->Id(), attr->Index());
    }
    else
    {
        server_syncstate_.MarkAttributeCreated(entity->Id(), comp->Id(), attr->Index());
    }
}

void SyncManager::OnAttributeRemoved(IComponent* comp, IAttribute* attr, AttributeChange::Type change)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;

    bool isServer = owner_->IsServer();
    
    // We do not allow to remove attributes in local or disconnected signaling mode in a replicated component.
    // Always replicate the removeal, because the client & server must have their attribute count in sync to
    // be able to send attribute bitmasks
    if (comp->IsLocal())
        return;
    Entity* entity = comp->ParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    
    if (isServer)
    {
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            if ((*i)->syncState) (*i)->syncState->MarkAttributeRemoved(entity->Id(), comp->Id(), attr->Index());
    }
    else
    {
        server_syncstate_.MarkAttributeRemoved(entity->Id(), comp->Id(), attr->Index());
    }
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
            if ((*i)->syncState) (*i)->syncState->MarkComponentDirty(entity->Id(), comp->Id());
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
            if ((*i)->syncState) (*i)->syncState->MarkComponentRemoved(entity->Id(), comp->Id());
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
            if ((*i)->syncState)
            {
                (*i)->syncState->MarkEntityDirty(entity->Id());
                if ((*i)->syncState->entities[entity->Id()].removed)
                {
                    LogWarning("An entity with ID " + QString::number(entity->Id()) + " is queued to be deleted, but a new entity \"" + 
                        entity->Name() + "\" is to be added to the scene!");
                }
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
            if ((*i)->syncState) (*i)->syncState->MarkEntityRemoved(entity->Id());
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
    
    updateAcc_ += (float)frametime;
    if (updateAcc_ < updatePeriod_)
        return;
    // If multiple updates passed, update still just once
    while(updateAcc_ >= updatePeriod_)
        updateAcc_ -= updatePeriod_;
    
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;
    
    if (owner_->IsServer())
    {
        // If we are server, process all authenticated users
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            if ((*i)->syncState) ProcessSyncState((*i)->connection, (*i)->syncState.get());
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
    
    unsigned sceneId = 0; ///\todo Replace with proper scene ID once multiscene support is in place.
    
    /*
    static int counter = 0;
    ++counter;
    if (counter >= 30)
    {
        std::cout << "Data in " << destination->BytesInPerSec() / 1024.0f << " kB/s Data out " << destination->BytesOutPerSec() / 1024.0f << " kB/s" << std::endl;
        counter = 0;
    }
    */
    
    ScenePtr scene = scene_.lock();
    int numMessagesSent = 0;
    bool isServer = owner_->IsServer();
    
    // Process the state's dirty entity queue.
    /// \todo Limit and prioritize the data sent. For now the whole queue is processed, regardless of whether the connection is being saturated.
    while (!state->dirtyQueue.empty())
    {
        EntitySyncState& entityState = *state->dirtyQueue.front();
        state->dirtyQueue.pop_front();
        entityState.isInQueue = false;
        
        EntityPtr entity = scene->GetEntity(entityState.id);
        bool removeState = false;
        if (!entity)
        {
            if (!entityState.removed)
                LogWarning("Entity " + QString::number(entityState.id) + " has gone missing from the scene without the remove properly signalled. Removing from replication state");
            entityState.isNew = false;
            removeState = true;
        }
        else
        {
            // Make sure we don't send data for local entities, or unacked entities after the create
            if (entity->IsLocal() || (!entityState.isNew && entity->IsUnacked()))
                continue;
        }
        
        // Remove entity
        if (entityState.removed)
        {
            // If we have both new & removed flags on the entity, it will probably result in buggy behaviour
            if (entityState.isNew)
            {
                LogWarning("Entity " + QString::number(entityState.id) + " queued for both deletion and creation. Buggy behaviour will possibly result!");
                // The delete has been processed. Do not remember it anymore, but requeue the state for creation
                entityState.removed = false;
                removeState = false;
                state->dirtyQueue.push_back(&entityState);
                entityState.isInQueue = true;
            }
            else
                removeState = true;
            
            kNet::DataSerializer ds(removeEntityBuffer_, 1024);
            ds.AddVLE<kNet::VLE8_16_32>(sceneId);
            ds.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
            QueueMessage(destination, cRemoveEntityMessage, true, true, ds);
            ++numMessagesSent;
        }
        // New entity
        else if (entityState.isNew)
        {
            kNet::DataSerializer ds(createEntityBuffer_, 64 * 1024);
            
            // Entity identification and temporary flag
            ds.AddVLE<kNet::VLE8_16_32>(sceneId);
            ds.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
            // Do not write the temporary flag as a bit to not desync the byte alignment at this point, as a lot of data potentially follows
            ds.Add<u8>(entity->IsTemporary() ? 1 : 0);
            
            const Entity::ComponentMap& components = entity->Components();
            // Count the amount of replicated components
            uint numReplicatedComponents = 0;
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
                if (i->second->IsReplicated()) ++numReplicatedComponents;
            ds.AddVLE<kNet::VLE8_16_32>(numReplicatedComponents);
            
            // Serialize each replicated component
            for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
            {
                ComponentPtr comp = i->second;
                if (!comp->IsReplicated())
                    continue;
                WriteComponentFullUpdate(ds, comp);
                // Mark the component undirty in the receiver's syncstate
                state->MarkComponentProcessed(entity->Id(), comp->Id());
            }
            
            QueueMessage(destination, cCreateEntityMessage, true, true, ds);
            ++numMessagesSent;
            
            // The create has been processed fully. Clear dirty flags.
            state->MarkEntityProcessed(entity->Id());
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
                bool removeCompState = false;
                if (!comp)
                {
                    if (!compState.removed)
                        LogWarning("Component " + QString::number(compState.id) + " of " + entity->ToString() + " has gone missing from the scene without the remove properly signalled. Removing from client replication state->");
                    compState.isNew = false;
                    removeCompState = true;
                }
                else
                {
                    // Make sure we don't send data for local components, or unacked components after the create
                    if (comp->IsLocal() || (!compState.isNew && comp->IsUnacked()))
                        continue;
                }
                
                // Remove component
                if (compState.removed)
                {
                    removeCompState = true;
                    
                    // If first component, write the entity ID first
                    if (!removeCompsDs.BytesFilled())
                    {
                        removeCompsDs.AddVLE<kNet::VLE8_16_32>(sceneId);
                        removeCompsDs.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                    }
                    // Then add component ID
                    removeCompsDs.AddVLE<kNet::VLE8_16_32>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                }
                // New component
                else if (compState.isNew)
                {
                    // If first component, write the entity ID first
                    if (!createCompsDs.BytesFilled())
                    {
                        createCompsDs.AddVLE<kNet::VLE8_16_32>(sceneId);
                        createCompsDs.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                    }
                    // Then add the component data
                    WriteComponentFullUpdate(createCompsDs, comp);
                    // Mark the component undirty in the receiver's syncstate
                    state->MarkComponentProcessed(entity->Id(), comp->Id());
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
                                LogError("CreateAttribute for nonexisting attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in " + entity->ToString() + ". Discarding.");
                            else if (!attrs[attrIndex]->IsDynamic())
                                LogError("CreateAttribute for a static attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in " + entity->ToString() + ". Discarding.");
                            else
                            {
                                // If first attribute, write the entity ID first
                                if (!createAttrsDs.BytesFilled())
                                {
                                    createAttrsDs.AddVLE<kNet::VLE8_16_32>(sceneId);
                                    createAttrsDs.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                                }
                                
                                IAttribute* attr = attrs[attrIndex];
                                createAttrsDs.AddVLE<kNet::VLE8_16_32>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
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
                            {
                                removeAttrsDs.AddVLE<kNet::VLE8_16_32>(sceneId);
                                removeAttrsDs.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                            }
                            removeAttrsDs.AddVLE<kNet::VLE8_16_32>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
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
                                        LogError("Attribute change for a nonexisting attribute index " + QString::number(attrIndex) + " was queued for component " + comp->TypeName() + " in " + entity->ToString() + ". Discarding.");
                                }
                            }
                        }
                    }
                    if (changedAttributes_.size())
                    {
                        // If first component for which attribute changes are sent, write the entity ID first
                        if (!editAttrsDs.BytesFilled())
                        {
                            editAttrsDs.AddVLE<kNet::VLE8_16_32>(sceneId);
                            editAttrsDs.AddVLE<kNet::VLE8_16_32>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                        }
                        editAttrsDs.AddVLE<kNet::VLE8_16_32>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                        
                        // Create a nested dataserializer for the actual attribute data, so we can skip components
                        kNet::DataSerializer attrDataDs(attrDataBuffer_, 16 * 1024);
                        
                        // There are changed attributes. Check if it is more optimal to send attribute indices, or the whole bitmask
                        unsigned bitsMethod1 = changedAttributes_.size() * 8 + 8;
                        unsigned bitsMethod2 = attrs.size();
                        // Method 1: indices
                        if (bitsMethod1 <= bitsMethod2)
                        {
                            attrDataDs.Add<kNet::bit>(0);
                            attrDataDs.Add<u8>(changedAttributes_.size());
                            for (unsigned i = 0; i < changedAttributes_.size(); ++i)
                            {
                                attrDataDs.Add<u8>(changedAttributes_[i]);
                                attrs[changedAttributes_[i]]->ToBinary(attrDataDs);
                            }
                        }
                        // Method 2: bitmask
                        else
                        {
                            attrDataDs.Add<kNet::bit>(1);
                            for (unsigned i = 0; i < attrs.size(); ++i)
                            {
                                if (compState.dirtyAttributes[i >> 3] & (1 << (i & 7)))
                                {
                                    attrDataDs.Add<kNet::bit>(1);
                                    attrs[i]->ToBinary(attrDataDs);
                                }
                                else
                                    attrDataDs.Add<kNet::bit>(0);
                            }
                        }
                        
                        // Add the attribute data array to the main serializer
                        editAttrsDs.AddVLE<kNet::VLE8_16_32>(attrDataDs.BytesFilled());
                        editAttrsDs.AddArray<u8>((unsigned char*)attrDataBuffer_, attrDataDs.BytesFilled());
                        
                        // Now zero out all remaining dirty bits
                        for (unsigned i = 0; i < numBytes; ++i)
                            compState.dirtyAttributes[i] = 0;
                    }
                }
                
                if (removeCompState)
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
            state->MarkEntityProcessed(entity->Id());
        }
        
        if (removeState)
            state->entities.erase(entityState.id);
    }
    //if (numMessagesSent)
    //    std::cout << "Sent " << numMessagesSent << " scenesync messages" << std::endl;
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

void SyncManager::HandleCreateEntity(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateEntity message");
        return;
    }

    if (!scene->AllowModifyEntity(user, 0)) //should be 'ModifyScene', but ModifyEntity is now the signal that covers all
        return;

    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    entity_id_t senderEntityID = entityID;
    
    if (!ValidateAction(source, cCreateEntityMessage, entityID))
        return;
    
    // If client gets a entity that already exists, destroy it forcibly
    if (!isServer && scene->GetEntity(entityID))
    {
        LogWarning("Received entity creation from server for entity ID " + QString::number(entityID) + " that already exists. Removing the old entity.");
        scene->RemoveEntity(entityID, AttributeChange::LocalOnly);
    }
    else if (isServer)
    {
        // Server never uses the client's entityID.
        entityID = scene->NextFreeId();
    }
    
    EntityPtr entity = scene->CreateEntity(entityID);
    if (!entity)
    {
        LogWarning("Could not create entity " + QString::number(entityID) + ", disregarding CreateEntity message");
        return;
    }
    
    // Read the temporary flag
    bool temporary = ds.Read<u8>() != 0;
    entity->SetTemporary(temporary);
    
    std::vector<std::pair<component_id_t, component_id_t> > componentIdRewrites;
    // Read the components
    unsigned numComponents = ds.ReadVLE<kNet::VLE8_16_32>();
    for(uint i = 0; i < numComponents; ++i)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        component_id_t senderCompID = compID;
        // If we are server, rewrite the ID
        if (isServer) compID = 0;
        
        u32 typeID = ds.ReadVLE<kNet::VLE8_16_32>();
        QString name = QString::fromStdString(ds.ReadString());
        unsigned attrDataSize = ds.ReadVLE<kNet::VLE8_16_32>();
        ds.ReadArray<u8>((u8*)&attrDataBuffer_[0], attrDataSize);
        kNet::DataDeserializer attrDs(attrDataBuffer_, attrDataSize);
        
        // If client gets a component that already exists, destroy it forcibly
        if (!isServer && entity->GetComponentById(compID))
        {
            LogWarning("Received component creation from server for component ID " + QString::number(compID) + " that already exists in " + entity->ToString() + ". Removing the old component.");
            entity->RemoveComponentById(compID, AttributeChange::LocalOnly);
        }
        
        ComponentPtr comp = entity->CreateComponentWithId(compID, typeID, name, change);
        if (!comp)
        {
            LogWarning("Failed to create component type " + QString::number(compID) + " to " + entity->ToString() + " while handling CreateEntity message, skipping component");
            continue;
        }
        // On server, get the assigned ID now
        if (isServer)
        {
            compID = comp->Id();
            componentIdRewrites.push_back(std::make_pair(senderCompID, compID));
        }
        // Create the component to the sender's syncstate, then mark it processed (undirty)
        state->MarkComponentProcessed(entityID, compID);
        
        // Fill static attributes
        unsigned numStaticAttrs = comp->NumStaticAttributes();
        const AttributeVector& attrs = comp->Attributes();
        for (uint i = 0; i < numStaticAttrs; ++i)
            attrs[i]->FromBinary(attrDs, AttributeChange::Disconnected);
        
        // Create any dynamic attributes
        while (attrDs.BitsLeft() > 2 * 8)
        {
            u8 index = attrDs.Read<u8>();
            u8 typeId = attrDs.Read<u8>();
            QString name = QString::fromStdString(attrDs.ReadString());
            IAttribute* newAttr = comp->CreateAttribute(index, typeId, name, change);
            if (!newAttr)
            {
                LogWarning("Failed to create dynamic attribute. Skipping rest of the attributes for this component.");
                break;
            }
            newAttr->FromBinary(attrDs, AttributeChange::Disconnected);
        }
    }
    
    // Emit the component changes last, to signal only a coherent state of the whole entity
    scene->EmitEntityCreated(entity.get(), change);
    const Entity::ComponentMap &components = entity->Components();
    for (Entity::ComponentMap::const_iterator i = components.begin(); i != components.end(); ++i)
        i->second->ComponentChanged(change);
    
    // Send CreateEntityReply (server only)
    if (isServer)
    {
        kNet::DataSerializer replyDs(createEntityBuffer_, 64 * 1024);
        replyDs.AddVLE<kNet::VLE8_16_32>(sceneID);
        replyDs.AddVLE<kNet::VLE8_16_32>(senderEntityID & UniqueIdGenerator::LAST_REPLICATED_ID);
        replyDs.AddVLE<kNet::VLE8_16_32>(entityID & UniqueIdGenerator::LAST_REPLICATED_ID);
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites.size());
        for (unsigned i = 0; i < componentIdRewrites.size(); ++i)
        {
            replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].first & UniqueIdGenerator::LAST_REPLICATED_ID);
            replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].second & UniqueIdGenerator::LAST_REPLICATED_ID);
        }
        QueueMessage(source, cCreateEntityReplyMessage, true, true, replyDs);
    }
    
    // Mark the entity processed (undirty) in the sender's syncstate so that create is not echoed back
    state->MarkEntityProcessed(entityID);
}

void SyncManager::HandleCreateComponents(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateComponents message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cCreateComponentsMessage, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for CreateComponents message");
        return;
    }

    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!scene->AllowModifyEntity(user, entity.get()))
        return;
    
    // Read the components
    std::vector<std::pair<component_id_t, component_id_t> > componentIdRewrites;
    std::vector<ComponentPtr> addedComponents;
    while (ds.BitsLeft() > 2 * 8)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        component_id_t senderCompID = compID;
        // If we are server, rewrite the ID
        if (isServer) compID = 0;
        
        u32 typeID = ds.ReadVLE<kNet::VLE8_16_32>();
        QString name = QString::fromStdString(ds.ReadString());
        unsigned attrDataSize = ds.ReadVLE<kNet::VLE8_16_32>();
        ds.ReadArray<u8>((u8*)&attrDataBuffer_[0], attrDataSize);
        kNet::DataDeserializer attrDs(attrDataBuffer_, attrDataSize);
        
        // If client gets a component that already exists, destroy it forcibly
        if (!isServer && entity->GetComponentById(compID))
        {
            LogWarning("Received component creation from server for component ID " + QString::number(compID) + " that already exists in " + entity->ToString() + ". Removing the old component.");
            entity->RemoveComponentById(compID, AttributeChange::LocalOnly);
        }
        
        ComponentPtr comp = entity->CreateComponentWithId(compID, typeID, name, change);
        if (!comp)
        {
            LogWarning("Failed to create component type " + QString::number(compID) + " to " + entity->ToString() + " while handling CreateComponents message, skipping component");
            continue;
        }
        // On server, get the assigned ID now
        if (isServer)
        {
            compID = comp->Id();
            componentIdRewrites.push_back(std::make_pair(senderCompID, compID));
        }
        
        // Create the component to the sender's syncstate, then mark it processed (undirty)
        state->MarkComponentProcessed(entityID, compID);
        
        addedComponents.push_back(comp);
        
        // Fill static attributes
        unsigned numStaticAttrs = comp->NumStaticAttributes();
        const AttributeVector& attrs = comp->Attributes();
        for (uint i = 0; i < numStaticAttrs; ++i)
            attrs[i]->FromBinary(attrDs, AttributeChange::Disconnected);
        
        // Create any dynamic attributes
        while (attrDs.BitsLeft() > 2 * 8)
        {
            u8 index = attrDs.Read<u8>();
            u8 typeId = attrDs.Read<u8>();
            QString name = QString::fromStdString(attrDs.ReadString());
            IAttribute* newAttr = comp->CreateAttribute(index, typeId, name, change);
            if (!newAttr)
            {
                LogWarning("Failed to create dynamic attribute. Skipping rest of the attributes for this component.");
                break;
            }
            newAttr->FromBinary(attrDs, AttributeChange::Disconnected);
        }
    }
    
    // Send CreateComponentsReply (server only)
    if (isServer)
    {
        kNet::DataSerializer replyDs(createEntityBuffer_, 64 * 1024);
        replyDs.AddVLE<kNet::VLE8_16_32>(sceneID);
        replyDs.AddVLE<kNet::VLE8_16_32>(entityID & UniqueIdGenerator::LAST_REPLICATED_ID);
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites.size());
        for (unsigned i = 0; i < componentIdRewrites.size(); ++i)
        {
            replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].first & UniqueIdGenerator::LAST_REPLICATED_ID);
            replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].second & UniqueIdGenerator::LAST_REPLICATED_ID);
        }
        QueueMessage(source, cCreateComponentsReplyMessage, true, true, replyDs);
    }
    
    // Emit the component changes last, to signal only a coherent state of the whole entity
    for (unsigned i = 0; i < addedComponents.size(); ++i)
        addedComponents[i]->ComponentChanged(change);
}

void SyncManager::HandleRemoveEntity(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveEntity message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveEntityMessage, entityID))
        return;
    
    if (!scene->GetEntity(entityID))
    {
        LogWarning("Missing entity " + QString::number(entityID) + " for RemoveEntity message");
        return;
    }
    
    scene->RemoveEntity(entityID, change);
    // Delete from the sender's syncstate so that we don't echo the delete back needlessly
    state->RemoveFromQueue(entityID); // Be sure to erase from dirty queue so that we don't invoke UDB
    state->entities.erase(entityID);
}

void SyncManager::HandleRemoveComponents(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveComponents message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveComponentsMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for RemoveComponents message");
        return;
    }
    
    while (ds.BitsLeft() >= 8)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        ComponentPtr comp = entity->GetComponentById(compID);
        if (!comp)
        {
            LogWarning("Component id " + QString::number(compID) + " not found in " + entity->ToString() + " for RemoveComponents message, disregarding");
            continue;
        }
        entity->RemoveComponent(comp, change);
        // Delete from the sender's syncstate, so that we don't echo the delete back needlessly
        if (state->entities.find(entityID) != state->entities.end())
        {
            state->entities[entityID].RemoveFromQueue(compID); // Be sure to erase from dirty queue so that we don't invoke UDB
            state->entities[entityID].components.erase(compID);
        }
    }
}

void SyncManager::HandleCreateAttributes(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateAttributes message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cCreateAttributesMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for CreateAttributes message");
        return;
    }

    if (!scene->AllowModifyEntity(user, 0)) //to check if creating entities is allowed (for this user)
        return;

    std::vector<IAttribute*> addedAttrs;
    while (ds.BitsLeft() >= 3 * 8)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        ComponentPtr comp = entity->GetComponentById(compID);
        if (!comp)
        {
            LogWarning("Component id " + QString::number(compID) + " not found in " + entity->ToString() + " for CreateAttributes message, aborting message parsing");
            return;
        }
        
        u8 attrIndex = ds.Read<u8>();
        u8 typeId = ds.Read<u8>();
        QString name = QString::fromStdString(ds.ReadString());
        
        if (isServer)
        {
            // If we are server, do not allow to overwrite existing attributes by client requests
            const AttributeVector& existingAttrs = comp->Attributes();
            if (attrIndex < existingAttrs.size() && existingAttrs[attrIndex])
            {
                LogWarning("Client attempted to overwrite an existing attribute index " + QString::number(attrIndex) + " in component " + comp->TypeName() + " in " + entity->ToString() + ", aborting CreateAttributes message parsing");
                return;
            }
        }
        
        IAttribute* attr = comp->CreateAttribute(attrIndex, typeId, name, change);
        if (!attr)
        {
            LogWarning("Could not create attribute into component " + comp->TypeName() + " in " + entity->ToString() + ", aborting CreateAttributes message parsing");
            return;
        }
        
        addedAttrs.push_back(attr);
        attr->FromBinary(ds, AttributeChange::Disconnected);
        
        // Remove the corresponding add command from the sender's syncstate, so that the attribute add is not echoed back
        state->entities[entityID].components[compID].newAndRemovedAttributes.erase(attrIndex);
    }
    
    // Signal attribute changes after creating and reading all
    for (unsigned i = 0; i < addedAttrs.size(); ++i)
    {
        IComponent* owner = addedAttrs[i]->Owner();
        u8 attrIndex = addedAttrs[i]->Index();
        owner->EmitAttributeChanged(addedAttrs[i], change);
        // Remove the dirty bit from sender's syncstate so that we do not echo the change back
        state->entities[entityID].components[owner->Id()].dirtyAttributes[attrIndex >> 3] &= ~(1 << (attrIndex & 7));
    }
}

void SyncManager::HandleRemoveAttributes(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveAttributes message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for RemoveAttributes message");
        return;
    }
    
    while (ds.BitsLeft() >= 8)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        u8 attrIndex = ds.Read<u8>();
        
        ComponentPtr comp = entity->GetComponentById(compID);
        if (!comp)
        {
            LogWarning("Component id " + QString::number(compID) + " not found in " + entity->ToString() + " for RemoveAttributes message");
            continue;
        }
        
        comp->RemoveAttribute(attrIndex, change);
        // Remove the corresponding remove command from the sender's syncstate, so that the attribute remove is not echoed back
        state->entities[entityID].components[compID].newAndRemovedAttributes.erase(attrIndex);
    }
}

void SyncManager::HandleEditAttributes(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding EditAttributes message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = isServer ? AttributeChange::Replicate : AttributeChange::LocalOnly;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
    
    // Record the update time for calculating the update interval
    float updateInterval = updatePeriod_; // Default update interval if state not found or interval not measured yet
    std::map<entity_id_t, EntitySyncState>::iterator it = state->entities.find(entityID);
    if (it != state->entities.end())
    {
        it->second.UpdateReceived();
        if (it->second.avgUpdateInterval > 0.0f)
            updateInterval = it->second.avgUpdateInterval;
    }
    // Add a fudge factor in case there is jitter in packet receipt or the server is too taxed
    updateInterval *= 1.25f;
    
    EntityPtr entity = scene->GetEntity(entityID);
    UserConnection* user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for EditAttributes message");
        return;
    }
    
    if (!scene->AllowModifyEntity(user, 0)) //to check if creating entities is allowed (for this user)
        return;

    std::vector<IAttribute*> changedAttrs;
    while (ds.BitsLeft() >= 8)
    {
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        unsigned attrDataSize = ds.ReadVLE<kNet::VLE8_16_32>();
        ds.ReadArray<u8>((u8*)&attrDataBuffer_[0], attrDataSize);
        kNet::DataDeserializer attrDs(attrDataBuffer_, attrDataSize);

        ComponentPtr comp = entity->GetComponentById(compID);
        if (!comp)
        {
            LogWarning("Component id " + QString::number(compID) + " not found in " + entity->ToString() + " for EditAttributes message, skipping to next component");
            continue;
        }
        const AttributeVector& attributes = comp->Attributes();

        int indexingMethod = attrDs.Read<kNet::bit>();
        if (!indexingMethod)
        {
            // Method 1: indices
            u8 numChangedAttrs = attrDs.Read<u8>();
            for (unsigned i = 0; i < numChangedAttrs; ++i)
            {
                u8 attrIndex = attrDs.Read<u8>();
                if (attrIndex >= attributes.size())
                {
                    LogWarning("Out of bounds attribute index in EditAttributes message, skipping to next component");
                    break;
                }
                IAttribute* attr = attributes[attrIndex];
                if (!attr)
                {
                    LogWarning("Nonexistent attribute in EditAttributes message, skipping to next component");
                    break;
                }
                
                bool interpolate = (!isServer && attr->Metadata() && attr->Metadata()->interpolation == AttributeMetadata::Interpolate);
                if (!interpolate)
                {
                    attr->FromBinary(attrDs, AttributeChange::Disconnected);
                    changedAttrs.push_back(attr);
                }
                else
                {
                    IAttribute* endValue = attr->Clone();
                    endValue->FromBinary(attrDs, AttributeChange::Disconnected);
                    scene->StartAttributeInterpolation(attr, endValue, updateInterval);
                }
            }
        }
        else
        {
            // Method 2: bitmask
            for (unsigned i = 0; i < attributes.size(); ++i)
            {
                int changed = attrDs.Read<kNet::bit>();
                if (changed)
                {
                    IAttribute* attr = attributes[i];
                    if (!attr)
                    {
                        LogWarning("Nonexistent attribute in EditAttributes message, skipping to next component");
                        break;
                    }
                    bool interpolate = (!isServer && attr->Metadata() && attr->Metadata()->interpolation == AttributeMetadata::Interpolate);
                    if (!interpolate)
                    {
                        attr->FromBinary(attrDs, AttributeChange::Disconnected);
                        changedAttrs.push_back(attr);
                    }
                    else
                    {
                        IAttribute* endValue = attr->Clone();
                        endValue->FromBinary(attrDs, AttributeChange::Disconnected);
                        scene->StartAttributeInterpolation(attr, endValue, updateInterval);
                    }
                }
            }
        }
    }
    
    // Signal attribute changes after reading all
    for (unsigned i = 0; i < changedAttrs.size(); ++i)
    {
        IComponent* owner = changedAttrs[i]->Owner();
        u8 attrIndex = changedAttrs[i]->Index();
        owner->EmitAttributeChanged(changedAttrs[i], change);
        // Remove the dirty bit from sender's syncstate so that we do not echo the change back
        state->entities[entityID].components[owner->Id()].dirtyAttributes[attrIndex >> 3] &= ~(1 << (attrIndex & 7));
    }
}

void SyncManager::HandleCreateEntityReply(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateEntityReply message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    if (isServer)
    {
        LogWarning("Discarding CreateEntityReply message on server");
        return;
    }
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t senderEntityID = ds.ReadVLE<kNet::VLE8_16_32>() | UniqueIdGenerator::FIRST_UNACKED_ID;
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    scene->ChangeEntityId(senderEntityID, entityID);
    state->RemoveFromQueue(senderEntityID); // Make sure we don't have stale pointers in the dirty queue
    state->entities[entityID] = state->entities[senderEntityID]; // Copy the sync state to the new ID
    state->entities[entityID].id = entityID; // Must remember to change ID manually
    state->entities.erase(senderEntityID);
    
    //std::cout << "CreateEntityReply, entity " << senderEntityID << " -> " << entityID << std::endl;
    
    EntitySyncState& entityState = state->entities[entityID];
    
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogError("Failed to get entity after ID change");
        return;
    }
    
    unsigned numComps = ds.ReadVLE<kNet::VLE8_16_32>();
    for (unsigned i = 0; i < numComps; ++i)
    {
        component_id_t senderCompID = ds.ReadVLE<kNet::VLE8_16_32>() | UniqueIdGenerator::FIRST_UNACKED_ID;
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        
        //std::cout << "CreateEntityReply, component " << senderCompID << " -> " << compID << std::endl;
        
        entity->ChangeComponentId(senderCompID, compID);
        entityState.components[compID] = entityState.components[senderCompID]; // Copy the sync state to the new ID
        entityState.components[compID].id = compID; // Must remember to change ID manually
        entityState.components.erase(senderCompID);
        
        // Send notification
        IComponent* comp = entity->GetComponentById(compID).get();
        scene->EmitComponentAcked(comp, senderCompID);
    }
    
    // Send notification
    scene->EmitEntityAcked(entity.get(), senderEntityID);
    
    for (std::map<component_id_t, ComponentSyncState>::iterator i = entityState.components.begin(); i != entityState.components.end(); ++i)
    {
        // Now mark every component dirty so they will be inspected for changes on the next update
        state->MarkComponentDirty(entityID, i->first);
    }
}

void SyncManager::HandleCreateComponentsReply(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateComponentsReply message");
        return;
    }
    
    bool isServer = owner_->IsServer();
    if (isServer)
    {
        LogWarning("Discarding CreateComponentsReply message on server");
        return;
    }
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    state->RemoveFromQueue(entityID); // Make sure we don't have stale pointers in the dirty queue
    EntitySyncState& entityState = state->entities[entityID];
    
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogError("Failed to get entity after ID change");
        return;
    }
    
    unsigned numComps = ds.ReadVLE<kNet::VLE8_16_32>();
    for (unsigned i = 0; i < numComps; ++i)
    {
        component_id_t senderCompID = ds.ReadVLE<kNet::VLE8_16_32>() | UniqueIdGenerator::FIRST_UNACKED_ID;
        component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
        
        //std::cout << "CreateComponentReply, component " << senderCompID << " -> " << compID << std::endl;
        
        entity->ChangeComponentId(senderCompID, compID);
        entityState.components[compID] = entityState.components[senderCompID]; // Copy the sync state to the new ID
        entityState.components[compID].id = compID; // Must remember to change ID manually
        entityState.components.erase(senderCompID);
        
        // Send notification
        IComponent* comp = entity->GetComponentById(compID).get();
        scene->EmitComponentAcked(comp, senderCompID);
    }
    
    for (std::map<component_id_t, ComponentSyncState>::iterator i = entityState.components.begin(); i != entityState.components.end(); ++i)
    {
        // Now mark every component dirty so they will be inspected for changes on the next update
        state->MarkComponentDirty(entityID, i->first);
    }
}

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

    EntityAction::ExecTypeField type = (EntityAction::ExecTypeField)(msg.executionType);

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
            return (*i)->syncState.get();
    }
    return 0;
}

}
