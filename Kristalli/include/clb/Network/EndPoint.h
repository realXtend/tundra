/** @file
	@author Jukka Jylänki

	This work is copyrighted material and may NOT be used for any kind of commercial or 
	personal advantage and may NOT be copied or redistributed without prior consent
	of the author(s). 

	@brief
*/
#ifndef EndPoint_h
#define EndPoint_h

#include <string>

/// EndPoint represents a (ip, port) source or destination of a socket.
struct EndPoint
{
	/// The IPv4 address of the endpoint, stored in big-endian.
	unsigned char ip[4];

	unsigned short port;

	bool operator < (const EndPoint &rhs) const
	{
		if (ip[0] < rhs.ip[0]) return true;
		if (ip[0] > rhs.ip[0]) return false;
		if (ip[1] < rhs.ip[1]) return true;
		if (ip[1] > rhs.ip[1]) return false;
		if (ip[2] < rhs.ip[2]) return true;
		if (ip[2] > rhs.ip[2]) return false;
		if (ip[3] < rhs.ip[3]) return true;
		if (ip[3] > rhs.ip[3]) return false;
		if (port < rhs.port) return true;
		if (port > rhs.port) return false;

		return false;
	}

	static EndPoint FromSockAddrIn(const sockaddr_in &addr)
	{
		EndPoint endPoint;
		endPoint.ip[0] = addr.sin_addr.S_un.S_un_b.s_b1;
		endPoint.ip[1] = addr.sin_addr.S_un.S_un_b.s_b2;
		endPoint.ip[2] = addr.sin_addr.S_un.S_un_b.s_b3;
		endPoint.ip[3] = addr.sin_addr.S_un.S_un_b.s_b4;
		endPoint.port = ntohs(addr.sin_port);

		return endPoint;
	}

	sockaddr_in ToSockAddrIn() const
	{
		sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(port);
		address.sin_addr.S_un.S_un_b.s_b1 = ip[0];
		address.sin_addr.S_un.S_un_b.s_b2 = ip[1];
		address.sin_addr.S_un.S_un_b.s_b3 = ip[2];
		address.sin_addr.S_un.S_un_b.s_b4 = ip[3];

		return address;
	}

	std::string ToString() const
	{
		char str[256];
		sprintf(str, "%d.%d.%d.%d:%d", (unsigned int)ip[0], (unsigned int)ip[1], (unsigned int)ip[2], (unsigned int)ip[3], (unsigned int)port);
		return std::string(str);
	}
};

#endif
