// For conditions of distribution and use, see copyright notice in license.txt
#include <utility>

#include "NetworkConnection.h"

using namespace std;

NetworkConnection::NetworkConnection(const char *address, int port)
{
	socket.connect(Poco::Net::SocketAddress(address, port));
}

NetworkConnection::~NetworkConnection()
{
}

int NetworkConnection::ReceiveBytes(uint8_t *bytes, size_t maxCount)
{
	int numBytes = min((int)maxCount, socket.available());
	if (numBytes == 0)
		return numBytes;

	return socket.receiveBytes(bytes, numBytes);
}

void NetworkConnection::SendBytes(const uint8_t *bytes, size_t count)
{
	socket.sendBytes(bytes, (int)count);
}
