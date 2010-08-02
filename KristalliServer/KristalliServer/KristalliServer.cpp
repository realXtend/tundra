#include "KristalliServer.h"
#include "KristalliServerMessages.h"
#include "Utilities.h"
#include "InputConsole.h"
#include "ECServer.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientLeft.h"
#include "MsgClientJoined.h"
#include "MsgServerReset.h"

using namespace std;
using namespace clb;

KristalliServer::KristalliServer() :
    server(0),
    console(0)
{
}

KristalliServer::~KristalliServer()
{
    delete console;
    console = 0;
}

void KristalliServer::AddService(IService* service)
{
    services.push_back(service);
}

void KristalliServer::RemoveService(IService* service)
{
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        if (*iter == service)
        {
            services.erase(iter);
            break;
        }
}

void KristalliServer::LoadScene()
{
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        (*iter)->LoadScene();
}

void KristalliServer::SaveScene()
{
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        (*iter)->SaveScene();
}

void KristalliServer::ResetScene()
{
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        (*iter)->ResetScene();
    
    // Send info to client that server was reset
    MsgServerReset msg;
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        iter->connection->Send(msg);
    }
}

void KristalliServer::RunServer(unsigned short port, SocketTransportLayer transport)
{
    console = new InputConsole();
    
    // Let all services load their scene information as necessary
    LoadScene();
    
    // Start the server either in TCP or UDP mode.
    server = network.StartServer(port, transport, this);
    if (!server)
    {
        cout << "Unable to start server in port " << port << "!" << endl;
        return;
    }
    
    cout << "Server waiting for connection in port " << port << "." << endl;
    
    while(1)
    {
        std::string command;
        if (console->ReadInput(command))
        {
            if ((command == "shutdown") || (command == "exit"))
                break;
            
            if (command == "save")
            {
                // Let all services save their scene information as necessary
                cout << "Saving server state" << endl;
                SaveScene();
            }
            
            if (command == "reset")
            {
                cout << "Resetting server state" << endl;
                ResetScene();
            }
            
            if (command == "createentity")
            {
                cout << "Creating new entity to scene" << endl;
                for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
                {
                    ECServer* ecServer = dynamic_cast<ECServer*>(*iter);
                    if (ecServer)
                        ecServer->CreateNewEntity();
                }
            }
        }
        
        server->ProcessMessages();

        // Update each service in turn.
        for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
            (*iter)->Update();

        Sleep(1);
    }
}

void KristalliServer::NewConnectionEstablished(MessageConnection *source)
{
    source->RegisterInboundMessageHandler(this);
    source->SetDatagramInFlowRatePerSecond(200);
    
    cout << "User joined from " << source->GetEndPoint().ToString() << ", waiting for login message" << endl;
}

void KristalliServer::ClientDisconnected(MessageConnection *source)
{
    ServerConnection *connection = GetConnection(source);
    if (!connection)
    {
        cout << "Nonexisting client disconnected!" << endl;
        return;
    }
    
    cout << "User " << connection->userName << " id " << (int)connection->userID << " disconnected." << endl;
    
    // Notify services
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        (*iter)->HandleDisconnect(connection);
    
    // Send "client left" to other participants
    MsgClientLeft msg;
    msg.userID = connection->userID;
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if (&*iter != connection)
            iter->connection->Send(msg);
    }
    
    RexUUID disconnectedUUID = connection->userUUID;
    
    // Erase the connection
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if (&*iter == connection)
        {
            serverConnections.erase(iter);
            break;
        }
    }
}

ServerConnection *KristalliServer::GetConnection(MessageConnection *connection)
{
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if (iter->connection == connection)
            return &*iter;
    }
    
    return 0;
}

u8 KristalliServer::AllocateNewConnectionID() const
{
    u8 newID = 1;
    for(ServerConnectionList::const_iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
        newID = max(newID, iter->userID+1);
    
    return newID;
}

void KristalliServer::HandleMessage(MessageConnection *source, message_id_t id, const char *data, size_t numBytes)
{
    try
    {
        switch (id)
        {
        case cLoginMessage:
            {
                // Use login message to establish connection into connection list
                MsgLogin msg(data, numBytes);
                HandleLoginMessage(source, msg);
            }
            break;
            
        default:
            {
                // All other messages than login must be from authenticated users
                ServerConnection* connection = GetConnection(source);
                if (!connection)
                {
#ifdef _DEBUG
                    cout << "Received message " << (int)id << " from unauthenticated user" << endl;
#endif
                }
                else
                {
                    // Pass message to all services
                    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
                        (*iter)->HandleMessage(connection, id, data, numBytes);
                }
            }
            break;
        }
    }
    catch (exception& e)
    {
        cout << "Exception while handling message " << (int)id << ": " << e.what() << endl;
    }
}

void KristalliServer::HandleLoginMessage(MessageConnection *source, MsgLogin& msg)
{
    string newName = BufferToString(msg.userName);
    RexUUID newUUID(msg.userUUID);
    cout << "Received login message from user " << newName << endl;
    
    /// \todo Authenticate here, for now always succeeds
    
    // Delete the old connection if found with same UUID
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if ((*iter).userUUID == newUUID)
        {
            serverConnections.erase(iter);
            cout << "Erased old connection" << endl;
            break;
        }
    }
    
    // Create new connection
    ServerConnection newConn;
    newConn.connection = source;
    newConn.userName = newName;
    newConn.userUUID = newUUID;
    newConn.userID = AllocateNewConnectionID();
    serverConnections.push_back(newConn);
    cout << "Created new connection, id " << (int)newConn.userID << endl;
    
    // Send loginreply
    MsgLoginReply replyMsg;
    replyMsg.success = 1;
    replyMsg.userID = newConn.userID;
    source->Send(replyMsg);
    
    // Send "client joined" to other participants
    MsgClientJoined joinedMsg;
    joinedMsg.userID = newConn.userID;
    joinedMsg.userName = StringToBuffer(newName);
    newUUID.ToBuffer(joinedMsg.userUUID);
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if (iter->connection != source)
            iter->connection->Send(joinedMsg);
    }
    
    // Send information of existing participants to the joined client
    for(ServerConnectionList::iterator iter = serverConnections.begin(); iter != serverConnections.end(); ++iter)
    {
        if (iter->connection != source)
        {
            MsgClientJoined existingUserMsg;
            existingUserMsg.userID = iter->userID;
            existingUserMsg.userName = StringToBuffer(iter->userName);
            iter->userUUID.ToBuffer(existingUserMsg.userUUID);
            source->Send(existingUserMsg);
        }
    }
    
    // Notify services
    for(ServiceList::iterator iter = services.begin(); iter != services.end(); ++iter)
        (*iter)->HandleConnect(GetConnection(source));
}
