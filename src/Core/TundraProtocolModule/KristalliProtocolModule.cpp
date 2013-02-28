// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "KristalliProtocolModule.h"

#include "Profiler.h"
#include "CoreStringUtils.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "ConsoleAPI.h"
#include "LoggingFunctions.h"
#include "CoreException.h"

#include <kNet.h>
#include <kNet/UDPMessageConnection.h>

#include <algorithm>
#include <utility>

#include "MemoryLeakCheck.h"

using namespace kNet;

namespace
{

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

static const int cInitialAttempts = 0;
static const int cReconnectAttempts = 0;

KristalliProtocolModule::KristalliProtocolModule() :
    IModule("KristalliProtocol"),
    serverConnection(0),
    server(0),
    reconnectAttempts(0),
    connectionPending(false),
    serverPort(0)
#ifdef KNET_USE_QT
    ,networkDialog(0)
#endif
{
}

KristalliProtocolModule::~KristalliProtocolModule()
{
    Disconnect();
#ifdef KNET_USE_QT
    SAFE_DELETE(networkDialog);
#endif
}

void KristalliProtocolModule::Load()
{
    // Enable log channels according to Tundras --loglevel input param.
    // For 'info' that is the most usable mode remove some of the spam from kNet.
    // If the log level is not given default to more information (even if default is 'info').
    // @todo Remove kNet::LogInfo from the default mode when the info is no longer needed for tundra console.
    if (GetFramework()->HasCommandLineParameter("--loglevel"))
    {
        QString logLevel = GetFramework()->CommandLineParameters("--loglevel").first().toLower();
        if (logLevel == "info")
            kNet::SetLogChannels(kNet::LogError | kNet::LogUser);
        else if (logLevel == "debug")
            kNet::SetLogChannels(kNet::LogInfo | kNet::LogError | kNet::LogUser | kNet::LogVerbose); 
        else // info
            kNet::SetLogChannels(kNet::LogInfo | kNet::LogError | kNet::LogUser); 
    }
    // Enable all log channels.
    else
        kNet::SetLogChannels(kNet::LogInfo | kNet::LogError | kNet::LogUser); 
}

void KristalliProtocolModule::Unload()
{
    Disconnect();
}

void KristalliProtocolModule::Initialize()
{
    defaultTransport = kNet::SocketOverUDP;
    QStringList cmdLineParams = framework_->CommandLineParameters("--protocol");
    if (cmdLineParams.size() > 0)
    {
        kNet::SocketTransportLayer transportLayer = StringToSocketTransportLayer(cmdLineParams.first().trimmed().toStdString().c_str());
        if (transportLayer != InvalidTransportLayer)
            defaultTransport = transportLayer;
    }
#ifdef KNET_USE_QT
    framework_->Console()->RegisterCommand("kNet", "Shows the kNet statistics window.", this, SLOT(OpenKNetLogWindow()));
#endif
}

void KristalliProtocolModule::Uninitialize()
{
    Disconnect();
}

void KristalliProtocolModule::OpenKNetLogWindow()
{
#ifdef KNET_USE_QT
    if (!networkDialog)
    {
        networkDialog = new NetworkDialog(0, &network);
        networkDialog->setAttribute(Qt::WA_DeleteOnClose);
    }
    networkDialog->show();
#else
    ::LogError("Cannot open kNet logging window - kNet was not built with Qt enabled!");
#endif
}

void KristalliProtocolModule::Update(f64 /*frametime*/)
{
    // Pulls all new inbound network messages and calls the message handler we've registered
    // for each of them.
    if (serverConnection)
    {
        PROFILE(KristalliProtocolModule_kNet_client_Process);
        serverConnection->Process();
    }

    // Note: Calling the above serverConnection->Process() may set serverConnection to null if the connection gets disconnected.
    // Therefore, in the code below, we cannot assume serverConnection is non-null, and must check it again.

    // Our client->server connection is never kept half-open.
    // That is, at the moment the server write-closes the connection, we also write-close the connection.
    // Check here if the server has write-closed, and also write-close our end if so.
    if (serverConnection && !serverConnection->IsReadOpen() && serverConnection->IsWriteOpen())
        serverConnection->Disconnect(0);
    
    // Process server incoming connections & messages if server up
    if (server)
    {
        PROFILE(KristalliProtocolModule_kNet_server_Process);

        server->Process();

        // In Tundra, we *never* keep half-open server->client connections alive. 
        // (the usual case would be to wait for a file transfer to complete, but Tundra messaging mechanism doesn't use that).
        // So, bidirectionally close all half-open connections.
        NetworkServer::ConnectionMap connections = server->GetConnections();
        for(NetworkServer::ConnectionMap::iterator iter = connections.begin(); iter != connections.end(); ++iter)
            if (!iter->second->IsReadOpen() && iter->second->IsWriteOpen())
                iter->second->Disconnect(0);
    }
    
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
                ::LogInfo(QString("Failed to connect to %1:%2").arg(serverIp.c_str()).arg(serverPort));
                emit ConnectionAttemptFailed();

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
        ::LogInfo(QString("Unable to connect to %1:%2").arg(serverIp.c_str()).arg(serverPort));
        return;
    }

