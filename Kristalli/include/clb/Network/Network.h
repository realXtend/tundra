/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef Network_h
#define Network_h

#include "Socket.h"
#include "NetworkServer.h"
#include "MessageConnection.h"

/// Provides the application an interface for both client and server networking.
class Network
{
	WSADATA wsaData;

	std::string machineIP;

	/// Maintains the server-related data structures if this computer
	/// is acting as a server. Otherwise this data is not used.
	Ptr(NetworkServer) server;
	/// Contains all active sockets in the system.
	std::list<Socket> sockets;

	std::list<MessageConnection *> messageConnections;

	/// Takes the ownership of the given socket, and returns a pointer to the owned one.
	Socket *StoreSocket(const Socket &cp);

	friend class NetworkServer;

	void SendUDPConnectDatagram(Socket &socket);

	/// Returns a new socket that is bound to listen to the given local port.
	/// The returned pointer is owned by this class.
	Socket *BindUDP(unsigned short port);

	/// Returns a new UDP socket that is bound to communicating with the given endpoint.
	/// Does NOT send the connection packet.
	/// The returned pointer is owned by this class.
	Socket *ConnectUDP(SOCKET connectSocket, const EndPoint &remoteEndPoint);

	/// Promotes the given (raw) socket to a MessageConnection.
	MessageConnection *PromoteToMessageConnection(Socket *socket, ConnectionState startingState);

	void Init();
	void DeInit();

public:
	Network();
	~Network();

	static void PrintAddrInfo(const addrinfo *address);

	void PrintHostNameInfo(const char *hostname, const char *port);

	/// Starts a network server that listens to the given local port.
	/// @param serverListener [in] A pointer to the listener object that will be registered to receive notifications
	///	about incoming connections.
	NetworkServer *StartServer(unsigned short port, SocketTransportLayer transport, INetworkServerListener *serverListener);

	void StopServer();

	/** Connects to the given address:port using KristalliProtocol over UDP or TCP.
		The returned pointer is owned by this class. Call Network::Disconnect to close the connection. */
	MessageConnection *Connect(const char *address, unsigned short port, SocketTransportLayer transport, IMessageHandler *messageHandler);

	/// @return Local machine IP.
	const char *MachineIP() const { return machineIP.c_str(); }

	/// Closes a previously established MessageConnection. Call this for each MessageConnection
	/// you created from a socket.
	void CloseMessageConnection(MessageConnection *connection);
};

/// Returns the WinSock error string associated with the given error id.
std::string GetWSAErrorString(int error);

/// Returns the WinSock error string corresponding to the last error that occurred in the WinSock2 library.
std::string GetLastWSAErrorString();

/// Outputs the given number of bytes formatted to KB or MB suffix for readability.
std::string FormatBytes(size_t numBytes);

#endif
