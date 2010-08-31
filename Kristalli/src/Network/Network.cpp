/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#include <string>

#include <cassert>
//#include "clb/Core/Logging.h"

#include "clb/Network/Network.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

const int cMaxTCPSendSize = 256 * 1024;
const int cMaxUDPSendSize = 1400;

std::string GetWSAErrorString(int error)
{
	void *lpMsgBuf = 0;

	HRESULT hresult = HRESULT_FROM_WIN32(error);
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_IGNORE_INSERTS,
		0, hresult, 0 /*Default language*/,
		(LPTSTR) &lpMsgBuf, 0, 0);

	// Copy message to C++ -style string, since the data need to be freed before return.
	std::string message = (LPCSTR)lpMsgBuf;

	LocalFree(lpMsgBuf);
	return message;
}

std::string GetLastWSAErrorString()
{
	return GetWSAErrorString(WSAGetLastError());
}

std::string FormatBytes(size_t numBytes)
{
	char str[256];
	if (numBytes >= 1000 * 1000)
		sprintf(str, "%.3fMB", (float)numBytes / 1024.f / 1024.f);
	else if (numBytes >= 1000)
		sprintf(str, "%.3fKB", (float)numBytes / 1024.f);
	else
		sprintf(str, "%dB", (int)numBytes);
	return std::string(str);
}

Network::Network()
{
	memset(&wsaData, 0, sizeof(wsaData));
	Init();
}

Network::~Network()
{
	StopServer();
	DeInit();
}

void PrintLocalIP()
{
    char ac[80];
    if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR)
	 {
        LOG("Error getting local host name!");
        return;
    }
    LOG("Host name is %s", ac);

    struct hostent *phe = gethostbyname(ac);
    if (phe == 0) {
        LOG("Bad host lookup.");
        return;
    }

    for (int i = 0; phe->h_addr_list[i] != 0; ++i) {
        struct in_addr addr;
        memcpy(&addr, phe->h_addr_list[i], sizeof(struct in_addr));
        LOG("Address %d: %s", i, inet_ntoa(addr));
    }
}

void Network::PrintAddrInfo(const addrinfo *ptr)
{
	if (!ptr)
	{
		LOG("(Null pointer passed to Network::PrintAddrInfo!)");
		return;
	}

	LOG("\tFlags: 0x%x\n", ptr->ai_flags);
	LOG("\tFamily: ");
	switch(ptr->ai_family)
	{
	case AF_UNSPEC:
		LOG("Unspecified\n");
		break;
	case AF_INET:
		LOG("AF_INET (IPv4)\n");
		break;
	case AF_INET6:
		LOG("AF_INET6 (IPv6)\n");
		break;
	case AF_NETBIOS:
		LOG("AF_NETBIOS (NetBIOS)\n");
		break;
	default:
		LOG("Other %u\n", ptr->ai_family);
		break;
	}
	LOG("\tSocket type: ");
	switch(ptr->ai_socktype)
	{
	case 0:
		LOG("Unspecified\n");
		break;
	case SOCK_STREAM:
		LOG("SOCK_STREAM (stream)\n");
		break;
	case SOCK_DGRAM:
		LOG("SOCK_DGRAM (datagram) \n");
		break;
	case SOCK_RAW:
		LOG("SOCK_RAW (raw) \n");
		break;
	case SOCK_RDM:
		LOG("SOCK_RDM (reliable message datagram)\n");
		break;
	case SOCK_SEQPACKET:
		LOG("SOCK_SEQPACKET (pseudo-stream packet)\n");
		break;
	default:
		LOG("Other %u\n", ptr->ai_socktype);
		break;
	}
	LOG("\tProtocol: ");
	switch(ptr->ai_protocol)
	{
	case 0:
		LOG("Unspecified\n");
		break;
	case IPPROTO_TCP:
		LOG("IPPROTO_TCP (TCP)\n");
		break;
	case IPPROTO_UDP:
		LOG("IPPROTO_UDP (UDP) \n");
		break;
	default:
		LOG("Other %u\n", ptr->ai_protocol);
		break;
	}
	LOG("\tLength of this sockaddr: %d\n", ptr->ai_addrlen);
	LOG("\tCanonical name: %s\n", ptr->ai_canonname);

	char address[256];
	sprintf(address, "%d.%d.%d.%d",
		(unsigned int)(unsigned char)ptr->ai_addr->sa_data[2], (unsigned int)(unsigned char)ptr->ai_addr->sa_data[3],
		(unsigned int)(unsigned char)ptr->ai_addr->sa_data[4], (unsigned int)(unsigned char)ptr->ai_addr->sa_data[5]);

//		char portStr[256];
//		sprintf(portStr, "%d", (unsigned long)clientAddr.sin_port);
	LOG("Address of this sockaddr: %s.\n", address);

}

