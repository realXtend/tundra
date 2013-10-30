// For conditions of distribution and use, see copyright notice in LICENSE

#include "WebSocketSyncManager.h"
#include "WebSocketServer.h"
#include "WebSocketUserConnection.h"

#include "WebSocketServerModule.h"

#include "KristalliProtocolModule.h"
#include "TundraMessages.h"
#include "MsgEntityAction.h"
#include "EntityAction.h"

#include "Scene.h"
#include "Entity.h"
#include "CoreStringUtils.h"
#include "EC_DynamicComponent.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "AttributeMetadata.h"
#include "LoggingFunctions.h"
#include "Profiler.h"
#include "EC_Placeable.h"
//#include "EC_RigidBody.h" // needed when cRigidBodyUpdateMessage is implemented
#include "SceneAPI.h"

#include <kNet.h>

#include <cstring>

// This variable is used for the interpolation stop check
UserConnection* currentSender = 0;

namespace WebSocket
{

void SyncManager::QueueMessage(UserConnection* connection, kNet::message_id_t id, bool /*reliable*/, bool /*inOrder*/, kNet::DataSerializer& ds)
{
    if (!connection)
        return;

    kNet::DataSerializer data(ds.BytesFilled() + 2);
    data.Add<u16>(static_cast<u16>(id));
    data.AddAlignedByteArray(ds.GetData(), static_cast<u32>(ds.BytesFilled()));
    
    connection->Send(data);
}

void SyncManager::WriteComponentFullUpdate(kNet::DataSerializer& ds, ComponentPtr comp)
{
    // Component identification
    ds.Add<u16>(comp->Id() & UniqueIdGenerator::LAST_REPLICATED_ID);
    ds.Add<u16>(comp->TypeId());
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
    ds.Add<u32>(static_cast<u32>(attrDs.BytesFilled()));
    ds.AddArray<u8>((unsigned char*)attrDataBuffer_, static_cast<u32>(attrDs.BytesFilled()));
}

SyncManager::SyncManager(WebSocketServerModule* owner) :
    owner_(owner),
    framework_(owner->GetFramework()),
    updatePeriod_(1.0f / 20.0f),
    updateAcc_(0.0)
{
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

SceneSyncState* SyncManager::SceneState(uint connectionId) const
{
    if (!owner_->IsServer())
        return 0;
    return SceneState(owner_->GetServer()->UserConnection(connectionId));
}

SceneSyncState* SyncManager::SceneState(WebSocket::UserConnection *connection) const
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
        LogError("WebSocket::SyncManager::RegisterToScene: Null scene, cannot replicate");
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

void SyncManager::HandleKristalliMessage(UserConnection* source, kNet::packet_id_t /*packetId*/, kNet::message_id_t messageId, const char* data, size_t numBytes)
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
            /*case cRigidBodyUpdateMessage:
                HandleRigidBodyChanges(source, packetId, data, numBytes);
                break;*/
            case cEntityActionMessage:
            {
                MsgEntityAction msg(data, numBytes);
                HandleEntityAction(source, msg);
                break;
            }
        }
    }
    catch (kNet::NetException& e)
    {
        LogError("Exception while handling scene sync network message " + QString::number(messageId) + ": " + QString(e.what()));
        throw; // Propagate the message so that Tundra server will kill the connection (if we are the server).
    }
    currentSender = 0;
}

void SyncManager::OnUserConnected(WebSocket::UserConnection *connection, QVariantMap * /*data*/)
{
    PROFILE(WebSocketSyncManager_NewUserConnected);
    
    if (!owner_->IsServer())
        return;

    ScenePtr scene = scene_.lock();
    if (!scene)
    {
        LogWarning("WebSocket::SyncManager: Cannot handle new user connection message - No scene set!");
        return;
    }
    
    // Connect to actions sent to specifically to this user
    connect(connection, SIGNAL(ActionTriggered(WebSocket::UserConnection*, Entity*, const QString&, const QStringList&)),
        this, SLOT(OnUserActionTriggered(WebSocket::UserConnection*, Entity*, const QString&, const QStringList&)));
    
    // Mark all entities in the sync state as new so we will send them
    connection->syncState = MAKE_SHARED(SceneSyncState, connection->ConnectionId(), owner_->IsServer());
    connection->syncState->SetParentScene(scene_);

    emit SceneStateCreated(connection, connection->syncState.get());

    for(Scene::iterator iter = scene->begin(); iter != scene->end(); ++iter)
    {
        EntityPtr entity = iter->second;
        if (entity->IsLocal())
            continue;
        entity_id_t id = entity->Id();
        connection->syncState->MarkEntityDirty(id);
    }
}

