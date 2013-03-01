// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "KristalliProtocolModule.h"
#include "SyncManager.h"
#include "TundraLogicModule.h"
#include "Client.h"
#include "Server.h"
#include "TundraMessages.h"
#include "MsgEntityAction.h"

#include "Scene/Scene.h"
#include "Entity.h"
#include "CoreStringUtils.h"
#include "EC_DynamicComponent.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "Profiler.h"
#include "EC_Placeable.h"
#include "EC_RigidBody.h"
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
    kNet::NetworkMessage* msg = connection->StartNewMessage(id, ds.BytesFilled());
    memcpy(msg->data, ds.GetData(), ds.BytesFilled());
    msg->reliable = reliable;
    msg->inOrder = inOrder;
    msg->priority = 100; // Fixed priority as in those defined with xml
    connection->EndAndQueueMessage(msg);
}

void SyncManager::WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp)
{
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
    updatePeriod_(1.0f / 20.0f),
    updateAcc_(0.0),
    maxLinExtrapTime_(3.0f),
    noClientPhysicsHandoff_(false)
{
    KristalliProtocolModule *kristalli = framework_->GetModule<KristalliProtocolModule>();
    connect(kristalli, SIGNAL(NetworkMessageReceived(kNet::MessageConnection *, kNet::packet_id_t, kNet::message_id_t, const char *, size_t)), 
        this, SLOT(HandleKristalliMessage(kNet::MessageConnection*, kNet::packet_id_t, kNet::message_id_t, const char*, size_t)));
    
    if (framework_->HasCommandLineParameter("--noclientphysics"))
        noClientPhysicsHandoff_ = true;
    
    GetClientExtrapolationTime();
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
    
    GetClientExtrapolationTime();
}

void SyncManager::GetClientExtrapolationTime()
{
    QStringList extrapTimeParam = framework_->CommandLineParameters("--clientextrapolationtime");
    if (extrapTimeParam.size() > 0)
    {
        bool ok;
        float newExtrapTime = extrapTimeParam.first().toFloat(&ok);
        if (ok && newExtrapTime >= 0.0f)
        {
            // First update period is always interpolation, and extrapolation time is in addition to that
            maxLinExtrapTime_ = 1.0f + newExtrapTime / 1000.0f / updatePeriod_;
        }
    }
}

SceneSyncState* SyncManager::SceneState(int connectionId) const
{
    if (!owner_->IsServer())
        return 0;
    return SceneState(owner_->GetServer()->GetUserConnection(connectionId));
}

SceneSyncState* SyncManager::SceneState(const UserConnectionPtr &connection) const
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
        LogError("SyncManager::RegisterToScene: Null scene, cannot replicate");
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

void SyncManager::HandleKristalliMessage(kNet::MessageConnection* source, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char* data, size_t numBytes)
{
    try
    {
        switch(messageId)
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
        case cRigidBodyUpdateMessage:
            HandleRigidBodyChanges(source, packetId, data, numBytes);
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
        LogError("Exception while handling scene sync network message " + QString::number(messageId) + ": " + QString(e.what()));
        throw; // Propagate the message so that Tundra server will kill the connection (if we are the server).
    }
    currentSender = 0;
}

void SyncManager::NewUserConnected(const UserConnectionPtr &user)
{
    PROFILE(SyncManager_NewUserConnected);

    ScenePtr scene = scene_.lock();
    if (!scene)
    {
        LogWarning("SyncManager: Cannot handle new user connection message - No scene set!");
        return;
    }
    
    // Connect to actions sent to specifically to this user
    connect(user.get(), SIGNAL(ActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)),
        this, SLOT(OnUserActionTriggered(UserConnection*, Entity*, const QString&, const QStringList&)));
    
    // Mark all entities in the sync state as new so we will send them
    user->syncState = MAKE_SHARED(SceneSyncState, user->ConnectionId(), owner_->IsServer());
    user->syncState->SetParentScene(scene_);

    if (owner_->IsServer())
        emit SceneStateCreated(user.get(), user->syncState.get());

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
    if (!isServer) // Since the server never interpolates attributes, we don't need to do this check on the server at all.
    {
        ScenePtr scene = scene_.lock();
        if (scene && !scene->IsInterpolating() && !currentSender)
        {
            if (attr->Metadata() && attr->Metadata()->interpolation == AttributeMetadata::Interpolate)
                // Note: it does not matter if the attribute was not actually interpolating
                scene->EndAttributeInterpolation(attr);
        }
    }
    
    // Is this change even supposed to go to the network?
    if (change != AttributeChange::Replicate || comp->IsLocal())
        return;

    Entity* entity = comp->ParentEntity();
    if (!entity || entity->IsLocal())
        return; // This is a local entity, don't take it to network.
    
    if (isServer)
    {
        // For each client connected to this server, mark this attribute dirty, so it will be updated to the
        // clients on the next network sync iteration.
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            if ((*i)->syncState)
                (*i)->syncState->MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
    }
    else
    {
        // As a client, mark the attribute dirty so we will push the new value to server on the next
        // network sync iteration.
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
    // If we are the server and the local script on this machine has requested a script to be executed on the server, it
    // means we just execute the action locally here, without sending to network.
    bool isServer = owner_->IsServer();
    if (isServer && (type & EntityAction::Server) != 0)
        entity->Exec(EntityAction::Local, action, params);

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
        msg.executionType = (u8)(type & ~EntityAction::Local);
        owner_->GetClient()->GetConnection()->Send(msg);
    }

    if (isServer && (type & EntityAction::Peers) != 0)
    {
        msg.executionType = (u8)EntityAction::Local; // Propagate as local actions.
        foreach(UserConnectionPtr c, owner_->GetKristalliModule()->GetUserConnections())
        {
            if (c->properties["authenticated"] == "true" && c->connection)
                c->connection->Send(msg);
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

/// Interpolates from (pos0, vel0) to (pos1, vel1) with a C1 curve (continuous in position and velocity)
float3 HermiteInterpolate(const float3 &pos0, const float3 &vel0, const float3 &pos1, const float3 &vel1, float t)
{
    float tt = t*t;
    float ttt = tt*t;
    float h1 = 2*ttt - 3*tt + 1;
    float h2 = 1 - h1;
    float h3 = ttt - 2*tt + t;
    float h4 = ttt - tt;

    return h1 * pos0 + h2 * pos1 + h3 * vel0 + h4 * vel1;
}

/// Returns the tangent vector (derivative) of the Hermite curve. Note that the differential is w.r.t. timesteps along the curve from t=[0,1] 
/// and not in "wallclock" time.
float3 HermiteDerivative(const float3 &pos0, const float3 &vel0, const float3 &pos1, const float3 &vel1, float t)
{
    float tt = t*t;
    float h1 = 6*(tt - t);
    float h2 = -h1;
    float h3 = 3*tt - 4*t + 1;
    float h4 = 3*tt - 2*t;

    return h1 * pos0 + h2 * pos1 + h3 * vel0 + h4 * vel1;
}

void SyncManager::InterpolateRigidBodies(f64 frametime, SceneSyncState* state)
{
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;

    for(std::map<entity_id_t, RigidBodyInterpolationState>::iterator iter = state->entityInterpolations.begin(); 
        iter != state->entityInterpolations.end();)
    {
        EntityPtr e = scene->GetEntity(iter->first);
        shared_ptr<EC_Placeable> placeable = e ? e->GetComponent<EC_Placeable>() : shared_ptr<EC_Placeable>();
        if (!placeable.get())
        {
            std::map<entity_id_t, RigidBodyInterpolationState>::iterator del = iter++;
            state->entityInterpolations.erase(del);
            continue;
        }

        shared_ptr<EC_RigidBody> rigidBody = e->GetComponent<EC_RigidBody>();

        RigidBodyInterpolationState &r = iter->second;
        if (!r.interpolatorActive)
        {
            ++iter;
            continue;
        }

        const float interpPeriod = updatePeriod_; // Time in seconds how long interpolating the Hermite spline from [0,1] should take.

        // Test: Uncomment to only interpolate.
//        r.interpTime = std::min(1.0f, r.interpTime + (float)frametime / interpPeriod);
        r.interpTime += (float)frametime / interpPeriod;

        // Objects without a rigidbody, or with mass 0 never extrapolate (objects with mass 0 are stationary for Bullet).
        const bool isNewtonian = rigidBody && rigidBody->mass.Get() > 0;

        float3 pos;
        if (r.interpTime < 1.0f) // Interpolating between two messages from server.
        {
            if (isNewtonian)
                pos = HermiteInterpolate(r.interpStart.pos, r.interpStart.vel * interpPeriod, r.interpEnd.pos, r.interpEnd.vel * interpPeriod, r.interpTime);
            else
                pos = HermiteInterpolate(r.interpStart.pos, float3::zero, r.interpEnd.pos, float3::zero, r.interpTime);
        }
        else // Linear extrapolation if server has not sent an update.
        {
            if (isNewtonian && maxLinExtrapTime_ > 1.0f)
                pos = r.interpEnd.pos + r.interpEnd.vel * (r.interpTime-1.f) * interpPeriod;
            else
                pos = r.interpEnd.pos;
        }
        ///\todo Orientation is only interpolated, and capped to end result. Also extrapolate orientation.
        Quat rot = Quat::Slerp(r.interpStart.rot, r.interpEnd.rot, Clamp01(r.interpTime));
        float3 scale = float3::Lerp(r.interpStart.scale, r.interpEnd.scale, Clamp01(r.interpTime));
        
        Transform t;
        t.SetPos(pos);
        t.SetOrientation(rot);
        t.SetScale(scale);
        placeable->transform.Set(t, AttributeChange::LocalOnly);

        // Local simulation steps:
        // One fixed update interval: interpolate
        // Two subsequent update intervals: linear extrapolation
        // All subsequente update intervals: local physics extrapolation.
        if (r.interpTime >= maxLinExtrapTime_) // Hand-off to client-side physics?
        {
            if (rigidBody)
            {
                if (!noClientPhysicsHandoff_)
                {
                    bool objectIsInRest = (r.interpEnd.vel.LengthSq() < 1e-4f && r.interpEnd.angVel.LengthSq() < 1e-4f);
                    // Now the local client-side physics will take over the simulation of this rigid body, but only if the object
                    // is moving. This is because the client shouldn't wake up the object (locally) if it's stationary, but wait for the
                    // server-side signal for that event.
                    rigidBody->SetClientExtrapolating(objectIsInRest == false);
                    // Give starting parameters for the simulation.
                    rigidBody->linearVelocity.Set(r.interpEnd.vel, AttributeChange::LocalOnly);
                    rigidBody->angularVelocity.Set(r.interpEnd.angVel, AttributeChange::LocalOnly);
                }
            }
            r.interpolatorActive = false;
            ++iter;
            
            // Could remove the interpolation structure here, as inter/extrapolation it is no longer active. However, it is currently
            // used to store most recently received entity position  & velocity data.
            //iter = state->entityInterpolations.erase(iter); // Finished interpolation.
        }
        else // Interpolation or linear extrapolation.
        {
            if (rigidBody)
            {
                // Ensure that the local side physics is not driving the position of this entity.
                rigidBody->SetClientExtrapolating(false);

                // Setting these is rather redundant, since Bullet doesn't simulate the entity using these variables. However, other
                // (locally simulated) objects can collide to this entity, in which case it's good to have the proper velocities for bullet,
                // so that the collision response simulates the appropriate forces/velocities in play.
                float3 curVel = float3::Lerp(r.interpStart.vel, r.interpEnd.vel, Clamp01(r.interpTime));
                // Test: To set continous velocity based on the Hermite curve, use the following:
 //               float3 curVel = HermiteDerivative(r.interpStart.pos, r.interpStart.vel*interpPeriod, r.interpEnd.pos, r.interpEnd.vel*interpPeriod, r.interpTime);

                rigidBody->linearVelocity.Set(curVel, AttributeChange::LocalOnly);

                ///\todo Setup angular velocity.
                rigidBody->angularVelocity.Set(float3::zero, AttributeChange::LocalOnly);
            }
            ++iter;
        }
    }
}

void SyncManager::Update(f64 frametime)
{
    PROFILE(SyncManager_Update);

    // For the client, smoothly update all rigid bodies by interpolating.
    if (!owner_->IsServer())
        InterpolateRigidBodies(frametime, &server_syncstate_);

    // Check if it is yet time to perform a network update tick.
    updateAcc_ += (float)frametime;
    if (updateAcc_ < updatePeriod_)
        return;

    // If multiple updates passed, update still just once.
    updateAcc_ = fmod(updateAcc_, updatePeriod_);
    
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;
    
    if (owner_->IsServer())
    {
        // If we are server, process all authenticated users

        // Then send out changes to other attributes via the generic sync mechanism.
        UserConnectionList& users = owner_->GetKristalliModule()->GetUserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
            if ((*i)->syncState)
            {
                // First send out all changes to rigid bodies.
                // After processing this function, the bits related to rigid body states have been cleared,
                // so the generic sync will not double-replicate the rigid body positions and velocities.
                ReplicateRigidBodyChanges((*i)->connection, (*i)->syncState.get());

                ProcessSyncState((*i)->connection, (*i)->syncState.get());
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

void SyncManager::ReplicateRigidBodyChanges(kNet::MessageConnection* destination, SceneSyncState* state)
{
    PROFILE(SyncManager_ReplicateRigidBodyChanges);
    
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;

    const int maxMessageSizeBytes = 1400;
    kNet::NetworkMessage *msg = destination->StartNewMessage(cRigidBodyUpdateMessage, maxMessageSizeBytes);
    msg->contentID = 0;
    msg->inOrder = true;
    msg->reliable = false;
    kNet::DataSerializer ds(msg->data, maxMessageSizeBytes);

    for(std::list<EntitySyncState*>::iterator iter = state->dirtyQueue.begin(); iter != state->dirtyQueue.end(); ++iter)
    {
        const int maxRigidBodyMessageSizeBits = 350; // An update for a single rigid body can take at most this many bits. (conservative bound)
        // If we filled up this message, send it out and start crafting anothero one.
        if (maxMessageSizeBytes * 8 - (int)ds.BitsFilled() <= maxRigidBodyMessageSizeBits)
        {
            destination->EndAndQueueMessage(msg, ds.BytesFilled());
            msg = destination->StartNewMessage(cRigidBodyUpdateMessage, maxMessageSizeBytes);
            ds = kNet::DataSerializer(msg->data, maxMessageSizeBytes);
        }
        EntitySyncState &ess = **iter;

        if (ess.isNew || ess.removed)
            continue; // Newly created and removed entities are handled through the traditional sync mechanism.

        EntityPtr e = scene->GetEntity(ess.id);
        shared_ptr<EC_Placeable> placeable = e->GetComponent<EC_Placeable>();
        if (!placeable.get())
            continue;

        std::map<component_id_t, ComponentSyncState>::iterator placeableComp = ess.components.find(placeable->Id());

        bool transformDirty = false;
        if (placeableComp != ess.components.end())
        {
            ComponentSyncState &pss = placeableComp->second;
            if (!pss.isNew && !pss.removed) // Newly created and deleted components are handled through the traditional sync mechanism.
            {
                transformDirty = (pss.dirtyAttributes[0] & 1) != 0; // The Transform of an EC_Placeable is the first attibute in the component.
                pss.dirtyAttributes[0] &= ~1;
            }
        }
        bool velocityDirty = false;
        bool angularVelocityDirty = false;
        
        shared_ptr<EC_RigidBody> rigidBody = e->GetComponent<EC_RigidBody>();
        if (rigidBody)
        {
            std::map<component_id_t, ComponentSyncState>::iterator rigidBodyComp = ess.components.find(rigidBody->Id());
            if (rigidBodyComp != ess.components.end())
            {
                ComponentSyncState &rss = rigidBodyComp->second;
                if (!rss.isNew && !rss.removed) // Newly created and deleted components are handled through the traditional sync mechanism.
                {
                    velocityDirty = (rss.dirtyAttributes[1] & (1 << 5)) != 0;
                    angularVelocityDirty = (rss.dirtyAttributes[1] & (1 << 6)) != 0;

                    rss.dirtyAttributes[1] &= ~(1 << 5);
                    rss.dirtyAttributes[1] &= ~(1 << 6);

                    velocityDirty = velocityDirty && (rigidBody->linearVelocity.Get().DistanceSq(ess.linearVelocity) >= 1e-2f);
                    angularVelocityDirty = angularVelocityDirty && (rigidBody->angularVelocity.Get().DistanceSq(ess.angularVelocity) >= 1e-1f);

                    // If the object enters rest, force an update, and force the update to be sent as reliable, so that the client
                    // is guaranteed to receive the message, and will put the object to rest, instead of extrapolating it away indefinitely.
                    if (rigidBody->linearVelocity.Get().IsZero(1e-4f) && !ess.linearVelocity.IsZero(1e-4f))
                    {
                        velocityDirty = true;
                        msg->reliable = true;
                    }
                    if (rigidBody->angularVelocity.Get().IsZero(1e-4f) && !ess.angularVelocity.IsZero(1e-4f))
                    {
                        angularVelocityDirty = true;
                        msg->reliable = true;
                    }
                }
            }
        }

        if (!transformDirty && !velocityDirty && !angularVelocityDirty)
            continue;

        const Transform &t = placeable->transform.Get();

        float timeSinceLastSend = kNet::Clock::SecondsSinceF(ess.lastNetworkSendTime);
        const float3 predictedClientSidePosition = ess.transform.pos + timeSinceLastSend * ess.linearVelocity;
        float error = t.pos.DistanceSq(predictedClientSidePosition);
        UNREFERENCED_PARAM(error)
        // TEST: To have the server estimate how far the client has simulated, use this.
//        bool posChanged = transformDirty && (timeSinceLastSend > 0.2f || t.pos.DistanceSq(/*ess.transform.pos*/ predictedClientSidePosition) > 5e-5f);
        bool posChanged = transformDirty && t.pos.DistanceSq(ess.transform.pos) > 1e-3f;
        bool rotChanged = transformDirty && (t.rot.DistanceSq(ess.transform.rot) > 1e-1f);
        bool scaleChanged = transformDirty && (t.scale.DistanceSq(ess.transform.scale) > 1e-3f);

        // Detect whether to send compact or full states for each variable.
        // 0 - don't send, 1 - send compact, 2 - send full.
        int posSendType = posChanged ? (t.pos.Abs().MaxElement() >= 1023.f ? 2 : 1) : 0;
        int rotSendType;
        int scaleSendType;
        int velSendType;
        int angVelSendType;

        float3x3 rot;
        if (rotChanged)
        {
            rot = t.Orientation3x3();
            float3 fwd = rot.Col(2);
            float3 up = rot.Col(1);
            float3 planeNormal = float3::unitY.Cross(rot.Col(2));
            float d = planeNormal.Dot(rot.Col(1));

            if (up.Dot(float3::unitY) >= 0.999f)
                rotSendType = 1; // Looking upright, 1 DOF.
            else if (Abs(d) <= 0.001f && Abs(fwd.Dot(float3::unitY)) < 0.95f && up.Dot(float3::unitY) > 0.f)
                rotSendType = 2; // No roll, i.e. 2 DOF. Use this only if not looking too close towards the +Y axis, due to precision issues, and only when object +Y is towards world up.
            else
                rotSendType = 3; // Full 3 DOF
        }
        else
            rotSendType = 0;

        if (scaleChanged)
        {
            float3 s = t.scale.Abs();
            scaleSendType = (s.MaxElement() - s.MinElement() <= 1e-3f) ? 1 : 2; // Uniform scale only?
        }
        else
            scaleSendType = 0;

        const float3 &linearVel = rigidBody ? rigidBody->linearVelocity.Get() : float3::zero;
        const float3 angVel = rigidBody ? DegToRad(rigidBody->angularVelocity.Get()) : float3::zero;

        velSendType = velocityDirty ? (linearVel.LengthSq() >= 64.f ? 2 : 1) : 0;
        angVelSendType = angularVelocityDirty ? 1 : 0;

        if (posSendType == 0 && rotSendType == 0 && scaleSendType == 0 && velSendType == 0 && angVelSendType == 0)
            continue;

        int bitIdx = ds.BitsFilled();
        UNREFERENCED_PARAM(bitIdx)
        ds.AddVLE<kNet::VLE8_16_32>(ess.id); // Sends max. 32 bits.

        ds.AddArithmeticEncoded(8, posSendType, 3, rotSendType, 4, scaleSendType, 3, velSendType, 3, angVelSendType, 2); // Sends fixed 8 bits.
        if (posSendType == 1) // Sends fixed 57 bits.
        {
            ds.AddSignedFixedPoint(11, 8, t.pos.x);
            ds.AddSignedFixedPoint(11, 8, t.pos.y);
            ds.AddSignedFixedPoint(11, 8, t.pos.z);
        }
        else if (posSendType == 2) // Sends fixed 96 bits.
        {
            ds.Add<float>(t.pos.x);
            ds.Add<float>(t.pos.y);
            ds.Add<float>(t.pos.z);
        }        

        if (rotSendType == 1) // Orientation with 1 DOF, only yaw.
        {
            // The transform is looking straight forward, i.e. the +y vector of the transform local space points straight towards +y in world space.
            // Therefore the forward vector has y == 0, so send (x,z) as a 2D vector.
            ds.AddNormalizedVector2D(rot.Col(2).x, rot.Col(2).z, 8);  // Sends fixed 8 bits.
        }
        else if (rotSendType == 2) // Orientation with 2 DOF, yaw and pitch.
        {
            float3 forward = rot.Col(2);
            forward.Normalize();
            ds.AddNormalizedVector3D(forward.x, forward.y, forward.z, 9, 8); // Sends fixed 17 bits.
        }
        else if (rotSendType == 3) // Orientation with 3 DOF, full yaw, pitch and roll.
        {
            Quat o = t.Orientation();

            float3 axis;
            float angle;
            o.ToAxisAngle(axis, angle);
            if (angle >= 3.141592654f) // Remove the quaternion double cover representation by constraining angle to [0, pi].
            {
                axis = -axis;
                angle = 2.f * 3.141592654f - angle;
            }

            // Sends 10-31 bits.
            u32 quantizedAngle = ds.AddQuantizedFloat(0, 3.141592654f, 10, angle);
            if (quantizedAngle != 0)
                ds.AddNormalizedVector3D(axis.x, axis.y, axis.z, 11, 10);
        }

        if (scaleSendType == 1) // Sends fixed 32 bytes.
        {
            ds.Add<float>(t.scale.x);
        }
        else if (scaleSendType == 2) // Sends fixed 96 bits.
        {
            ds.Add<float>(t.scale.x);
            ds.Add<float>(t.scale.y);
            ds.Add<float>(t.scale.z);
        }

        if (velSendType == 1) // Sends fixed 32 bits.
        {
            ds.AddVector3D(linearVel.x, linearVel.y, linearVel.z, 11, 10, 3, 8);
            ess.linearVelocity = linearVel;
        }
        else if (velSendType == 2) // Sends fixed 39 bits.
        {
            ds.AddVector3D(linearVel.x, linearVel.y, linearVel.z, 11, 10, 10, 8);
            ess.linearVelocity = linearVel;
        }

        if (angVelSendType == 1)
        {
            Quat o = Quat::FromEulerZYX(angVel.z, angVel.y, angVel.x);

            float3 axis;
            float angle;
            o.ToAxisAngle(axis, angle);
            if (angle >= 3.141592654f) // Remove the quaternion double cover representation by constraining angle to [0, pi].
            {
                axis = -axis;
                angle = 2.f * 3.141592654f - angle;
            }
             // Sends at most 31 bits.
            u32 quantizedAngle = ds.AddQuantizedFloat(0, 3.141592654f, 10, angle);
            if (quantizedAngle != 0)
                ds.AddNormalizedVector3D(axis.x, axis.y, axis.z, 11, 10);

            ess.angularVelocity = angVel;
        }
        if (posSendType != 0)
            ess.transform.pos = t.pos;
        if (rotSendType != 0)
            ess.transform.rot = t.rot;
        if (scaleSendType != 0)
            ess.transform.scale = t.scale;

//        std::cout << "pos: " << posSendType << ", rot: " << rotSendType << ", scale: " << scaleSendType << ", vel: " << velSendType << ", angvel: " << angVelSendType << std::endl;

        int bitsEnd = ds.BitsFilled();
        UNREFERENCED_PARAM(bitsEnd)
        ess.lastNetworkSendTime = kNet::Clock::Tick();
    }
    if (ds.BytesFilled() > 0)
        destination->EndAndQueueMessage(msg, ds.BytesFilled());
    else
        destination->FreeMessage(msg);
}

void SyncManager::HandleRigidBodyChanges(kNet::MessageConnection* source, kNet::packet_id_t packetId, const char* data, size_t numBytes)
{
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;

    kNet::DataDeserializer dd(data, numBytes);
    while(dd.BitsLeft() >= 9)
    {
        u32 entityID = dd.ReadVLE<kNet::VLE8_16_32>();
        EntityPtr e = scene->GetEntity(entityID);
        shared_ptr<EC_Placeable> placeable = e ? e->GetComponent<EC_Placeable>() : shared_ptr<EC_Placeable>();
        shared_ptr<EC_RigidBody> rigidBody = e ? e->GetComponent<EC_RigidBody>() : shared_ptr<EC_RigidBody>();
        Transform t = e ? placeable->transform.Get() : Transform();

        float3 newLinearVel = rigidBody ? rigidBody->linearVelocity.Get() : float3::zero;

        // If the server omitted linear velocity, interpolate towards the last received linear velocity.
        std::map<entity_id_t, RigidBodyInterpolationState>::iterator iter = e ? server_syncstate_.entityInterpolations.find(entityID) : server_syncstate_.entityInterpolations.end();
        if (iter != server_syncstate_.entityInterpolations.end())
            newLinearVel = iter->second.interpEnd.vel;

        int posSendType;
        int rotSendType;
        int scaleSendType;
        int velSendType;
        int angVelSendType;
        dd.ReadArithmeticEncoded(8, posSendType, 3, rotSendType, 4, scaleSendType, 3, velSendType, 3, angVelSendType, 2);

        if (posSendType == 1)
        {
            t.pos.x = dd.ReadSignedFixedPoint(11, 8);
            t.pos.y = dd.ReadSignedFixedPoint(11, 8);
            t.pos.z = dd.ReadSignedFixedPoint(11, 8);
        }
        else if (posSendType == 2)
        {
            t.pos.x = dd.Read<float>();
            t.pos.y = dd.Read<float>();
            t.pos.z = dd.Read<float>();
        }

        if (rotSendType == 1) // 1 DOF
        {
            float3 forward;
            dd.ReadNormalizedVector2D(8, forward.x, forward.z);
            forward.y = 0.f;
            float3x3 orientation = float3x3::LookAt(float3::unitZ, forward, float3::unitY, float3::unitY);
            t.SetOrientation(orientation);
        }
        else if (rotSendType == 2)
        {
            float3 forward;
            dd.ReadNormalizedVector3D(9, 8, forward.x, forward.y, forward.z);

            float3x3 orientation = float3x3::LookAt(float3::unitZ, forward, float3::unitY, float3::unitY);
            t.SetOrientation(orientation);

        }
        else if (rotSendType == 3)
        {
            // Read the quantized float manually, without a call to ReadQuantizedFloat, to be able to compare the quantized bit pattern.
	        u32 quantizedAngle = dd.ReadBits(10);
            if (quantizedAngle != 0)
            {
                float angle = quantizedAngle * 3.141592654f / (float)((1 << 10) - 1);
                float3 axis;
                dd.ReadNormalizedVector3D(11, 10, axis.x, axis.y, axis.z);
                t.SetOrientation(Quat(axis, angle));
            }
            else
                t.SetOrientation(Quat::identity);
        }

        if (scaleSendType == 1)
            t.scale = float3::FromScalar(dd.Read<float>());
        else if (scaleSendType == 2)
        {
            t.scale.x = dd.Read<float>();
            t.scale.y = dd.Read<float>();
            t.scale.z = dd.Read<float>();
        }

        if (velSendType == 1)
            dd.ReadVector3D(11, 10, 3, 8, newLinearVel.x, newLinearVel.y, newLinearVel.z);
        else if (velSendType == 2)
            dd.ReadVector3D(11, 10, 10, 8, newLinearVel.x, newLinearVel.y, newLinearVel.z);

        float3 newAngVel = rigidBody ? rigidBody->angularVelocity.Get() : float3::zero;

        if (angVelSendType == 1)
        {
            // Read the quantized float manually, without a call to ReadQuantizedFloat, to be able to compare the quantized bit pattern.
            u32 quantizedAngle = dd.ReadBits(10);
            if (quantizedAngle != 0)
            {
                float angle = quantizedAngle * 3.141592654f / (float)((1 << 10) - 1);
                float3 axis;
                dd.ReadNormalizedVector3D(11, 10, axis.x, axis.y, axis.z);
                Quat q(axis, angle);
                newAngVel = q.ToEulerZYX();
                Swap(newAngVel.z, newAngVel.x);
                newAngVel = RadToDeg(newAngVel);
            }
        }

        if (!e) // Discard this message - we don't have the entity in our scene to which the message applies to.
            continue;

        // Did anything change?
        if (posSendType != 0 || rotSendType != 0 || scaleSendType != 0 || velSendType != 0 || angVelSendType != 0)
        {
            // Create or update the interpolation state.
            Transform orig = placeable->transform.Get();

            std::map<entity_id_t, RigidBodyInterpolationState>::iterator iter = server_syncstate_.entityInterpolations.find(entityID);
            if (iter != server_syncstate_.entityInterpolations.end())
            {
                RigidBodyInterpolationState &interp = iter->second;

                if (kNet::PacketIDIsNewerThan(interp.lastReceivedPacketCounter, packetId))
                    continue; // This is an out-of-order received packet. Ignore it. (latest-data-guarantee)
                interp.lastReceivedPacketCounter = packetId;

                const float interpPeriod = updatePeriod_; // Time in seconds how long interpolating the Hermite spline from [0,1] should take.
                float3 curVel;

                if (interp.interpTime < 1.0f)
                    curVel = HermiteDerivative(interp.interpStart.pos, interp.interpStart.vel*interpPeriod, interp.interpEnd.pos, interp.interpEnd.vel*interpPeriod, interp.interpTime);
                else
                    curVel = interp.interpEnd.vel;
                float3 curAngVel = float3::zero; ///\todo
                interp.interpStart.pos = orig.pos;
                if (posSendType != 0)
                    interp.interpEnd.pos = t.pos;
                interp.interpStart.rot = orig.Orientation();
                if (rotSendType != 0)
                    interp.interpEnd.rot = t.Orientation();
                interp.interpStart.scale = orig.scale;
                if (scaleSendType != 0)
                    interp.interpEnd.scale = t.scale;
                interp.interpStart.vel = curVel;
                if (velSendType != 0)
                    interp.interpEnd.vel = newLinearVel;
                interp.interpStart.angVel = curAngVel;
                if (angVelSendType != 0)
                    interp.interpEnd.angVel = newAngVel;
                interp.interpTime = 0.f;
                interp.interpolatorActive = true;

                // Objects without a rigidbody, or with mass 0 never extrapolate (objects with mass 0 are stationary for Bullet).
                const bool isNewtonian = rigidBody && rigidBody->mass.Get() > 0;
                if (!isNewtonian)
                    interp.interpStart.vel = interp.interpEnd.vel = float3::zero;
            }
            else
            {
                RigidBodyInterpolationState interp;
                interp.interpStart.pos = orig.pos;
                interp.interpEnd.pos = t.pos;
                interp.interpStart.rot = orig.Orientation();
                interp.interpEnd.rot = t.Orientation();
                interp.interpStart.scale = orig.scale;
                interp.interpEnd.scale = t.scale;
                interp.interpStart.vel = rigidBody ? rigidBody->linearVelocity.Get() : float3::zero;
                interp.interpEnd.vel = newLinearVel;
                interp.interpStart.angVel = rigidBody ? rigidBody->angularVelocity.Get() : float3::zero;
                interp.interpEnd.angVel = newAngVel;
                interp.interpTime = 0.f;
                interp.lastReceivedPacketCounter = packetId;
                interp.interpolatorActive = true;
                server_syncstate_.entityInterpolations[entityID] = interp;
            }
        }
    }
}

void SyncManager::ProcessSyncState(kNet::MessageConnection* destination, SceneSyncState* state)
{
    PROFILE(SyncManager_ProcessSyncState);
    
    unsigned sceneId = 0; ///\todo Replace with proper scene ID once multiscene support is in place.
    
    ScenePtr scene = scene_.lock();
    int numMessagesSent = 0;
    bool isServer = owner_->IsServer();
    UNREFERENCED_PARAM(isServer)
    
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
            {
                if (i->second->IsReplicated())
                    ++numReplicatedComponents;
            }
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
    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!user || user->properties["authenticated"] != "true")
        return false;
    
    return true;
}

void SyncManager::HandleCreateEntity(kNet::MessageConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = GetSceneSyncState(source);
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateEntity message");
        return;
    }

    if (!scene->AllowModifyEntity(user.get(), 0)) //should be 'ModifyScene', but ModifyEntity is now the signal that covers all
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

    /** As the client created the entity and already has it in its local state,
        we must add it to the servers sync state for the client without emitting any StateChangeRequest signals.
        @note The below state->MarkComponentProcessed() already accomplishes part of this, but still do explicitly here!
        @note The below entity->CreateComponentWithId() will trigger the signaling logic but it will stop in 
        SceneSyncState::FillRequest() as the Entity is not yet in the scene! */
    if (isServer)
    {
        state->RemovePendingEntity(senderEntityID);
        state->RemovePendingEntity(entityID);
        state->MarkEntityProcessed(entityID);
    }
    
    std::vector<std::pair<component_id_t, component_id_t> > componentIdRewrites;

    try
    {    
        // Read the temporary flag
        bool temporary = ds.Read<u8>() != 0;
        entity->SetTemporary(temporary);
        
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
    } catch(kNet::NetException &/*e*/)
    {
        LogError("Failed to deserialize the creation of a new entity from the peer. Deleting the partially crafted entity!");
        scene->RemoveEntity(entity->Id(), AttributeChange::Disconnected);
        throw; // Propagate the exception up, to handle a peer which is sending us bad protocol bits.
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
    
    std::vector<std::pair<component_id_t, component_id_t> > componentIdRewrites;
    std::vector<ComponentPtr> addedComponents;

    EntityPtr entity;
    u32 sceneID;
    entity_id_t entityID;
    try
    {
        kNet::DataDeserializer ds(data, numBytes);
        sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
        entityID = ds.ReadVLE<kNet::VLE8_16_32>();
        
        if (!ValidateAction(source, cCreateComponentsMessage, entityID))
            return;

        entity = scene->GetEntity(entityID);
        if (!entity)
        {
            LogWarning("Entity " + QString::number(entityID) + " not found for CreateComponents message");
            return;
        }

        UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
        if (!scene->AllowModifyEntity(user.get(), entity.get()))
            return;
        
        // Read the components
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
    } catch(kNet::NetException &/*e*/)
    {
        LogError("Failed to deserialize the creation of new component(s) from the peer. Deleting the partially crafted components!");
        for(size_t i = 0; i < addedComponents.size(); ++i)
            entity->RemoveComponent(addedComponents[i], AttributeChange::Disconnected);
        throw; // Propagate the exception up, to handle a peer which is sending us bad protocol bits.
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
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveEntityMessage, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);

    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
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
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveComponentsMessage, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);

    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
        return;

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
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cCreateAttributesMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);
    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for CreateAttributes message");
        return;
    }

    if (!scene->AllowModifyEntity(user.get(), 0)) //to check if creating entities is allowed (for this user)
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
        try
        {
            attr->FromBinary(ds, AttributeChange::Disconnected);
        } catch (kNet::NetException &/*e*/)
        {
            LogError("Failed to deserialize the creation of a new attribute from the peer!");
            comp->RemoveAttribute(attrIndex, AttributeChange::Disconnected);
            throw;
        }
        
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
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);

    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
        return;

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
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
        
    EntityPtr entity = scene->GetEntity(entityID);
    UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);

    if (entity && !scene->AllowModifyEntity(user.get(), entity.get())) // check if allowed to modify this entity.
        return;

    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for EditAttributes message");
        return;
    }
    
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
    UNREFERENCED_PARAM(sceneID)
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
    UNREFERENCED_PARAM(sceneID)
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
        LogWarning("SyncManager: Ignoring received MsgEntityAction \"" + QString(msg.name.size() == 0 ? "(null)" : std::string((const char *)&msg.name[0], msg.name.size()).c_str()) + "\" (" + QString::number(msg.parameters.size()) + " parameters) for entity ID " + QString::number(msg.entityId) + " as no scene exists!");
        return;
    }
    
    entity_id_t entityId = msg.entityId;
    EntityPtr entity = scene->GetEntity(entityId);
    if (!entity)
    {
        LogWarning("Entity with ID " + QString::number(entityId) + " not found for EntityAction message \"" + QString(msg.name.size() == 0 ? "(null)" : std::string((const char *)&msg.name[0], msg.name.size()).c_str()) + "\" (" + QString::number(msg.parameters.size()) + " parameters).");
        return;
    }

    // If we are server, get the user who sent the action, so it can be queried
    if (isServer)
    {
        Server* server = owner_->GetServer().get();
        if (server)
        {
            UserConnectionPtr user = owner_->GetKristalliModule()->GetUserConnection(source);
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
        server->SetActionSender(UserConnectionPtr());
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
