#include "ECServer.h"
#include "ECServerMessages.h"
#include "KristalliServer.h"
#include "KristalliServerMessages.h"
#include "Utilities.h"

#include "MsgActionReply.h"
#include "MsgComponentDeleted.h"
#include "MsgComponentUpdated.h"
#include "MsgCreateEntity.h"
#include "MsgDeleteComponent.h"
#include "MsgDeleteEntity.h"
#include "MsgEntityDeleted.h"
#include "MsgEntityUpdated.h"
#include "MsgModifyComponent.h"
#include "MsgSetPermissions.h"

#include <stdio.h>

using namespace std;
using namespace clb;

ECServer::ECServer(KristalliServer* kristalliServer) :
    server(kristalliServer)
{
    server->AddService(this);
}

ECServer::~ECServer()
{
    server->RemoveService(this);
}

void ECServer::HandleConnect(ServerConnection* connection)
{
    // Replicate initial scene to new user
    SendInitialScene(connection);
}

void ECServer::HandleDisconnect(ServerConnection* connection)
{
}

void ECServer::ResetScene()
{
    LoadScene();
}

void ECServer::LoadScene()
{
    // Replicate deletion of old entities
    for (uint i = 0; i < scene.entities.size(); ++i)
    {
        ServerConnectionList& serverConnections = server->GetConnections();
        for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
            ReplicateEntityDelete(&*iter, scene.entities[i]->entityID, scene.entities[i]->entityUUID);
    }
    
    scene.Clear();
    
    FILE *file = fopen("scene.dat", "rb");
    if (!file)
        return;
    
    fseek(file, 0, SEEK_END);
    unsigned long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    std::vector<u8> buffer;
    buffer.resize(length);
    fread(&buffer[0], 1, length, file);
    fclose(file);
    
    try
    {
        DataDeserializer ds((const char*)&buffer[0], length);
        u32 entityCount = ds.Read<u32>();
        for (u32 i = 0; i < entityCount; ++i)
        {
            u32 entityID = ds.Read<u32>();
            RexUUID entityUUID;
            ds.ReadArray<u8>(&entityUUID.data[0], RexUUID::cSizeBytes);
            RexUUID creatorUUID;
            ds.ReadArray<u8>(&creatorUUID.data[0], RexUUID::cSizeBytes);
            
            EntityPtr newEntity = new Entity(entityID);
            newEntity->entityUUID = entityUUID;
            newEntity->creatorUUID = creatorUUID;
            /// \todo permissions are not stored/reloaded. We probably won't even use/define them for now
            
            u32 componentCount = ds.Read<u32>();
            for (u32 j = 0; j < componentCount; ++j)
            {
                std::string componentTypeName = ds.ReadString();
                std::string componentName = ds.ReadString();
                
                ComponentPtr newComponent = newEntity->GetOrCreateComponent(componentTypeName, componentName);
                
                std::vector<u8> componentData;
                componentData.resize(ds.Read<u32>());
                ds.ReadArray<u8>(&componentData[0], componentData.size());
                newComponent->data = componentData;
            }
            
            scene.entities.push_back(newEntity);
        }
    }
    catch (...) {}
    
    // Now replicate the new entities & their components to everyone
    for (uint i = 0; i < scene.entities.size(); ++i)
    {
        ServerConnectionList& serverConnections = server->GetConnections();
        for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
            ReplicateEntity(&*iter, scene.entities[i]);
    }
}

void ECServer::SaveScene()
{
    std::vector<u8> sceneData;
    sceneData.resize(1024 * 1024);
    DataSerializer ds((char*)&sceneData[0], sceneData.size());
    
    ds.Add<u32>(scene.entities.size());
    for (u32 i = 0; i < scene.entities.size(); ++i)
    {
        EntityPtr entity = scene.entities[i];
        ds.Add<u32>(entity->entityID);
        ds.AddArray<u8>(&entity->entityUUID.data[0], RexUUID::cSizeBytes);
        ds.AddArray<u8>(&entity->creatorUUID.data[0], RexUUID::cSizeBytes);
        
        ds.Add<u32>(entity->components.size());
        for (u32 j = 0; j < entity->components.size(); ++j)
        {
            ComponentPtr comp = entity->components[j];
            ds.AddString(comp->typeName);
            ds.AddString(comp->name);
            ds.Add<u32>(comp->data.size());
            ds.AddArray<u8>(&comp->data[0], comp->data.size());
        }
    }
    sceneData.resize(ds.BytesFilled());
    FILE* file = fopen("scene.dat", "wb");
    if (!file)
    {
        LOG("Failed to open scene.dat for writing");
        return;
    }
    fwrite(&sceneData[0], 1, sceneData.size(), file);
    fclose(file);
}