void SyncManager::OnClientEntityAction(WebSocket::UserConnection *source, MsgEntityAction action)
{
    HandleEntityAction(source, action);
}

void SyncManager::OnAttributeChanged(IComponent* comp, IAttribute* attr, AttributeChange::Type change)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;
    if (!owner_->IsServer())
        return;

    // Is this change even supposed to go to the network?
    if (change != AttributeChange::Replicate || comp->IsLocal())
        return;

    Entity* entity = comp->ParentEntity();
    if (!entity || entity->IsLocal())
        return; // This is a local entity, don't take it to network.
    
    // For each client connected to this server, mark this attribute dirty, so it will be updated to the
    // clients on the next network sync iteration.
    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState)
            (*i)->syncState->MarkAttributeDirty(entity->Id(), comp->Id(), attr->Index());
}

void SyncManager::OnAttributeAdded(IComponent* comp, IAttribute* attr, AttributeChange::Type /*change*/)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;
    if (!owner_->IsServer())
        return;
    
    // We do not allow to create attributes in local or disconnected signaling mode in a replicated component.
    // Always replicate the creation, because the client & server must have their attribute count in sync to 
    // be able to send attribute bitmasks
    if (comp->IsLocal())
        return;
    Entity* entity = comp->ParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;
    
    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState) (*i)->syncState->MarkAttributeCreated(entity->Id(), comp->Id(), attr->Index());
}

void SyncManager::OnAttributeRemoved(IComponent* comp, IAttribute* attr, AttributeChange::Type /*change*/)
{
    assert(comp && attr);
    if (!comp || !attr)
        return;
    if (!owner_->IsServer())
        return;
    
    // We do not allow to remove attributes in local or disconnected signaling mode in a replicated component.
    // Always replicate the removeal, because the client & server must have their attribute count in sync to
    // be able to send attribute bitmasks
    if (comp->IsLocal())
        return;
    Entity* entity = comp->ParentEntity();
    if ((!entity) || (entity->IsLocal()))
        return;

    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState) (*i)->syncState->MarkAttributeRemoved(entity->Id(), comp->Id(), attr->Index());
}

void SyncManager::OnComponentAdded(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    assert(entity && comp);
    if (!entity || !comp)
        return;
    if (!owner_->IsServer())
        return;
        
    if ((change != AttributeChange::Replicate) || (comp->IsLocal()))
        return;
    if (entity->IsLocal())
        return;

    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState) (*i)->syncState->MarkComponentDirty(entity->Id(), comp->Id());
}

void SyncManager::OnComponentRemoved(Entity* entity, IComponent* comp, AttributeChange::Type change)
{
    assert(entity && comp);
    if (!entity || !comp)
        return;
    if (!owner_->IsServer())
        return;
        
    if ((change != AttributeChange::Replicate) || (comp->IsLocal()))
        return;
    if (entity->IsLocal())
        return;

    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState) (*i)->syncState->MarkComponentRemoved(entity->Id(), comp->Id());
}

