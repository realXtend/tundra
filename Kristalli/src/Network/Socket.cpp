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
#include "clb/Network/Socket.h"
#include "clb/Network/Network.h"

#include "clb/Memory/DebugMemoryLeakCheck.h"

Socket::Socket()
:connectSocket(INVALID_SOCKET)
{
}

/*
Socket::Socket(SOCKET connection)
:connectSocket(connection)
{
}
*/

Socket::~Socket()
{
// Should write an AutoPtr-style copy ctor or indirect to be able to do this:
//	Disconnect();
//	Close();
}

Socket::Socket(SOCKET connection, const char *address, unsigned short port, SocketTransportLayer transport_, size_t maxSendSize_)
:connectSocket(connection), destinationAddress(address), destinationPort(port), 
transport(transport_), maxSendSize(maxSendSize_)
{
	u32 val = 0;
	u32 val2 = 0;
	int valLen = sizeof(val);

	val = 32768;
	val2 = 32768;
	setsockopt(connection, SOL_SOCKET, SO_RCVBUF, (char*)&val, valLen);
	setsockopt(connection, SOL_SOCKET, SO_SNDBUF, (char*)&val2, valLen);

	getsockopt(connection, SOL_SOCKET, SO_RCVBUF, (char*)&val, &valLen);
	getsockopt(connection, SOL_SOCKET, SO_SNDBUF, (char*)&val2, &valLen);

	LOG("Socket created. RCVBUF: %d, SNDBUF: %d", val, val2);
}

bool Socket::Connected() const
{
	return connectSocket != INVALID_SOCKET;
}

size_t Socket::Receive(char *dst, size_t maxBytes, EndPoint *endPoint)
{
	if (connectSocket == INVALID_SOCKET)
		return 0;

	if (transport == SocketOverUDP && destinationAddress.length() == 0)
	{
		sockaddr_in from;
		int fromLen = sizeof(from);
		int numBytesRead = recvfrom(connectSocket, dst, maxBytes, 0, (sockaddr*)&from, &fromLen);
		if (numBytesRead == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK && error != 0)
			{
				///\todo Mark UDP server and client sockets separately. For a server socket, we cannot Close() here,
				/// but for client sockets it is safe.
//				LOG("recvfrom failed: %s(%u) in socket %s", GetWSAErrorString(error).c_str(), error, ToString().c_str());
//				Close();
			}

			return 0;
		}
//		if (numBytesRead > 0)
//			LOG("recvfrom (%d) in socket %s", numBytesRead, ToString().c_str());

/*
		char address[256];
		sprintf(address, "%d.%d.%d.%d",
			(int)from.sin_addr.S_un.S_un_b.s_b1, (int)from.sin_addr.S_un.S_un_b.s_b2, 
			(int)from.sin_addr.S_un.S_un_b.s_b3, (int)from.sin_addr.S_un.S_un_b.s_b4);

		char portStr[256];
		sprintf(portStr, "%d", (unsigned long)from.sin_port);
		LOG("Got a UDP reply from %s:%s. Binding UDP listen socket to that address.",
			address, portStr);
		int ret = connect(connectSocket, (sockaddr*)&from, fromLen);
		if (ret == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			LOG("connect failed: %s(%u)", GetWSAErrorString(error).c_str(), error);
			return 0;
		}

		destinationAddress = address;
*/
		if (endPoint)
			*endPoint = EndPoint::FromSockAddrIn(from);

//		if (numBytesRead > 0)
//			std::cout << "x. Socket 0x" << this << " read " << numBytesRead << " bytes from " << (endPoint ? endPoint->ToString() : destinationAddress) << std::endl;

		return numBytesRead;
	}

	int ret;
	if (transport == SocketOverTCP)
		ret = recv(connectSocket, dst, maxBytes, 0);
	else
	{
		int fromlen = sizeof(udpPeerName);
		ret = recvfrom(connectSocket, dst, maxBytes, 0, (sockaddr*)&udpPeerName, &fromlen);
		if (endPoint)
			*endPoint = EndPoint::FromSockAddrIn(udpPeerName);

//		if (ret > 0)
//			std::cout << "y. Socket 0x" << this << " read " << ret << "? bytes from " << EndPoint::FromSockAddrIn(udpPeerName).ToString() << std::endl;

	}

	if (ret > 0)
	{
//		LOG("recv (%d) in socket %s", ret, ToString().c_str());
		return (size_t)ret;
	}
	else if (ret == 0)
	{
		LOG("Received 0 bytes from network. Connection closed in socket %s.", ToString().c_str());
		Disconnect();
		return 0;
	}
	else
	{
		int error = WSAGetLastError();
		if (error != WSAEWOULDBLOCK && error != 0)
		{
			LOG("recv/recvfrom failed in socket %s. Error %s(%d)", ToString().c_str(), GetWSAErrorString(error).c_str(), error);
			Close();
		}
		return 0;
	}
}

