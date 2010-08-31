/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
//#include "clb/Core/Logging.h"
#include "clb/Network/NetworkServer.h"
#include "clb/Network/Network.h"
#include "clb/Network/DataGram.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

NetworkServer::NetworkServer(Network *owner_, Socket *listenSocket_, SocketTransportLayer transport_)
:owner(owner_), listenSocket(listenSocket_), transport(transport_), acceptNewConnections(true), networkServerListener(0)
{
	assert(owner);
	assert(listenSocket);
}

void NetworkServer::RegisterServerListener(INetworkServerListener *listener)
{
	networkServerListener = listener;
}

void NetworkServer::SetAcceptNewConnections(bool acceptNewConnections_)
{
	acceptNewConnections = acceptNewConnections_;
}

void NetworkServer::CloseListenSocket()
{
	if (transport == SocketOverUDP)
		acceptNewConnections = false;
	else
	{
		if (listenSocket)
			listenSocket->Close();
	}
}

Socket *NetworkServer::AcceptConnections()
{
	if (!listenSocket || !listenSocket->Connected())
		return 0;

	sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));
	int addrLen = sizeof(clientAddr);
	SOCKET &listenSock = listenSocket->WinSocket();
	SOCKET acceptSocket = accept(listenSock, (sockaddr*)&clientAddr, &addrLen);
	if (acceptSocket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK)
		{
			int error = WSAGetLastError();
			LOG("accept failed: %s(%u)", GetWSAErrorString(error).c_str(), error);
			closesocket(listenSock);
			listenSock = INVALID_SOCKET;
		}
		return 0;//Socket();
	}
   
	char address[256];
	sprintf(address, "%d.%d.%d.%d",
		(int)clientAddr.sin_addr.S_un.S_un_b.s_b1, (int)clientAddr.sin_addr.S_un.S_un_b.s_b2, 
		(int)clientAddr.sin_addr.S_un.S_un_b.s_b3, (int)clientAddr.sin_addr.S_un.S_un_b.s_b4);

	unsigned short port = ntohs(clientAddr.sin_port);
	LOG("Client connected from %s:%d.\n", address, (unsigned int)port);

	const size_t maxTcpSendSize = 65536;
	Socket *socket = owner->StoreSocket(Socket(acceptSocket, address, port, SocketOverTCP, maxTcpSendSize));
	socket->SetBlocking(false);
//	clients.push_back(socket);
	return socket;
}

void NetworkServer::ProcessMessages()
{
	// Clean up all disconnected sockets.
	ConnectionMap::iterator iter = clients.begin();
	while(iter != clients.end())
	{
		ConnectionMap::iterator next = iter;
		++next;
		if (!iter->second->Connected())
		{
			LOG("Client %s disconnected!", iter->second->ToString().c_str());
			if (networkServerListener)
				networkServerListener->ClientDisconnected(iter->second);
			owner->CloseMessageConnection(iter->second);
			clients.erase(iter->first);
		}
		iter = next;
	}

	// If the listen socket is in UDP mode, the same socket is used to both
	// receive new connection attempts and to receive all communication data for all clients.
	// If in UDP mode, pull in all new data and pass those to the MessageConnections for deserialization and processing.
	if (transport == SocketOverUDP)
		ReadUDPSocketData();
	else
	{
		// Accept the first inbound connection.
		Socket *client = AcceptConnections();
		if (client)
		{
			LOG("Client connected from %s.", client->ToString().c_str());

			// Build a message connection on top of the raw socket.
			MessageConnection *clientConnection = owner->PromoteToMessageConnection(client, ConnectionOK);
			if (!clientConnection || !clientConnection->Connected())
			{
				LOG("Unable to start connection!");
//				return;
			}
			else
			{
				clientConnection->StartWorkerThread();
				if (networkServerListener)
					networkServerListener->NewConnectionEstablished(clientConnection);

				sockaddr_in sockname;
				int socknamelen = sizeof(sockname);
				int ret = getpeername(client->WinSocket(), (sockaddr*)&sockname, &socknamelen);
				if (ret == 0)
				{
					EndPoint endPoint = EndPoint::FromSockAddrIn(sockname);
					std::cout << "Client connected from " << endPoint.ToString() << std::endl;
					clients[endPoint] = clientConnection;
				}
				else
					assert(false);
			}
		}
	}

	// Process all new inbound data for each connection handled by this server.
	for(ConnectionMap::iterator iter = clients.begin(); iter != clients.end(); ++iter)
		iter->second->ProcessMessages();
}