void SyncManager::OnEntityCreated(Entity* entity, AttributeChange::Type change)
{
    assert(entity);
    if (!entity)
        return;
    if (!owner_->IsServer())
        return;
    if ((change != AttributeChange::Replicate) || (entity->IsLocal()))
        return;

    UserConnectionList &users = owner_->GetServer()->UserConnections();
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

void SyncManager::OnEntityRemoved(Entity* entity, AttributeChange::Type change)
{
    assert(entity);
    if (!entity)
        return;
    if (!owner_->IsServer())
        return;
        
    if (change != AttributeChange::Replicate)
        return;
    if (entity->IsLocal())
        return;

    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        if ((*i)->syncState) (*i)->syncState->MarkEntityRemoved(entity->Id());
}

void SyncManager::OnActionTriggered(Entity *entity, const QString &action, const QStringList &params, EntityAction::ExecTypeField type)
{
    bool isServer = owner_->IsServer();
    if (isServer && (type & EntityAction::Peers) != 0)
    {
        // Craft EntityAction message.
        MsgEntityAction msg;
        msg.entityId = entity->Id();
        msg.name = StringToBuffer(action.toStdString());
        msg.executionType = (u8)EntityAction::Local; // Propagate as local actions.

        for(int i=0; i<params.size(); ++i)
        {
            MsgEntityAction::S_parameters p = { StringToBuffer(params[i].toStdString()) };
            msg.parameters.push_back(p);
        }
        
        kNet::DataSerializer data(2 + msg.Size());
        data.Add<u16>(static_cast<u16>(cEntityActionMessage));
        msg.SerializeTo(data);
        
        UserConnectionList &users = owner_->GetServer()->UserConnections();
        for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
        {
            UserConnection *c = (*i);
            if (c->properties["authenticated"].toBool())
                c->Send(data);
        }
    }
}

void SyncManager::OnUserActionTriggered(WebSocket::UserConnection* user, Entity *entity, const QString &action, const QStringList &params)
{
    if (!entity || !user)
        return;
    if (!owner_->IsServer())
        return;
    if (!user->properties["authenticated"].toBool())
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
    
    kNet::DataSerializer data(2 + msg.Size());
    data.Add<u16>(static_cast<u16>(cEntityActionMessage));
    msg.SerializeTo(data);

    user->Send(data);
}

void SyncManager::Update(f64 frametime)
{
    PROFILE(WebSocketSyncManager_Update);
    
    if (!owner_->IsServer())
        return;

    // Check if it is yet time to perform a network update tick.
    updateAcc_ += (float)frametime;
    if (updateAcc_ < updatePeriod_)
        return;

    // If multiple updates passed, update still just once.
    updateAcc_ = fmod(updateAcc_, updatePeriod_);
    
    ScenePtr scene = scene_.lock();
    if (!scene)
        return;
    
    // If we are server, process all authenticated users
    // Then send out changes to other attributes via the generic sync mechanism.
    UserConnectionList &users = owner_->GetServer()->UserConnections();
    for(UserConnectionList::iterator i = users.begin(); i != users.end(); ++i)
    {
        if (!(*i)->syncState)
            continue;
        
        // First send out all changes to rigid bodies.
        // After processing this function, the bits related to rigid body states have been cleared,
        // so the generic sync will not double-replicate the rigid body positions and velocities.
        //ReplicateRigidBodyChanges((*i), (*i)->syncState.get());

        ProcessSyncState((*i), (*i)->syncState.get());
    }
}

/*
void SyncManager::ReplicateRigidBodyChanges(UserConnection* destination, SceneSyncState* state)
{
    PROFILE(WebSocketSyncManager_ReplicateRigidBodyChanges);
    
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
        //bool posChanged = transformDirty && (timeSinceLastSend > 0.2f || t.pos.DistanceSq( predictedClientSidePosition) > 5e-5f);
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

        //std::cout << "pos: " << posSendType << ", rot: " << rotSendType << ", scale: " << scaleSendType << ", vel: " << velSendType << ", angvel: " << angVelSendType << std::endl;

        int bitsEnd = ds.BitsFilled();
        UNREFERENCED_PARAM(bitsEnd)
        ess.lastNetworkSendTime = kNet::Clock::Tick();
    }
    if (ds.BytesFilled() > 0)
        destination->EndAndQueueMessage(msg, ds.BytesFilled());
    else
        destination->FreeMessage(msg);
}
*/

void SyncManager::ProcessSyncState(UserConnection* destination, SceneSyncState* state)
{
    PROFILE(WebSocketSyncManager_ProcessSyncState);
    
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
            ds.Add<u8>(sceneId);
            ds.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
            QueueMessage(destination, cRemoveEntityMessage, true, true, ds);
            ++numMessagesSent;
        }
        // New entity
        else if (entityState.isNew)
        {
            kNet::DataSerializer ds(createEntityBuffer_, 64 * 1024);
            
            // Entity identification and temporary flag
            ds.Add<u8>(sceneId);
            ds.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
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
            ds.Add<u16>(numReplicatedComponents);
            
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
                        removeCompsDs.Add<u8>(sceneId);
                        removeCompsDs.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                    }
                    // Then add component ID
                    removeCompsDs.Add<u16>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                }
                // New component
                else if (compState.isNew)
                {
                    // If first component, write the entity ID first
                    if (!createCompsDs.BytesFilled())
                    {
                        createCompsDs.Add<u8>(sceneId);
                        createCompsDs.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
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
                                    createAttrsDs.Add<u8>(sceneId);
                                    createAttrsDs.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                                }
                                
                                IAttribute* attr = attrs[attrIndex];
                                createAttrsDs.Add<u16>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
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
                                removeAttrsDs.Add<u8>(sceneId);
                                removeAttrsDs.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                            }
                            removeAttrsDs.Add<u16>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                            removeAttrsDs.Add<u8>(attrIndex);
                        }
                    }
                    compState.newAndRemovedAttributes.clear();
                    
                    // Now, if remaining dirty bits exist, they must be sent in the edit attributes message. These are the majority of our network data.
                    changedAttributes_.clear();
                    size_t numBytes = (attrs.size() + 7) >> 3;
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
                        /// @todo HACK for web clients while ReplicateRigidBodyChanges() is not implemented! 
                        /// Don't send out minuscule pos/rot/scale changes as it spams the network.
                        bool sendChanges = true;
                        if (changedAttributes_.size() == 1 && changedAttributes_[0] == 0 && comp->TypeId() == EC_Placeable::TypeIdStatic())
                        {
                            // EC_Placeable::Transform is the only change!
                            EC_Placeable *placeable = dynamic_cast<EC_Placeable*>(comp.get());
                            if (placeable)
                            {
                                const Transform &t = placeable->transform.Get();
                                bool posChanged = (t.pos.DistanceSq(entityState.transform.pos) > 1e-3f);
                                bool rotChanged = (t.rot.DistanceSq(entityState.transform.rot) > 1e-1f);
                                bool scaleChanged = (t.scale.DistanceSq(entityState.transform.scale) > 1e-3f);
                                
                                if (!posChanged && !rotChanged && !scaleChanged) // Dont send anything!
                                {
                                    //qDebug() << "EC_Placeable too small changes: " << t.pos.DistanceSq(entityState.transform.pos) << t.rot.DistanceSq(entityState.transform.rot) << t.scale.DistanceSq(entityState.transform.scale);
                                    sendChanges = false;
                                }
                                else
                                    entityState.transform = t; // Lets send the update. Update transform for the next above comparison.
                            }
                        }
                        
                        if (sendChanges)
                        {
                            // If first component for which attribute changes are sent, write the entity ID first
                            if (!editAttrsDs.BytesFilled())
                            {
                                editAttrsDs.Add<u8>(sceneId);
                                editAttrsDs.Add<u16>(entityState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                            }
                            editAttrsDs.Add<u16>(compState.id & UniqueIdGenerator::LAST_REPLICATED_ID);
                            
                            // Create a nested dataserializer for the actual attribute data, so we can skip components
                            kNet::DataSerializer attrDataDs(attrDataBuffer_, 16 * 1024);
                            
                            // There are changed attributes. Check if it is more optimal to send attribute indices, or the whole bitmask
                            size_t bitsMethod1 = changedAttributes_.size() * 8 + 8;
                            size_t bitsMethod2 = attrs.size();
                            // Method 1: indices
                            if (bitsMethod1 <= bitsMethod2)
                            {
                                attrDataDs.Add<kNet::bit>(0);
                                attrDataDs.Add<u8>(static_cast<u8>(changedAttributes_.size()));
                                for (size_t  i = 0; i < changedAttributes_.size(); ++i)
                                {
                                    attrDataDs.Add<u8>(changedAttributes_[i]);
                                    attrs[changedAttributes_[i]]->ToBinary(attrDataDs);
                                }
                            }
                            // Method 2: bitmask
                            else
                            {
                                attrDataDs.Add<kNet::bit>(1);
                                for (size_t  i = 0; i < attrs.size(); ++i)
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
                            editAttrsDs.Add<u16>(static_cast<u16>(attrDataDs.BytesFilled()));
                            editAttrsDs.AddArray<u8>((unsigned char*)attrDataBuffer_, static_cast<u32>(attrDataDs.BytesFilled()));
                        }
                        
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

bool SyncManager::ValidateAction(UserConnection* source, unsigned /*messageID*/, entity_id_t /*entityID*/)
{
    assert(source);
    
    // For now, always trust scene actions from server
    if (!owner_->IsServer())
        return true;
    
    // And for now, always also trust scene actions from clients, if they are known and authenticated
    if (!source->properties.value("authenticated", false).toBool())
        return false;
    
    return true;
}

void SyncManager::HandleCreateEntity(UserConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;

    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateEntity message");
        return;
    }
    
    /* @todo implement
    WebSocket::UserConnection *user = source;
    if (!scene->AllowModifyEntity(user, 0)) //should be 'ModifyScene', but ModifyEntity is now the signal that covers all
        return;
    */

    // For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    entity_id_t senderEntityID = entityID;
    
    if (!ValidateAction(source, cCreateEntityMessage, entityID))
        return;
    
    // Server never uses the client's entityID.
    entityID = scene->NextFreeId();

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
    state->RemovePendingEntity(senderEntityID);
    state->RemovePendingEntity(entityID);
    state->MarkEntityProcessed(entityID);
    
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
            
            // Rewrite the ID on the server
            compID = 0;
            
            u32 typeID = ds.ReadVLE<kNet::VLE8_16_32>();
            QString name = QString::fromStdString(ds.ReadString());
            unsigned attrDataSize = ds.ReadVLE<kNet::VLE8_16_32>();
            ds.ReadArray<u8>((u8*)&attrDataBuffer_[0], attrDataSize);
            kNet::DataDeserializer attrDs(attrDataBuffer_, attrDataSize);
            
            ComponentPtr comp = entity->CreateComponentWithId(compID, typeID, name, change);
            if (!comp)
            {
                LogWarning("Failed to create component type " + QString::number(compID) + " to " + entity->ToString() + " while handling CreateEntity message, skipping component");
                continue;
            }
            
            // On server, get the assigned ID now
            compID = comp->Id();
            componentIdRewrites.push_back(std::make_pair(senderCompID, compID));
            
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
    } 
    catch(kNet::NetException &/*e*/)
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
    kNet::DataSerializer replyDs(createEntityBuffer_, 64 * 1024);
    replyDs.AddVLE<kNet::VLE8_16_32>(sceneID);
    replyDs.AddVLE<kNet::VLE8_16_32>(senderEntityID & UniqueIdGenerator::LAST_REPLICATED_ID);
    replyDs.AddVLE<kNet::VLE8_16_32>(entityID & UniqueIdGenerator::LAST_REPLICATED_ID);
    replyDs.AddVLE<kNet::VLE8_16_32>(static_cast<u32>(componentIdRewrites.size()));
    for (unsigned i = 0; i < componentIdRewrites.size(); ++i)
    {
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].first & UniqueIdGenerator::LAST_REPLICATED_ID);
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].second & UniqueIdGenerator::LAST_REPLICATED_ID);
    }
    QueueMessage(source, cCreateEntityReplyMessage, true, true, replyDs);
    
    // Mark the entity processed (undirty) in the sender's syncstate so that create is not echoed back
    state->MarkEntityProcessed(entityID);
}

