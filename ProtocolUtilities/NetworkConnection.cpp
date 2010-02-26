// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include <utility>

#include "NetworkConnection.h"

using namespace std;

namespace ProtocolUtilities
{

NetworkConnection::NetworkConnection(const char *address, int port): bOpen(true)
{
    socket.connect(Poco::Net::SocketAddress(address, port));
    
    const size_t cBufferSize = 100000;
    socket.setReceiveBufferSize(cBufferSize);
    socket.setSendBufferSize(cBufferSize);
}

NetworkConnection::~NetworkConnection()
{
}

bool NetworkConnection::PacketsAvailable() const
{
    if (!bOpen)
        return false;
    
    return socket.available() != 0;
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

void NetworkConnection::Close()
{
    socket.close();
    bOpen = false;
}

}