    if (serverTransport == kNet::SocketOverUDP)
        dynamic_cast<kNet::UDPMessageConnection*>(serverConnection.ptr())->SetDatagramSendRate(500);

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
        const QString error = "Failed to start server on port " + QString::number(port) + ".";
        ::LogError(error);
        throw Exception((error + "Please make sure that the port is free and not used by another application. The program will now abort.").toStdString().c_str());
    }
    
    std::cout << std::endl;
    ::LogInfo("Server started");
    ::LogInfo("* Port     : " + QString::number(port));
    ::LogInfo("* Protocol : " + SocketTransportLayerToString(transport));
    ::LogInfo("* Headless : " + BoolToString(framework_->IsHeadless()));
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

    if (dynamic_cast<kNet::UDPMessageConnection*>(source))
        dynamic_cast<kNet::UDPMessageConnection*>(source)->SetDatagramSendRate(500);

    source->RegisterInboundMessageHandler(this);
    
    UserConnectionPtr connection = MAKE_SHARED(UserConnection);
    connection->userID = AllocateNewConnectionID();
    connection->connection = source;
    connections.push_back(connection);

    // For TCP mode sockets, set the TCP_NODELAY option to improve latency for the messages we send.
    if (source->GetSocket() && source->GetSocket()->TransportLayer() == kNet::SocketOverTCP)
        source->GetSocket()->SetNaglesAlgorithmEnabled(false);

    ::LogInfo(QString("User connected from %1, connection ID %2.").arg(source->RemoteEndPoint().ToString().c_str()).arg(connection->userID));

    emit ClientConnectedEvent(connection.get());
}

void KristalliProtocolModule::ClientDisconnected(MessageConnection *source)
{
    // Delete from connection list if it was a known user
    for(UserConnectionList::iterator iter = connections.begin(); iter != connections.end(); ++iter)
    {
        if ((*iter)->connection == source)
        {
            emit ClientDisconnectedEvent(iter->get());
            
            ::LogInfo("User disconnected, connection ID " + QString::number((*iter)->userID));
            connections.erase(iter);
            return;
        }
    }
    ::LogWarning("User without a known UserConnection* disconnected!");
}

void KristalliProtocolModule::HandleMessage(kNet::MessageConnection *source, kNet::packet_id_t packetId, kNet::message_id_t messageId, const char *data, size_t numBytes)
{
    assert(source);
    assert(data || numBytes == 0);

    try
    {
        emit NetworkMessageReceived(source, packetId, messageId, data, numBytes);
    }
    catch(std::exception &e)
    {
        ::LogError("KristalliProtocolModule: Exception \"" + QString(e.what()) + "\" thrown when handling network message id " +
            QString::number(messageId) + " size " + QString::number(numBytes) + " from client " + source->ToString().c_str());

        // Kill the connection. For debugging purposes, don't disconnect the client if the server is running a debug build.
#ifndef _DEBUG
        source->Disconnect(0);
        source->Close(0);
        // kNet will call back to KristalliProtocolModule::ClientDisconnected() to clean up the high-level Tundra UserConnection object.
#endif
    }
}

u32 KristalliProtocolModule::AllocateNewConnectionID() const
{
    u32 newID = 1;
    for(UserConnectionList::const_iterator iter = connections.begin(); iter != connections.end(); ++iter)
        newID = std::max((u32)newID, (u32)((*iter)->userID+1));
    
    return newID;
}

UserConnectionPtr KristalliProtocolModule::GetUserConnection(MessageConnection* source) const
{
    for(UserConnectionList::const_iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if ((*iter)->connection == source)
            return *iter;

    return UserConnectionPtr();
}

UserConnectionPtr KristalliProtocolModule::GetUserConnection(u32 id) const
{
    for(UserConnectionList::const_iterator iter = connections.begin(); iter != connections.end(); ++iter)
        if ((*iter)->userID == id)
            return *iter;

    return UserConnectionPtr();
}