void ECServer::HandleMessage(ServerConnection* connection, message_id_t id, const char *data, size_t numBytes)
{
    switch (id)
    {
    case cCreateEntityMessage:
        {
            MsgCreateEntity msg(data, numBytes);
            HandleCreateEntityMessage(connection, msg);
        }
        break;
    case cDeleteEntityMessage:
        {
            MsgDeleteEntity msg(data, numBytes);
            HandleDeleteEntityMessage(connection, msg);
        }
        break;
    case cModifyComponentMessage:
        {
            MsgModifyComponent msg(data, numBytes);
            HandleModifyComponentMessage(connection, msg);
        }
        break;
    case cDeleteComponentMessage:
        {
            MsgDeleteComponent msg(data, numBytes);
            HandleDeleteComponentMessage(connection, msg);
        }
        break;
    case cSetPermissionsMessage:
        {
            MsgSetPermissions msg(data, numBytes);
            HandleSetPermissionsMessage(connection, msg);
        }
        break;
    }
}

void ECServer::HandleCreateEntityMessage(ServerConnection* source, MsgCreateEntity& msg)
{
    /// \todo Permission check. Does the user have right to add an entity?
    
    u32 newEntityID = scene.GetNextFreeEntityID();
    EntityPtr entity = scene.GetOrCreateEntity(newEntityID);
    entity->creatorUUID = source->userUUID;
    // Set permissions if specified
    for (unsigned i = 0; i < msg.permissions.size(); ++i)
    {
        EntityPermission perm;
        perm.permissionFlags = msg.permissions[i].permissionFlags;
        perm.userUUID = RexUUID(msg.permissions[i].userUUID);
        entity->permissions.push_back(perm);
    }
    // Set initial components if specified
    for (unsigned i = 0; i < msg.initialComponents.size(); ++i)
    {
        std::string compTypeName = BufferToString(msg.initialComponents[i].componentTypeName);
        std::string compName = BufferToString(msg.initialComponents[i].componentName);
        ComponentPtr comp = entity->GetOrCreateComponent(compTypeName, compName);
        comp->data = msg.initialComponents[i].componentData;
    }
    
    // Send back actionreply
    MsgActionReply replyMsg;
    replyMsg.actionID = msg.actionID;
    replyMsg.entityID = newEntityID;
    entity->entityUUID.ToBuffer(replyMsg.entityUUID);
    replyMsg.success = 1;
    source->connection->Send(replyMsg);
    
    // Replicate new entity to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        ReplicateEntity(&*iter, entity);
}

void ECServer::HandleDeleteEntityMessage(ServerConnection* source, MsgDeleteEntity& msg)
{
    /// \todo Permission check. Does the user have right to delete this entity?
    
    MsgActionReply replyMsg;
    replyMsg.actionID = msg.actionID;
    replyMsg.entityID = msg.entityID;
    RexUUID(msg.entityUUID).ToBuffer(replyMsg.entityUUID);
    replyMsg.success = 0;
    
    EntityPtr entity = scene.GetEntity(msg.entityID, RexUUID(msg.entityUUID));
    if (!entity.ptr())
    {
        // Entity not found, send failed actionreply
        source->connection->Send(replyMsg);
        return;
    }
    
    scene.RemoveEntity(entity);
    
    // Send back successful actionreply
    replyMsg.success = 1;
    source->connection->Send(replyMsg);
    
    // Replicate deletion of entity to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        ReplicateEntityDelete(&*iter, msg.entityID, RexUUID(msg.entityUUID));
}

void ECServer::HandleModifyComponentMessage(ServerConnection* source, MsgModifyComponent& msg)
{
    /// \todo Permission check. Does the user have right to modify components of this entity?
    
    // For convenience, and for the fact that there will be both non-OpenSim & OpenSim-entities, we allow
    // to create an entity (with an user-specified ID) on the fly with this message. Have to set the 
    // creatorUUID in that case, though, and then there is issue of correct default permissions
    
    bool newEntity = false;
    EntityPtr entity = scene.GetEntity(msg.entityID, RexUUID(msg.entityUUID));
    if (!entity.ptr())
    {
        newEntity = true;
        entity = scene.GetOrCreateEntity(msg.entityID);
        entity->entityUUID = RexUUID(msg.entityUUID);
        entity->creatorUUID = source->userUUID;
    }
    
    std::string compTypeName = BufferToString(msg.componentTypeName);
    std::string compName = BufferToString(msg.componentName);
    ComponentPtr comp = entity->GetOrCreateComponent(compTypeName, compName);
    comp->data = msg.componentData;
    
    // Send back actionreply
    MsgActionReply replyMsg;
    replyMsg.actionID = msg.actionID;
    replyMsg.entityID = entity->entityID;
    entity->entityUUID.ToBuffer(replyMsg.entityUUID);
    replyMsg.success = 1;
    source->connection->Send(replyMsg);
    
    // Replicate changed component (or new entity) to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    if (!newEntity)
    {
        for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
            ReplicateComponent(&*iter, comp);
    }
    else
    {
        for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
            ReplicateEntity(&*iter, entity);
    }
}

