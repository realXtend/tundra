// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexEntity_h
#define incl_RexEntity_h

#include "OpenSimProtocolModule.h"
#include "NetInMessage.h"
#include "NetMessage.h"

namespace OgreRenderer
{
    class EC_OgrePlaceable;
}

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
        
        // Sends the basic movement message
	    void SendAgentUpdatePacket(Core::Quaternion bodyrot, Core::Quaternion headrot, uint8_t state, 
	        RexTypes::Vector3 camcenter, RexTypes::Vector3 camataxis, RexTypes::Vector3 camleftaxis, RexTypes::Vector3 camupaxis,
	        float far, uint32_t controlflags, uint8_t flags);        
        
        /// Sends a packet which indicates selection of a group of prims.
        ///@param Local ID of the object which is selected.
        void SendObjectSelectPacket(Core::entity_id_t object_id);
        
        /// Sends a packet which indicates selection of a prim.
        ///@param List of local ID's of objects which are selected.        
        void SendObjectSelectPacket(std::vector<Core::entity_id_t> object_id_list);

        /// Sends a packet which indicates deselection of prim(s).
        ///@param Local ID of the object which is deselected.
        void SendObjectDeselectPacket(Core::entity_id_t object_id);        

        /// Sends a packet which indicates deselection of a group of prims.
        ///@param List of local ID's of objects which are deselected.
        void SendObjectDeselectPacket(std::vector<Core::entity_id_t> object_id_list);        
        
        /// Sends a packet indicating change in Object's position, rotation and scale.
        ///@param List of updated entity pointers.
        void SendMultipleObjectUpdatePacket(std::vector<Foundation::EntityPtr> entity_ptr_list);
        
        ///@return Name of the sim we're connected to.
        std::string GetSimName() { return simname_; }
        
        ///@return A structure of connection spesific information, e.g. AgentID and SessiondID.
        ClientParameters GetInfo() const { return myInfo_; }
        
        ///@return True if client connected to a server.
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
