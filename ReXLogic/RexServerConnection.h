// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexEntity_h
#define incl_RexEntity_h

#include "OpenSimProtocolModule.h"
#include "NetInMessage.h"
#include "NetMessage.h"


namespace RexLogic
{
    class MODULE_API RexServerConnection
    {
        friend class NetworkEventHandler;
    public:
        RexServerConnection(Foundation::Framework *framework);
        virtual ~RexServerConnection();
        
        bool ConnectToServer(std::string username, std::string password, std::string serveraddress);
        void RequestLogout();
        void CloseServerConnection();

        // Send the UDP chat packet.
        void SendChatFromViewerPacket(std::string text);
        /// Sends the first UDP packet to open up the circuit with the server.             
        void SendUseCircuitCodePacket();
        /// Signals that agent is coming into the region. The region should be expecting the agent.
        /// Server starts to send object updates etc after it has received this packet.        
        void SendCompleteAgentMovementPacket();
        /// Sends a message requesting logout from the server. The server is then going to flood us with some
        /// inventory UUIDs after that, but we'll be ignoring those.        
        void SendLogoutRequestPacket();
        
        // Sim name
        std::string GetSimName() { return simname_; }
        // Connection info        
        ClientParameters GetInfo() const { return myInfo_; }
        // Connected to server
        bool IsConnected() { return connected_; }

    private:
        Foundation::Framework *framework_;    
    
        /// Pointer to the network interface.
        OpenSimProtocol::OpenSimProtocolModule *netInterface_;
		
        /// Server-spesific info for this client.
        ClientParameters myInfo_;
		
        /// Name of the sim we're connected.
        std::string simname_;		
        
        // Connected to server
        bool connected_;
    };
}

#endif