void ECServer::HandleDeleteComponentMessage(ServerConnection* source, MsgDeleteComponent& msg)
{
    /// \todo Permission check. Does the user have right to delete components from this entity?
    
    MsgActionReply replyMsg;
    replyMsg.actionID = msg.actionID;
    replyMsg.entityID = msg.entityID;
    RexUUID(msg.entityUUID).ToBuffer(replyMsg.entityUUID);
    replyMsg.success = 0;
    
    EntityPtr entity = scene.GetEntity(msg.entityID, RexUUID(msg.entityUUID));
    if (!entity.ptr())
    {
        // Entity not found, send failed actionreply
        source->connection->Send(replyMsg);
        return;
    }
    
    std::string compTypeName = BufferToString(msg.componentTypeName);
    std::string compName = BufferToString(msg.componentName);
    ComponentPtr comp = entity->GetComponent(compTypeName, compName);
    if (!comp.ptr())
    {
        // Component not found, send failed actionreply
        source->connection->Send(replyMsg);
        return;
    }
    
    entity->RemoveComponent(compTypeName, compName);
    
    // Send back successful actionreply
    replyMsg.success = 1;
    source->connection->Send(replyMsg);
    
    // Replicate deletion of component to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        ReplicateComponentDelete(&*iter, msg.entityID, RexUUID(msg.entityUUID), compTypeName, compName);
}

void ECServer::HandleSetPermissionsMessage(ServerConnection* source, MsgSetPermissions& msg)
{
    /// \todo Permission check. Does the user have right to set permissions?
    
    MsgActionReply replyMsg;
    replyMsg.actionID = msg.actionID;
    replyMsg.entityID = msg.entityID;
    RexUUID(msg.entityUUID).ToBuffer(replyMsg.entityUUID);
    replyMsg.success = 0;
    
    EntityPtr entity = scene.GetEntity(msg.entityID, RexUUID(msg.entityUUID));
    if (!entity.ptr())
    {
        // Entity not found, send failed actionreply
        source->connection->Send(replyMsg);
        return;
    }
    
    entity->permissions.clear();
    for (unsigned i = 0; i < msg.permissions.size(); ++i)
    {
        EntityPermission perm;
        perm.permissionFlags = msg.permissions[i].permissionFlags;
        perm.userUUID = RexUUID(msg.permissions[i].userUUID);
        entity->permissions.push_back(perm);
    }

    // Send back actionreply
    replyMsg.success = 1;
    source->connection->Send(replyMsg);
    
    // Replicate new permissions to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        ReplicateEntity(&*iter, entity, true);
}

void ECServer::SendInitialScene(ServerConnection* conn)
{
    for (EntityVector::iterator i = scene.entities.begin(); i != scene.entities.end(); ++i)
        ReplicateEntity(conn, *i);
}

void ECServer::ReplicateEntity(ServerConnection* dest, EntityPtr entity, bool noComponents)
{
    assert(entity.ptr());
    
    MsgEntityUpdated msg;
    msg.entityID = entity->entityID;
    entity->entityUUID.ToBuffer(msg.entityUUID);
    entity->creatorUUID.ToBuffer(msg.creatorUUID);
    
    /// \todo user might not have permission to know permissions?
    /// \todo check that user has right to see this entity
    
    msg.permissions.resize(entity->permissions.size());
    for (unsigned i = 0; i < entity->permissions.size(); ++i)
    {
        entity->permissions[i].userUUID.ToBuffer(msg.permissions[i].userUUID);
        msg.permissions[i].permissionFlags = entity->permissions[i].permissionFlags;
    }
    
    dest->connection->Send(msg);
    
    if (!noComponents)
    {
        for (ComponentVector::iterator i = entity->components.begin(); i != entity->components.end(); ++i)
            ReplicateComponent(dest, *i);
    }
}

void ECServer::ReplicateComponent(ServerConnection* dest, ComponentPtr component)
{
    assert(component.ptr());
    assert(component->parentEntity);
    
    MsgComponentUpdated msg;
    msg.entityID = component->parentEntity->entityID;
    component->parentEntity->entityUUID.ToBuffer(msg.entityUUID);
    msg.componentTypeName = StringToBuffer(component->typeName);
    msg.componentName = StringToBuffer(component->name);
    msg.componentData = component->data;
    
    dest->connection->Send(msg);
}

void ECServer::ReplicateEntityDelete(ServerConnection* dest, u32 entityID, const RexUUID& entityUUID)
{
    MsgEntityDeleted msg;
    msg.entityID = entityID;
    entityUUID.ToBuffer(msg.entityUUID);
    
    dest->connection->Send(msg);
}

void ECServer::ReplicateComponentDelete(ServerConnection* dest, u32 entityID, const RexUUID& entityUUID, const std::string& compTypeName, const std::string& compName)
{
    MsgComponentDeleted msg;
    msg.entityID = entityID;
    entityUUID.ToBuffer(msg.entityUUID);
    msg.componentTypeName = StringToBuffer(compTypeName);
    msg.componentName = StringToBuffer(compName);
    
    dest->connection->Send(msg);
}

void ECServer::CreateNewEntity()
{
    u32 newEntityID = scene.GetNextFreeEntityID();
    EntityPtr entity = scene.GetOrCreateEntity(newEntityID);
    
    // Replicate new entity to everyone
    ServerConnectionList& serverConnections = server->GetConnections();
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        ReplicateEntity(&*iter, entity);
}
