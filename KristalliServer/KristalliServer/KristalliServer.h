#pragma once

#include "IService.h"

struct MsgLogin;

class InputConsole;

#include <set>

/// KristalliServer main class. Maintains connection list & logins and hands off messages to services
class KristalliServer : public IMessageHandler, public INetworkServerListener
{
public:
    KristalliServer();
    ~KristalliServer();
    
    void AddService(IService* service);
    void RemoveService(IService* service);
    
    void RunServer(unsigned short port, SocketTransportLayer transport);
    void NewConnectionEstablished(MessageConnection* source);
    void ClientDisconnected(MessageConnection* source);
    void HandleMessage(MessageConnection* source, message_id_t id, const char *data, size_t numBytes);
    ServerConnection* GetConnection(MessageConnection* connection);
    ServerConnectionList& GetConnections() { return serverConnections; }
    ServiceList& GetServices() { return services; }
    
private:
    void HandleLoginMessage(MessageConnection* source, MsgLogin& msg);
    u8 AllocateNewConnectionID() const;
    void LoadScene();
    void SaveScene();
    void ResetScene();
    
    Network network;
    NetworkServer* server;
    ServiceList services;
    ServerConnectionList serverConnections;
    InputConsole* console;
};