void SyncManager::HandleCreateComponents(UserConnection* source, const char* data, size_t numBytes)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
        
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding CreateComponents message");
        return;
    }

    // For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
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

        /* @todo implement
        WebSocket::UserConnection *user = source;
        if (!scene->AllowModifyEntity(user, entity.get()))
            return;
        */
        
        // Read the components
        while (ds.BitsLeft() > 2 * 8)
        {
            component_id_t compID = ds.ReadVLE<kNet::VLE8_16_32>();
            component_id_t senderCompID = compID;
            
            // Rewrite the ID on server
            compID = 0;
            
            u32 typeID = ds.ReadVLE<kNet::VLE8_16_32>();
            QString name = QString::fromStdString(ds.ReadString());
            unsigned attrDataSize = ds.ReadVLE<kNet::VLE8_16_32>();
            ds.ReadArray<u8>((u8*)&attrDataBuffer_[0], attrDataSize);
            kNet::DataDeserializer attrDs(attrDataBuffer_, attrDataSize);
            
            ComponentPtr comp = entity->CreateComponentWithId(compID, typeID, name, change);
            if (!comp)
            {
                LogWarning("Failed to create component type " + QString::number(compID) + " to " + entity->ToString() + " while handling CreateComponents message, skipping component");
                continue;
            }
            // On server, get the assigned ID now
            compID = comp->Id();
            componentIdRewrites.push_back(std::make_pair(senderCompID, compID));
            
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
    } 
    catch(kNet::NetException &/*e*/)
    {
        LogError("Failed to deserialize the creation of new component(s) from the peer. Deleting the partially crafted components!");
        for(size_t i = 0; i < addedComponents.size(); ++i)
            entity->RemoveComponent(addedComponents[i], AttributeChange::Disconnected);
        throw; // Propagate the exception up, to handle a peer which is sending us bad protocol bits.
    }
    
    // Send CreateComponentsReply
    kNet::DataSerializer replyDs(createEntityBuffer_, 64 * 1024);
    replyDs.AddVLE<kNet::VLE8_16_32>(sceneID);
    replyDs.AddVLE<kNet::VLE8_16_32>(entityID & UniqueIdGenerator::LAST_REPLICATED_ID);
    replyDs.AddVLE<kNet::VLE8_16_32>(static_cast<u32>(componentIdRewrites.size()));
    for (unsigned i = 0; i < componentIdRewrites.size(); ++i)
    {
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].first & UniqueIdGenerator::LAST_REPLICATED_ID);
        replyDs.AddVLE<kNet::VLE8_16_32>(componentIdRewrites[i].second & UniqueIdGenerator::LAST_REPLICATED_ID);
    }
    QueueMessage(source, cCreateComponentsReplyMessage, true, true, replyDs);
    
    // Emit the component changes last, to signal only a coherent state of the whole entity
    for (unsigned i = 0; i < addedComponents.size(); ++i)
        addedComponents[i]->ComponentChanged(change);
}

