// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "Profiler.h"
#include "EventManager.h"
#include "CoreStringUtils.h"

#include <algorithm>

using namespace kNet;

namespace KristalliProtocol
{

namespace
{
    const std::string moduleName("KristalliProtocol");

/*
    const struct
    {
        SocketTransportLayer transport;
        int portNumber;
    } destinationPorts[] = 
    {
        { SocketOverUDP, 2345 }, // The default Kristalli over UDP port.

        { SocketOverTCP, 2345 }, // The default Kristalli over TCP port.
        { SocketOverUDP, 123 }, // Network Time Protocol.

        { SocketOverTCP, 80 }, // HTTP.
        { SocketOverTCP, 443 }, // HTTPS.
        { SocketOverTCP, 20 }, // FTP Data.
        { SocketOverTCP, 21 }, // FTP Control.
        { SocketOverTCP, 22 }, // SSH.
        { SocketOverTCP, 23 }, // TELNET.
        { SocketOverUDP, 25 }, // SMTP. (Microsoft)
        { SocketOverTCP, 25 }, // SMTP.
        { SocketOverTCP, 110 }, // POP3 Server listen port.
        { SocketOverTCP, 995 }, // POP3 over SSL.
        { SocketOverTCP, 109 }, // POP2.
        { SocketOverTCP, 6667 }, // IRC.

        // For more info on the following windows ports, see: http://support.microsoft.com/kb/832017

        { SocketOverTCP, 135 }, // Windows RPC.
        { SocketOverUDP, 137 }, // Windows Cluster Administrator. / NetBIOS Name Resolution.
        { SocketOverUDP, 138 }, // Windows NetBIOS Datagram Service.
        { SocketOverTCP, 139 }, // Windows NetBIOS Session Service.

        { SocketOverUDP, 389 }, // Windows LDAP Server.
        { SocketOverTCP, 389 }, // Windows LDAP Server.

        { SocketOverTCP, 445 }, // Windows SMB.

        { SocketOverTCP, 5722 }, // Windows RPC.

        { SocketOverTCP, 993 }, // IMAP over SSL.

//        { SocketOverTCP, 1433 }, // SQL over TCP.
//        { SocketOverUDP, 1434 }, // SQL over UDP.

        { SocketOverUDP, 53 }, // DNS.
        { SocketOverTCP, 53 }, // DNS. Microsoft states it uses TCP 53 for DNS as well.
        { SocketOverUDP, 161 }, // SNMP agent port.
        { SocketOverUDP, 162 }, // SNMP manager port.
        { SocketOverUDP, 520 }, // RIP.
        { SocketOverUDP, 67 }, // DHCP client->server.
        { SocketOverUDP, 68 }, // DHCP server->client.
    };

    /// The number of different port choices to try from the list.
    const int cNumPortChoices = sizeof(destinationPorts) / sizeof(destinationPorts[0]);
*/
}

static const int cInitialAttempts = 1;
static const int cReconnectAttempts = 5;

KristalliProtocolModule::KristalliProtocolModule()
:IModule(NameStatic())
, serverConnection(0)
, server(0)
, reconnectAttempts(0)
{
}

KristalliProtocolModule::~KristalliProtocolModule()
{
    Disconnect();
}

void KristalliProtocolModule::Load()
{
}

void KristalliProtocolModule::Unload()
{
    Disconnect();
}

void KristalliProtocolModule::PreInitialize()
{
}

void KristalliProtocolModule::Initialize()
{
    Foundation::EventManagerPtr event_manager = framework_->GetEventManager();

    networkEventCategory = event_manager->RegisterEventCategory("Kristalli");
    event_manager->RegisterEvent(networkEventCategory, Events::NETMESSAGE_IN, "NetMessageIn");
}

void KristalliProtocolModule::PostInitialize()
{
}

void KristalliProtocolModule::Uninitialize()
{
    Disconnect();
}

void KristalliProtocolModule::Update(f64 frametime)
{
    // Pulls all new inbound network messages and calls the message handler we've registered
    // for each of them.
    if (serverConnection)
        serverConnection->Process();
    
    // Process server incoming connections & messages if server up
    if (server)
        server->Process();
    
    if ((!serverConnection || serverConnection->GetConnectionState() == ConnectionClosed ||
        serverConnection->GetConnectionState() == ConnectionPending) && serverIp.length() != 0)
    {
        const int cReconnectTimeout = 5 * 1000.f;
        if (reconnectTimer.Test())
        {
            if (reconnectAttempts)
            {
                PerformConnection();
                --reconnectAttempts;
            }
            else
            {
                LogInfo("Failed to connect to " + serverIp + ":" + ToString(serverPort));
                framework_->GetEventManager()->SendEvent(networkEventCategory, Events::CONNECTION_FAILED, 0);
                reconnectTimer.Stop();
                serverIp = "";
            }
        }
        else if (!reconnectTimer.Enabled())
            reconnectTimer.StartMSecs(cReconnectTimeout);
    }

    // If connection was made, enable a larger number of reconnection attempts in case it gets lost
    if (serverConnection && serverConnection->GetConnectionState() == ConnectionOK)
        reconnectAttempts = cReconnectAttempts;
    
    RESETPROFILER;
}

const std::string &KristalliProtocolModule::NameStatic()
{
    return moduleName;
}

void KristalliProtocolModule::Connect(const char *ip, unsigned short port, SocketTransportLayer transport)
{
    if (Connected() && serverConnection && serverConnection->GetEndPoint().ToString() != serverIp)
        Disconnect();
    
    serverIp = ip;
    serverPort = port;
    serverTransport = transport;
    reconnectAttempts = cInitialAttempts; // Initial attempts when establishing connection
    
    if (!Connected())
        PerformConnection(); // Start performing a connection attempt to the desired address/port/transport
}

void KristalliProtocolModule::PerformConnection()
{
    if (Connected() && serverConnection)
    {
        serverConnection->Close();
//        network.CloseMessageConnection(serverConnection);
        serverConnection = 0;
    }

    // Connect to the server.
    serverConnection = network.Connect(serverIp.c_str(), serverPort, serverTransport, this);
    if (!serverConnection)
    {
        LogError("Unable to connect to " + serverIp + ":" + ToString(serverPort));
        return;
    }
}

void KristalliProtocolModule::Disconnect()
{
    if (serverConnection)
    {
        serverConnection->Disconnect();
//        network.CloseMessageConnection(serverConnection);
        ///\todo Wait? This closes the connection.
        serverConnection = 0;
    }

    // Clear the remembered destination server ip address so that the automatic connection timer will not try to reconnect.
    serverIp = "";
    
    reconnectTimer.Stop();
}

bool KristalliProtocolModule::StartServer(unsigned short port, SocketTransportLayer transport)
{
    StopServer();
    
    server = network.StartServer(port, transport, this);
    if (!server)
    {
        LogError("Failed to start server on port " + ToString((int)port));
        throw Exception(("Failed to start server on port " + ToString((int)port) + ". Please make sure that the port is free and not used by another application. The program will now abort.").c_str());
    }
    
    LogInfo("Started server on port " + ToString((int)port));
    return true;
}

void KristalliProtocolModule::StopServer()
{
    if (server)
    {
        network.StopServer();
        connections.clear();
        LogInfo("Stopped server");
        server = 0;
    }
}

void KristalliProtocolModule::NewConnectionEstablished(kNet::MessageConnection *source)
{
    source->RegisterInboundMessageHandler(this);
    ///\todo Regression. Re-enable. -jj.
//    source->SetDatagramInFlowRatePerSecond(200);
    
    UserConnection connection;
    connection.userID = AllocateNewConnectionID();
    connection.connection = source;
    connections.push_back(connection);
    
    LogInfo("User connected from " + source->GetEndPoint().ToString() + ", connection ID " + ToString((int)connection.userID));
    
    Events::KristalliUserConnected msg(&connection);
    framework_->GetEventManager()->SendEvent(networkEventCategory, Events::USER_CONNECTED, &msg);
}

void KristalliProtocolModule::ClientDisconnected(MessageConnection *source)
{
    // Delete from connection list if it was a known user
    for(UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if (iter->connection == source)
        {
            Events::KristalliUserDisconnected msg(&(*iter));
            framework_->GetEventManager()->SendEvent(networkEventCategory, Events::USER_DISCONNECTED, &msg);
            
            LogInfo("User disconnected, connection ID " + ToString((int)iter->userID));
            connections.erase(iter);
            return;
        }

    LogInfo("Unknown user disconnected");
}

void KristalliProtocolModule::HandleMessage(MessageConnection *source, message_id_t id, const char *data, size_t numBytes)
{
    assert(source);
    assert(data);

    try
    {
        Events::KristalliNetMessageIn msg(source, id, data, numBytes);

        framework_->GetEventManager()->SendEvent(networkEventCategory, Events::NETMESSAGE_IN, &msg);
    } catch(std::exception &e)
    {
        LogError("KristalliProtocolModule: Exception \"" + std::string(e.what()) + "\" thrown when handling network message id " +
            ToString(id) + " size " + ToString((int)numBytes) + " from client " + source->ToString());
    }
}

bool KristalliProtocolModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data)
{
    return false;
}

u8 KristalliProtocolModule::AllocateNewConnectionID() const
{
    u8 newID = 1;
    for(UserConnectionList::const_iterator iter = connections.begin(); iter != connections.end(); ++iter)
        newID = std::max((int)newID, (int)(iter->userID+1));
    
    return newID;
}

UserConnection* KristalliProtocolModule::GetUserConnection(MessageConnection* source)
{
    for (UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if (iter->connection == source)
            return &(*iter);

    return 0;
}

UserConnection* KristalliProtocolModule::GetUserConnection(u8 id)
{
    for (UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if (iter->userID == id)
            return &(*iter);

    return 0;
}

UserConnectionList KristalliProtocolModule::GetAuthenticatedUsers() const
{
    UserConnectionList ret;
    for (UserConnectionList::const_iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if (iter->authenticated)
            ret.push_back(*iter);

    return ret;
}

} // ~KristalliProtocolModule namespace

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace KristalliProtocol;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(KristalliProtocolModule)
POCO_END_MANIFEST
