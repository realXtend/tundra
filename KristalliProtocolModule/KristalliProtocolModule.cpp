// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "DebugOperatorNew.h"

#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "Profiler.h"
#include "EventManager.h"
#include "CoreStringUtils.h"
#include "ConsoleServiceInterface.h"
#include "ConsoleCommandServiceInterface.h"

#include "UiAPI.h"
#include "NaaliMainWindow.h"
#include "kNet.h"
#include "kNet/qt/NetworkDialog.h"
#include "LoggingFunctions.h"

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
    kNet::SetLogChannels(kNet::LogInfo | kNet::LogError | kNet::LogUser); // Enable all log channels.
}

void KristalliProtocolModule::Initialize()
{
    EventManagerPtr event_manager = framework_->GetEventManager();
    networkEventCategory = event_manager->RegisterEventCategory("Kristalli");

    defaultTransport = kNet::SocketOverTCP;
    const boost::program_options::variables_map &options = framework_->ProgramOptions();
    if (options.count("protocol") > 0)
        if (QString(options["protocol"].as<std::string>().c_str()).trimmed().toLower() == "udp")
            defaultTransport = kNet::SocketOverUDP;
}

void KristalliProtocolModule::PostInitialize()
{
    RegisterConsoleCommand(Console::CreateCommand(
            "kNet", "Shows the kNet statistics window.", 
            Console::Bind(this, &KristalliProtocolModule::OpenKNetLogWindow)));
}

void KristalliProtocolModule::Uninitialize()
{
    Disconnect();
}

Console::CommandResult KristalliProtocolModule::OpenKNetLogWindow(const StringVector &)
{
    NetworkDialog *networkDialog = new NetworkDialog(0, &network);
    networkDialog->setAttribute(Qt::WA_DeleteOnClose);
    networkDialog->show();

    return Console::ResultSuccess();
}

void KristalliProtocolModule::Update(f64 frametime)
{
    // Pulls all new inbound network messages and calls the message handler we've registered
    // for each of them.
    if (serverConnection)
        serverConnection->Process();

    // Note: Calling the above serverConnection->Process() may set serverConnection to null if the connection gets disconnected.
    // Therefore, in the code below, we cannot assume serverConnection is non-null, and must check it again.

    // Our client->server connection is never kept partially open.
    // That is, at the moment the server write-closes the connection, we also write-close the connection.
    // Check here if the server has write-closed, and also write-close our end if so.
    if (serverConnection && !serverConnection->IsReadOpen() && serverConnection->IsWriteOpen())
        serverConnection->Disconnect(0);
    
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
                ::LogInfo("Failed to connect to " + serverIp + ":" + ToString(serverPort));
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
    if (Connected() && serverConnection && serverConnection->RemoteEndPoint().IPToString() != serverIp)
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
        ::LogError("Unable to connect to " + serverIp + ":" + ToString(serverPort));
        return;
    }

    // For TCP mode sockets, set the TCP_NODELAY option to improve latency for the messages we send.
    if (serverConnection->GetSocket() && serverConnection->GetSocket()->TransportLayer() == kNet::SocketOverTCP)
        serverConnection->GetSocket()->SetNaglesAlgorithmEnabled(false);
}

void KristalliProtocolModule::Disconnect()
{
    // Clear the remembered destination server ip address so that the automatic connection timer will not try to reconnect.
    serverIp = "";
    reconnectTimer.Stop();
    
    if (serverConnection)
    {
        serverConnection->Disconnect();
//        network.CloseMessageConnection(serverConnection);
        ///\todo Wait? This closes the connection.
        serverConnection = 0;
    }


}

bool KristalliProtocolModule::StartServer(unsigned short port, SocketTransportLayer transport)
{
    StopServer();
    
    const bool allowAddressReuse = true;
    server = network.StartServer(port, transport, this, allowAddressReuse);
    if (!server)
    {
        ::LogError("Failed to start server on port " + ToString((int)port));
        throw Exception(("Failed to start server on port " + ToString((int)port) + ". Please make sure that the port is free and not used by another application. The program will now abort.").c_str());
    }
    
    ::LogInfo("Server started");
    ::LogInfo(QString("* Port     : ") + QString::number(port));
    ::LogInfo(QString("* Protocol : ") + (transport == kNet::SocketOverUDP ? "UDP" : "TCP"));
    ::LogInfo(QString("* Headless : ") + (framework_->IsHeadless() == true ? "True" : "False"));
    return true;
}

void KristalliProtocolModule::StopServer()
{
    if (server)
    {
        network.StopServer();
        connections.clear();
        ::LogInfo("Server stopped");
        server = 0;
    }
}

void KristalliProtocolModule::NewConnectionEstablished(kNet::MessageConnection *source)
{
    assert(source);
    if (!source)
        return;

    source->RegisterInboundMessageHandler(this);
    ///\todo Regression. Re-enable. -jj.
//    source->SetDatagramInFlowRatePerSecond(200);
    
    UserConnection* connection = new UserConnection();
    connection->userID = AllocateNewConnectionID();
    connection->connection = source;
    connections.push_back(connection);

    // For TCP mode sockets, set the TCP_NODELAY option to improve latency for the messages we send.
    if (source->GetSocket() && source->GetSocket()->TransportLayer() == kNet::SocketOverTCP)
        source->GetSocket()->SetNaglesAlgorithmEnabled(false);

    ::LogInfo("User connected from " + source->RemoteEndPoint().ToString() + ", connection ID " + ToString((int)connection->userID));
    
    Events::KristalliUserConnected msg(connection);
    framework_->GetEventManager()->SendEvent(networkEventCategory, Events::USER_CONNECTED, &msg);
}

void KristalliProtocolModule::ClientDisconnected(MessageConnection *source)
{
    // Delete from connection list if it was a known user
    for(UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if ((*iter)->connection == source)
        {
            Events::KristalliUserDisconnected msg((*iter));
            framework_->GetEventManager()->SendEvent(networkEventCategory, Events::USER_DISCONNECTED, &msg);
            
            ::LogInfo("User disconnected, connection ID " + ToString((int)(*iter)->userID));
            delete(*iter);
            connections.erase(iter);
            return;
        }

        ::LogInfo("Unknown user disconnected");
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
        ::LogError("KristalliProtocolModule: Exception \"" + std::string(e.what()) + "\" thrown when handling network message id " +
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
        newID = std::max((int)newID, (int)((*iter)->userID+1));
    
    return newID;
}

UserConnection* KristalliProtocolModule::GetUserConnection(MessageConnection* source)
{
    for(UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if ((*iter)->connection == source)
            return (*iter);

    return 0;
}

UserConnection* KristalliProtocolModule::GetUserConnection(u8 id)
{
    for(UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if ((*iter)->userID == id)
            return (*iter);

    return 0;
}

} // ~KristalliProtocolModule namespace

void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Foundation::Framework *fw)
{
    IModule *module = new KristalliProtocol::KristalliProtocolModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
