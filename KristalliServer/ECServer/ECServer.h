#pragma once

#include "clb/Network/KristalliProtocol.h"
#include "IService.h"
#include "RexUUID.h"
#include "Scene.h"

struct MsgCreateEntity;
struct MsgDeleteEntity;
struct MsgModifyComponent;
struct MsgDeleteComponent;
struct MsgSetPermissions;

/// ECServer service. Handles EC replication
class ECServer : public IService
{
public:
    ECServer(KristalliServer* server);
    ~ECServer();
    
    virtual void HandleConnect(ServerConnection* connection);
    virtual void HandleDisconnect(ServerConnection* connection);
    virtual void HandleMessage(ServerConnection* source, message_id_t id, const char *data, size_t numBytes);
    virtual void LoadScene();
    virtual void SaveScene();
    virtual void ResetScene();
    void CreateNewEntity();
    
private:
    void SendInitialScene(ServerConnection* dest);
    void ReplicateEntity(ServerConnection* dest, EntityPtr entity, bool noComponents = false);
    void ReplicateComponent(ServerConnection* dest, ComponentPtr component);
    void ReplicateEntityDelete(ServerConnection* dest, u32 entityID, const RexUUID& entityUUID);
    void ReplicateComponentDelete(ServerConnection* dest, u32 entityID, const RexUUID& entityUUID, const std::string& compTypeName, const std::string& compName);
    void HandleCreateEntityMessage(ServerConnection* source, MsgCreateEntity& msg);
    void HandleDeleteEntityMessage(ServerConnection* source, MsgDeleteEntity& msg);
    void HandleModifyComponentMessage(ServerConnection* source, MsgModifyComponent& msg);
    void HandleDeleteComponentMessage(ServerConnection* source, MsgDeleteComponent& msg);
    void HandleSetPermissionsMessage(ServerConnection* source, MsgSetPermissions& msg);
    EntityPtr LookUpEntity(u32 entityID, RexUUID entityUUID);
    
    KristalliServer* server;
    Scene scene;
};