void Socket::Disconnect()
{
	if (connectSocket == INVALID_SOCKET)
		return;

	if (transport == SocketOverTCP)
	{
		int result = shutdown(connectSocket, SD_BOTH);
		if (result == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			LOG("Socket shutdown failed: %s(%u) in socket %s.", GetWSAErrorString(error).c_str(), error, ToString().c_str());
		}
		else
			LOG("Shut down socket %s.", ToString().c_str());

	}
	if (transport == SocketOverTCP)
		closesocket(connectSocket);
	connectSocket = INVALID_SOCKET;
	destinationAddress = "";
	destinationPort = 0;
}

void Socket::Close()
{
	if (connectSocket == INVALID_SOCKET)
		return;

	LOG("Closed socket %s.", ToString().c_str());

	// Each TCP Socket owns the SOCKET they contain. For UDP, the same SOCKET is shared by several Sockets, so closing
	// one would close them all. \todo The server instance should "own" the UDP SOCKET and close it down.
	if (transport == SocketOverTCP)
		closesocket(connectSocket);

	connectSocket = INVALID_SOCKET;
	destinationAddress = "";
	destinationPort = 0;
}

void Socket::SetBlocking(bool isBlocking)
{
	if (connectSocket == INVALID_SOCKET)
		return;

	u_long nonBlocking = (isBlocking == false) ? 1 : 0;
	ioctlsocket(connectSocket, FIONBIO, &nonBlocking);	
}

/// @return True on success, false otherwise.
bool Socket::Send(const char *data, size_t numBytes)
{
	if (connectSocket == INVALID_SOCKET)
	{
		LOG("Trying to send a datagram to INVALID_SOCKET!");
		return false;
	}

	int sendTriesLeft = 100;
	size_t numBytesSent = 0;
	while(numBytesSent < numBytes && sendTriesLeft-- > 0)
	{
		size_t numBytesLeftToSend = (size_t)(numBytes - numBytesSent);
		SetBlocking(true);
		int ret;
		if (transport == SocketOverUDP)
			ret = sendto(connectSocket, data + numBytesSent, (int)numBytesLeftToSend, 0, (sockaddr*)&udpPeerName, sizeof(udpPeerName));
		else
			ret = send(connectSocket, data + numBytesSent, (int)numBytesLeftToSend, 0);
		if (ret == SOCKET_ERROR)
		{
			int error = WSAGetLastError();
			if (error != WSAEWOULDBLOCK)
			{
				if (error != 0)
					Close();
				LOG("Failed to send %d bytes over socket %s. Error %s(%u)", numBytes, ToString().c_str(), GetWSAErrorString(error).c_str(), error);
			}
			SetBlocking(false);
			return false;
		}
		SetBlocking(false);
//		LOG("Sent %d bytes over socket %s.", ret, ToString().c_str());
		numBytesSent += ret;

		if (numBytesSent < numBytes)
		{
			FD_SET writeSocketSet;
			FD_SET errorSocketSet;
			FD_ZERO(&writeSocketSet);
			FD_ZERO(&errorSocketSet);
			FD_SET(connectSocket, &writeSocketSet);
			FD_SET(connectSocket, &errorSocketSet);
			timeval timeout = { 5, 0 }; 
			int ret = select(0, 0, &writeSocketSet, &errorSocketSet, &timeout);
			if (ret == 0 || ret == SOCKET_ERROR)
			{
				LOG("Waiting for socket %s to become write-ready failed!", ToString().c_str());
				// If we did manage to send any bytes through, the stream is now out of sync,
				// disconnect.
				if (numBytesSent > 0)
					Disconnect();
				return false;
			}
		}
	}

	if (sendTriesLeft <= 0)
	{
		if (numBytesSent > 0)
		{
			LOG("Could not send %d bytes to socket %s. Achieved %d bytes, stream has now lost bytes, closing connection!", numBytes, 
				ToString().c_str(), numBytesSent);
			Close();
		}
		else
			LOG("Could not send %d bytes to socket %s.", numBytes, ToString().c_str());
		return false;
	}

	return true;
}
/*
namespace
{
	std::string DumpSocketOptions(SOCKET socket)
	{
		BOOL val;
		getsockopt(socket, SOL_SOCKET, SO_ACCEPTCONN, &val, sizeof(val));

	}
}
*/

EndPoint Socket::GetEndPoint() const
{
	return EndPoint::FromSockAddrIn(udpPeerName);
}

std::string Socket::ToString() const
{
	sockaddr_in addr;
	int namelen = sizeof(addr);
	int peerRet = getpeername(connectSocket, (sockaddr*)&addr, &namelen); // Note: This works only if family==INETv4
	EndPoint peerName = EndPoint::FromSockAddrIn(addr);

	int sockRet = getsockname(connectSocket, (sockaddr*)&addr, &namelen); // Note: This works only if family==INETv4
	EndPoint sockName = EndPoint::FromSockAddrIn(addr);

	char str[256];
	sprintf(str, "%s:%d (%s, connected=%s, maxSendSize=%d, sock: %s, peer: %s)", DestinationAddress(), (unsigned int)DestinationPort(), 
		(transport == SocketOverTCP) ? "TCP" : "UDP", Connected() ? "true" : "false", maxSendSize,
		sockRet == 0 ? sockName.ToString().c_str() : "(-)", peerRet == 0 ? peerName.ToString().c_str() : "(-)");

	return std::string(str);
}

