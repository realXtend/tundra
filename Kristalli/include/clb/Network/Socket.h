/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef Socket_h
#define Socket_h

#include "WS2Include.h"

#include <vector>
#include <list>

#include "clb/Core/Ptr.h"
#include "clb/Network/EndPoint.h"

/// Identifiers for the possible bottom-level tranport layers.
enum SocketTransportLayer
{
	SocketOverUDP,
	SocketOverTCP
};

/// Represents a low-level network socket.
class Socket : public clb::RefCountable
{
	SOCKET connectSocket;
	sockaddr_in udpPeerName;
	std::string destinationAddress;
	unsigned short destinationPort;
	SocketTransportLayer transport;
	size_t maxSendSize;

public:
	Socket();
	Socket(SOCKET connection, const char *address, unsigned short port, SocketTransportLayer transport, size_t maxSendSize);
	~Socket();
//	explicit Socket(SOCKET connection); better

	bool Connected() const;

	void Close();
	void Disconnect();
	bool Send(const char *data, size_t numBytes);

	void SetUDPPeername(const sockaddr_in &peer) { udpPeerName = peer; }
	/// Reads in data from the socket. 
	/// @param endPoint [out] If the socket is an UDP socket that is not bound to an address, this will contain the source address.
	size_t Receive(char *dst, size_t maxBytes, EndPoint *endPoint = 0);

	/// Returns which transport layer the connection is using.
	SocketTransportLayer TransportLayer() const { return transport; }
	size_t MaxSendSize() const { return maxSendSize; }

	const char *DestinationAddress() const { return destinationAddress.c_str(); }
	unsigned short DestinationPort() const { return destinationPort; }

	std::string ToString() const;

	/// Returns the EndPoint this socket is connected to.
	EndPoint GetEndPoint() const;

	/// Sets the socket to blocking or nonblocking state.
	void SetBlocking(bool isBlocking);

	SOCKET &WinSocket() { return connectSocket; }
};

#endif