void SyncManager::HandleRemoveEntity(UserConnection* source, const char* data, size_t numBytes)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
        
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveEntity message");
        return;
    }
    
    // For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveEntityMessage, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);

    /* @todo implement
    WebSocket::UserConnection *user = source;
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
        return;
    */

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

void SyncManager::HandleRemoveComponents(UserConnection* source, const char* data, size_t numBytes)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
        
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveComponents message");
        return;
    }
    
    // For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveComponentsMessage, entityID))
        return;

    EntityPtr entity = scene->GetEntity(entityID);

    /* @todo implement
    WebSocket::UserConnection *user = owner_->GetServer()->UserConnection(source);
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
        return;
    */

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

void SyncManager::HandleCreateAttributes(UserConnection* source, const char* data, size_t numBytes)
{
    assert(source);
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
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
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for CreateAttributes message");
        return;
    }
    
    /* @todo implement
    WebSocket::UserConnection *user = source;
    if (!scene->AllowModifyEntity(user.get(), 0)) //to check if creating entities is allowed (for this user)
        return;
    */

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
        
        // If we are server, do not allow to overwrite existing attributes by client requests
        const AttributeVector& existingAttrs = comp->Attributes();
        if (attrIndex < existingAttrs.size() && existingAttrs[attrIndex])
        {
            LogWarning("Client attempted to overwrite an existing attribute index " + QString::number(attrIndex) + " in component " + comp->TypeName() + " in " + entity->ToString() + ", aborting CreateAttributes message parsing");
            return;
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
        }
        catch (kNet::NetException &/*e*/)
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