void Network::PrintHostNameInfo(const char *hostname, const char *port)
{
	DWORD dwRetval;

	int i = 1;

	addrinfo hints;

	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	ZeroMemory( &hints, sizeof(hints) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//--------------------------------
	// Call getaddrinfo(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfo structures containing response
	// information
	addrinfo *result = NULL;
	dwRetval = getaddrinfo(hostname, port, &hints, &result);
	if (dwRetval != 0)
	{
		LOG("getaddrinfo failed with error: %d\n", dwRetval);
		return;
	}

	LOG("getaddrinfo returned success\n");

	// Retrieve each address and print out the hex bytes
	for (addrinfo *ptr = result; ptr != NULL; ptr = ptr->ai_next)
	{
		LOG("getaddrinfo response %d\n", i++);
		PrintAddrInfo(ptr);
	}

	freeaddrinfo(result);

	PrintLocalIP();
}

void Network::Init()
{
	// Initialize Winsock
	int result = WSAStartup(MAKEWORD(2,2), &wsaData);
	if (result != 0)
	{
		LOG("WSAStartup failed: %s(%d)", GetWSAErrorString(result).c_str(), result);
		return;
	}

	char str[256];
	gethostname(str, 256);
	machineIP = str;
//	LOG("gethostname %s", str);
}

NetworkServer *Network::StartServer(unsigned short port, SocketTransportLayer transport, INetworkServerListener *serverListener)
{
	addrinfo *result = NULL;
	addrinfo *ptr = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = (transport == SocketOverTCP) ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = (transport == SocketOverTCP) ? IPPROTO_TCP : IPPROTO_UDP;

	char strPort[256];
	sprintf(strPort, "%d", (unsigned int)port);

	int ret = getaddrinfo(NULL, strPort, &hints, &result);
	if (ret != 0)
	{
		LOG("getaddrinfo failed: %s(%d)", GetWSAErrorString(ret).c_str(), ret);
		return 0;
	}

	SOCKET listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (listenSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		LOG("Error at socket(): %s(%u)", GetWSAErrorString(error).c_str(), error);
		freeaddrinfo(result);
		return 0;
	}
/*
	// Allow other sockets to be bound to this address after this. 
	// (Possibly unsecure, only enable for development purposes - to avoid having to wait for the server listen socket 
	//  to time out if the server crashes.)
	BOOL val = TRUE;
	ret = setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&val, sizeof(val));
	if (ret != 0)
	{
		int error = WSAGetLastError();
		LOG("Warning: setsockopt to SO_REUSEADDR failed: %s(%u)", GetWSAErrorString(error).c_str(), error);
	}
*/
	// Setup the listening socket - bind it to a port.
	// If we are setting up a TCP socket, the socket will be only for listening and accepting incoming connections.
	// If we are setting up an UDP socket, all connection initialization and data transfers will be managed through this socket.
	ret = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		LOG("bind failed: %s(%u)", GetWSAErrorString(error).c_str(), error);
		closesocket(listenSocket);
		freeaddrinfo(result);
		return 0;
	}

	freeaddrinfo(result);

	// For a reliable TCP socket, start the server with a call to listen().
	if (transport == SocketOverTCP)
	{
		// Transition the bound socket to a listening state.
		ret = listen(listenSocket, SOMAXCONN);
		if (ret == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			LOG( "Error at listen(): %s(%u)", GetWSAErrorString(error).c_str(), error);
			closesocket(listenSocket);
			return 0;//Socket();
		}
	}

	const size_t maxSendSize = (transport == SocketOverTCP ? cMaxTCPSendSize : cMaxUDPSendSize);
	sockets.push_back(Socket(listenSocket, "", port, transport, maxSendSize));
	Socket *listenSock = &sockets.back();
	listenSock->SetBlocking(false);

	server = new NetworkServer(this, listenSock, transport);
	server->RegisterServerListener(serverListener);

	LOG("Server up (%s). Waiting for client to connect...", listenSock->ToString().c_str());

	return server;
}

void Network::StopServer()
{
	///\todo This is a forceful stop. Perhaps have a benign teardown as well?
	server = 0;
}

void Network::DeInit()
{
	while(messageConnections.size() > 0)
		CloseMessageConnection(messageConnections.front());

	while(sockets.size() > 0)
	{
		sockets.front().Disconnect();
		sockets.pop_front();
	}
	WSACleanup();
}