void NetworkServer::ReadUDPSocketData()
{
	assert(listenSocket);

	EndPoint endPoint;
	Datagram data;
	size_t bytesReceived = listenSocket->Receive((char*)&data.data[0], cDatagramBufferSize, &endPoint);
	if (bytesReceived == 0)
		return; // No datagram available, return.
	data.size = bytesReceived;

//	LOG("Received a datagram of size %d to socket %s from endPoint %s.", bytesReceived, listenSocket->ToString().c_str(),
//		endPoint.ToString().c_str());
	ConnectionMap::iterator iter = clients.find(endPoint);
	if (iter != clients.end())
	{
		// If the datagram came from a known endpoint, pass it to the connection object that handles that endpoint.
		iter->second->InjectDatagram(data);
	}
	else
	{
		// The endpoint for this datagram is not known, deserialize it as a new connection attempt packet.
		ProcessNewConnectionAttempt(endPoint, data);
	}
}

bool NetworkServer::ProcessNewConnectionAttempt(const EndPoint &endPoint, Datagram &datagram)
{
	LOG("New inbound connection attempt from %s with datagram of size %d.", endPoint.ToString().c_str(), datagram.size);
	if (!acceptNewConnections)
	{
		LOG("Ignored connection attempt since server is set not to accept new connections.");
		return false;
	}

	// Pass the datagram contents to a callback that decides whether this connection is allowed.
	if (networkServerListener)
	{
		bool connectionAccepted = networkServerListener->NewConnectionAttempt(endPoint, datagram);
		if (!connectionAccepted)
		{
			LOG("Server listener did not accept the new connection.");
			return false;
		}
	}

	///\todo Check IP banlist.
	///\todo Check that the maximum number of active concurrent connections is not exceeded.

	SOCKET sock = listenSocket->WinSocket();
//	sockaddr_in sockname;
//	int socknamelen = sizeof(sockname);
//	int ret = getsockname(sock, (sockaddr*)&sockname, &socknamelen);
	// Accept the connection and create a new UDP socket that communicates to that endpoint.
	Socket *socket = owner->ConnectUDP(sock, endPoint);
	if (!socket)
		return false;

	MessageConnection *connection = owner->PromoteToMessageConnection(socket, ConnectionOK);
	if (!connection)
	{
		socket->Disconnect();
		return false;
	}
	connection->SetInjectionMode(true);
	clients[endPoint] = connection;

	// Pass the MessageConnection to the main application so it can hook the inbound packet stream.
	if (networkServerListener)
		networkServerListener->NewConnectionEstablished(connection);

	connection->StartWorkerThread();

	return true;
}

void NetworkServer::RunModalServer()
{
	///\todo Loop until StopModalServer() is called.
	for(;;)
	{
		ProcessMessages();

		///\todo WSACreateEvent/WSAWaitForMultipleEvents for improved responsiveness and performance.
		Sleep(1);
	}
}

void NetworkServer::Disconnect(MessageConnection *connection)
{
	for(ConnectionMap::iterator iter = clients.begin();
		iter != clients.end(); ++iter)
		if (iter->second == connection)
		{
			if (networkServerListener)
				networkServerListener->ClientDisconnected(connection);

			clients.erase(iter);
			owner->CloseMessageConnection(connection);

			return;
		}

		LOG("Unknown MessageConnection passed to NetworkServer::Disconnect!");
}