void SyncManager::HandleRemoveAttributes(UserConnection* source, const char* data, size_t numBytes)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
        
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding RemoveAttributes message");
        return;
    }
    
    // For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
    
    EntityPtr entity = scene->GetEntity(entityID);

    /* @todo implement
    WebSocket::UserConnection *user = owner_->GetServer()->UserConnection(source);
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get()))
        return;
    */

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

void SyncManager::HandleEditAttributes(UserConnection* source, const char* data, size_t numBytes)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
        
    // Get matching syncstate for reflecting the changes
    SceneSyncState* state = source->syncState.get();
    ScenePtr scene = GetRegisteredScene();
    if (!scene || !state)
    {
        LogWarning("Null scene or sync state, disregarding EditAttributes message");
        return;
    }
    
    // For clients, the change type is LocalOnly. For server, the change type is Replicate, so that it will get replicated to all clients in turn
    AttributeChange::Type change = AttributeChange::Replicate;
    
    kNet::DataDeserializer ds(data, numBytes);
    unsigned sceneID = ds.ReadVLE<kNet::VLE8_16_32>(); ///\todo Dummy ID. Lookup scene once multiscene is properly supported
    UNREFERENCED_PARAM(sceneID)
    entity_id_t entityID = ds.ReadVLE<kNet::VLE8_16_32>();
    
    if (!ValidateAction(source, cRemoveAttributesMessage, entityID))
        return;
        
    EntityPtr entity = scene->GetEntity(entityID);
    if (!entity)
    {
        LogWarning("Entity " + QString::number(entityID) + " not found for EditAttributes message");
        return;
    }
    
    /* @todo implement
    WebSocket::UserConnection *user = source;
    if (entity && !scene->AllowModifyEntity(user.get(), entity.get())) // check if allowed to modify this entity.
        return;
    */
        
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
                
                attr->FromBinary(attrDs, AttributeChange::Disconnected);
                changedAttrs.push_back(attr);
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

                    attr->FromBinary(attrDs, AttributeChange::Disconnected);
                    changedAttrs.push_back(attr);
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