MessageConnection *Network::Connect(const char *address, unsigned short port, SocketTransportLayer transport, IMessageHandler *messageHandler)
{
	addrinfo *result = NULL;
	addrinfo *ptr = NULL;
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = (transport == SocketOverTCP) ? SOCK_STREAM : SOCK_DGRAM;
	hints.ai_protocol = (transport == SocketOverTCP) ? IPPROTO_TCP : IPPROTO_UDP;

	char strPort[256];
	sprintf(strPort, "%d", (unsigned int)port);
	int ret = getaddrinfo(address, strPort, &hints, &result);
	if (ret != 0)
	{
		LOG("Network::Connect: getaddrinfo failed: %s(%d)", GetWSAErrorString(ret).c_str(), ret);
		return 0;
	}

	SOCKET connectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (connectSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		LOG("Network::Connect: Error at socket(): %s(%u)", GetWSAErrorString(error).c_str(), error);
		freeaddrinfo(result);
		return 0;
	}

	// Connect to server.
	ret = connect(connectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (ret == SOCKET_ERROR)
	{
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
	}

	freeaddrinfo(result);

	if (connectSocket == INVALID_SOCKET)
	{
		LOG("Unable to connect to server!");
		return 0;
	}

	sockaddr_in peername;
	int peernamelen = sizeof(peername);
	getpeername(connectSocket, (sockaddr*)&peername, &peernamelen);

	Socket socket(connectSocket, address, port, transport, (transport == SocketOverTCP) ? cMaxTCPSendSize : cMaxUDPSendSize);
	socket.SetUDPPeername(peername);
	if (transport == SocketOverUDP)
	{
		SendUDPConnectDatagram(socket);
		LOG("Sent a UDP Connection Start datagram to to %s.", socket.ToString().c_str());
	}
	else
		LOG("Connected a TCP socket to %s.", socket.ToString().c_str());

	socket.SetBlocking(false);
	sockets.push_back(socket);

	Socket *sock = &sockets.back();

	MessageConnection *connection = PromoteToMessageConnection(sock, (transport == SocketOverTCP) ? ConnectionOK : ConnectionPending);
	assert(connection); ///\todo Convert assert() to error checking.

	connection->RegisterInboundMessageHandler(messageHandler);
	connection->StartWorkerThread();

	return connection;
}

Socket *Network::BindUDP(unsigned short port)
{
	SOCKET connectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (connectSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		LOG("Error at socket(): %s(%u)", GetWSAErrorString(error).c_str(), error);
		return 0;//Socket();
	}

	sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.S_un.S_addr = INADDR_ANY;

	int ret = bind(connectSocket, (sockaddr*)&local, sizeof(local));
	if (ret == SOCKET_ERROR)
	{
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
		return 0;
	}

	sockets.push_back(Socket(connectSocket, "", port, SocketOverUDP, cMaxUDPSendSize));
	Socket *socket = &sockets.back();
	socket->SetBlocking(false);

	LOG("Bound an UDP socket to %s.", socket->ToString().c_str());

	return socket;
}

Socket *Network::ConnectUDP(SOCKET connectSocket, const EndPoint &remoteEndPoint)
{/*
	SOCKET connectSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (connectSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		LOG("Error at socket(): %s(%u)", GetWSAErrorString(error).c_str(), error);
		return 0;//Socket();
	}
	*/
/*
	sockaddr_in localAddr = localEndPoint.ToSockAddrIn();
	int ret = bind(connectSocket, (sockaddr*)&localAddr, sizeof(localAddr));
	if (ret == SOCKET_ERROR)
	{
		LOG("Failed to bind!");
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
		return 0;
	}
*/
	
	sockaddr_in remoteAddr = remoteEndPoint.ToSockAddrIn();
/*
	int ret = connect(connectSocket, (sockaddr*)&remoteAddr, sizeof(remoteAddr));
	if (ret == SOCKET_ERROR)
	{
		LOG("Failed to connect!");
		closesocket(connectSocket);
		connectSocket = INVALID_SOCKET;
		return 0;
	}
*/
	char strIp[256];
	sprintf(strIp, "%d.%d.%d.%d", remoteEndPoint.ip[0], remoteEndPoint.ip[1], remoteEndPoint.ip[2], remoteEndPoint.ip[3]);

//	sockets.push_back(Socket(connectSocket, "", remoteEndPoint.port, SocketOverUDP, cMaxUDPSendSize));
	sockets.push_back(Socket(connectSocket, strIp, remoteEndPoint.port, SocketOverUDP, cMaxUDPSendSize));
	Socket *socket = &sockets.back();
	socket->SetBlocking(false);
	socket->SetUDPPeername(remoteEndPoint.ToSockAddrIn());

	LOG("Connected an UDP socket to %s.", socket->ToString().c_str());
	return socket;
}

Socket *Network::StoreSocket(const Socket &cp)
{
	sockets.push_back(cp);
	return &sockets.back();
}

void Network::SendUDPConnectDatagram(Socket &socket)
{
	///\todo Craft the connection attempt datagram.
	char data[256];
	memset(data, 0, sizeof(data));
	socket.Send(data, sizeof(data));
}

MessageConnection *Network::PromoteToMessageConnection(Socket *socket, ConnectionState startingState)
{
//	EnterCriticalSection(&connectionLock);
	MessageConnection *newConn = new MessageConnection(socket, startingState);
	messageConnections.push_back(newConn);
	MessageConnection *conn = messageConnections.back();
//	LeaveCriticalSection(&connectionLock);

	return conn;
}

void Network::CloseMessageConnection(MessageConnection *connection)
{
	assert(connection);

	for(std::list<MessageConnection *>::iterator iter = messageConnections.begin();
		iter != messageConnections.end(); ++iter)
		if (*iter == connection)
		{
			connection->Disconnect();
			connection->Close();
			messageConnections.erase(iter);
			break;
		}

	delete connection;
}
