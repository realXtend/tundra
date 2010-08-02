/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef NetworkServer_h
#define NetworkServer_h

#include <list>

#include "clb/Core/Ptr.h"

#include "Socket.h"
#include "MessageConnection.h"
#include "Datagram.h"
#include "INetworkServerListener.h"

class Network;

/// Manages all low-level networking required in maintaining a network server and keeps
/// track of all currently established connections.
class NetworkServer : public clb::RefCountable
{
	NetworkServer(Network *owner, Socket *listenSocket, SocketTransportLayer transport);

	Socket *listenSocket;

	typedef std::map<EndPoint, MessageConnection*> ConnectionMap;

	/// The list of active client connections.
	ConnectionMap clients;

	Network *owner;

	/// Specifies which transport layer we are using.
	SocketTransportLayer transport;

	/// If true, new connection attempts are processed. Otherwise, just discard all connection packets.
	bool acceptNewConnections;

	/// If the server is running in UDP mode, the listenSocket is the socket that receives all application data.
	/// This function pulls all new data from the socket and sends it to MessageConnection instances for deserialization and processing.
	void ReadUDPSocketData();

	INetworkServerListener *networkServerListener;

	void RegisterServerListener(INetworkServerListener *listener);

	/// Stops listening for new connections, but all already established connections are maintained.
	void CloseListenSocket();

	Socket *AcceptConnections();

	bool ProcessNewConnectionAttempt(const EndPoint &endPoint, Datagram &datagram);

	friend class Network;

public:

	/// Enters a stand-alone processing loop that manages incoming connections until server is shut down.
	void RunModalServer();

	/// Enables or disables whether new connection attempts are allowed.
	void SetAcceptNewConnections(bool acceptNewConnections);

	/// Enables or disables whether rejected connection attempts are messages back to the client (UDP only).
	/// i.e. whether to message "Connection rejected" back to the peer.
	void SetStealthMode(bool stealthModeEnabled);

	/// Handles all new connection attempts and pulls in new messages from all existing connections.
	void ProcessMessages();

	/// Starts the disconnection procedure for the given connection.
	void Disconnect(MessageConnection *connection);

	/// Forcibly erases the given connection from the active connection list. The client will be left to time out.
	void Close(MessageConnection *connection);

	/// Returns all the currently tracked connections.
	ConnectionMap &GetConnections() { return clients; }
};

#endif