void SyncManager::HandleEntityAction(UserConnection* source, MsgEntityAction& msg)
{
    // This module will be ran only in server instances
    if (!owner_->IsServer())
        return;
    WebSocket::Server* server = owner_->GetServer().get();
    if (!server)
        return;
    
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

    // Get the user who sent the action, so it can be queried
    server->SetActionSender(source);
    
    QString action = BufferToString(msg.name).c_str();
    QStringList params;
    for(uint i = 0; i < msg.parameters.size(); ++i)
        params << BufferToString(msg.parameters[i].parameter).c_str();
    EntityAction::ExecTypeField type = (EntityAction::ExecTypeField)(msg.executionType);

    bool handled = false;

    if ((type & EntityAction::Local) != 0 || (type & EntityAction::Server) != 0)
    {
        entity->Exec(EntityAction::Local, action, params); // Execute the action locally, so that it doesn't immediately propagate back to network for sending.
        handled = true;
    }

    // If execution type is Peers, replicate to all peers but the sender.
    if ((type & EntityAction::Peers) != 0)
    {
        entity->Exec(EntityAction::Peers, action, params); // This will trigger the signal to both web and native clients via the Scene.
        handled = true;
    }
    
    if (!handled)
        LogWarning("SyncManager: Received MsgEntityAction message \"" + action + "\", but it went unhandled because of its type=" + QString::number(type));

    // Clear the action sender after action handling
    server->SetActionSender(0);
}

}
