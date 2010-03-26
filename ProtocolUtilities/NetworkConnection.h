// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Rex_NetworkConnection_h
#define incl_Rex_NetworkConnection_h

#include "Poco/Net/DatagramSocket.h"
#include "RexTypes.h"

namespace ProtocolUtilities
{
    /// NetworkConnection represents the socket of a bidirectional UDP connection.
    class NetworkConnection
    {
    public:
        /// Connects to the given address.
        NetworkConnection(const char *address, int port);
        ~NetworkConnection();

        /// @return True if there are available UDP packets in the stream and the socket is open. 
        bool PacketsAvailable() const;

        /// Reads bytes from the socket. Doesn't block, but returns 0 if no bytes available.
        /// @param maxCount The maximum number of bytes to fill into the buffer.
        /// @return The number of bytes that was actually filled into the buffer.
        int ReceiveBytes(uint8_t *bytes, size_t maxCount);

        /// Pushes out a packet with the given contents.
        void SendBytes(const uint8_t *bytes, size_t count);

        /// Closes the socket.
        void Close();

        /// @return True if the socket is open.
        bool Open() const { return bOpen; }

    private:
        /// PoCo UDP socket.
        Poco::Net::DatagramSocket socket;

        /// Signals that socket is open for use. ///\todo Remove this boolean altogether. -jj.
        bool bOpen;
    };
}

#endif
