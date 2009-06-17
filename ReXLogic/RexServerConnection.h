// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexEntity_h
#define incl_RexEntity_h

#include "NetworkEvents.h"
#include "RexTypes.h"

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
}

namespace RexLogic
{
    class MODULE_API RexServerConnection
    {
        friend class NetworkEventHandler;
    public:

		enum ConnectionType { DirectConnection = 0, AuthenticationConnection };

        RexServerConnection(Foundation::Framework *framework);
        virtual ~RexServerConnection();
        
        /// Connects to a reX server.
        /** @param username Username (Firstname Lastname).
          * @param password Password.
          * @param serveraddress Simulator server address and port.
          * @param auth_server_address Authentication server address and port.
          * @param auth_login Authentication server username.
          * @return True if the connection was succesfull. False otherwise.
          */
        bool ConnectToServer(const std::string& username,
            const std::string& password,
            const std::string& serveraddress,
            const std::string& auth_server_address = "",
            const std::string& auth_login = "");
        
        /// Creates the UDP connection after a succesfull XML-RPC login.
        /// @return True, if success.
        bool CreateUDPConnection();
        
        /// Request logout from the server.
        void RequestLogout();
        
        /// Disconnects from the server. Doesn't send the LogOutRequest packet.
        void ForceServerDisconnect();
        
        /// Set the connection type.
        /// @param ConnectionType enum.
		void SetConnectionType(ConnectionType type) { connection_type_ = type; }
        
        /// Get the connection type.
        /// @return ConnectionType enum.
		ConnectionType GetConnectionType() const { return connection_type_; }

        /// Send the UDP chat packet.
        void SendChatFromViewerPacket(std::string text);
        
        /// Sends the first UDP packet to open up the circuit with the server.             
        void SendUseCircuitCodePacket();
        
        /// Signals that agent is coming into the region. The region should be expecting the agent.
        /// Server starts to send object updates etc after it has received this packet.        
        void SendCompleteAgentMovementPacket();
        
        /// Tells client bandwidth to the server
        /// \todo make configurable or measure, now a fixed value
        void SendAgentThrottlePacket();
        
        /// Sends a message requesting logout from the server. The server is then going to flood us with some
        /// inventory UUIDs after that, but we'll be ignoring those.        
        void SendLogoutRequestPacket();
        
        // Sends the basic movement message
	    void SendAgentUpdatePacket(Core::Quaternion bodyrot, Core::Quaternion headrot, uint8_t state, 
	        RexTypes::Vector3 camcenter, RexTypes::Vector3 camataxis, RexTypes::Vector3 camleftaxis, RexTypes::Vector3 camupaxis,
	        float far, uint32_t controlflags, uint8_t flags);        
        
        /// Sends a packet which indicates selection of a group of prims.
        /// @param Local ID of the object which is selected.
        void SendObjectSelectPacket(Core::entity_id_t object_id);
        
        /// Sends a packet which indicates selection of a prim.
        /// @param List of local ID's of objects which are selected.        
        void SendObjectSelectPacket(std::vector<Core::entity_id_t> object_id_list);

        /// Sends a packet which indicates deselection of prim(s).
        /// @param Local ID of the object which is deselected.
        void SendObjectDeselectPacket(Core::entity_id_t object_id);        

        /// Sends a packet which indicates deselection of a group of prims.
        /// @param List of local ID's of objects which are deselected.
        void SendObjectDeselectPacket(std::vector<Core::entity_id_t> object_id_list);        
        
        /// Sends a packet indicating change in Object's position, rotation and scale.
        /// @param List of updated entity pointers.
        void SendMultipleObjectUpdatePacket(std::vector<Scene::EntityPtr> entity_ptr_list);

        /// Sends a packet indicating change in Object's name.
        /// @param List of updated entity pointers.
        void SendObjectNamePacket(std::vector<Scene::EntityPtr> entity_ptr_list);
        
        /// Sends a packet which indicates object has been touched.
        /// @param Local ID of the object which has been touched.
        void SendObjectGrabPacket(Core::entity_id_t object_id);
        
        /// Sends a packet indicating change in Object's description
        /// @param List of updated entity pointers.
        void SendObjectDescriptionPacket(std::vector<Scene::EntityPtr> entity_ptr_list);

        /// Sends handshake reply packet
        void SendRegionHandshakeReplyPacket(RexTypes::RexUUID agent_id, RexTypes::RexUUID session_id, uint32_t flags);

        /// Sends hardcoded agentappearance packet
        void SendAgentSetAppearancePacket();
                        
        /// @return Name of the sim we're connected to.
        std::string GetSimName() { return simName_; }
        
        /// @return A structure of connection spesific information, e.g. AgentID and SessiondID.
        OpenSimProtocol::ClientParameters GetInfo() const { return myInfo_; }
        
        /// @return True if the client connected to a server.
        bool IsConnected() { return connected_; }
        
        /// @return The state of the connection.
        volatile OpenSimProtocol::Connection::State GetConnectionState();
        
    private:
        /// Convenience function to get the weak pointer when building messages.
        NetOutMessage *StartMessageBuilding(NetMsgID message_id);
        
        /// Convenience function to get the weak pointer when sending messages.
        void FinishMessageBuilding(NetOutMessage *msg);
        
        /// The framework we belong to.        
        Foundation::Framework *framework_;
    
        /// Pointer to the network interface.
		boost::weak_ptr<OpenSimProtocol::OpenSimProtocolModule> netInterface_;
        
        /// Server-spesific info for this client.
        OpenSimProtocol::ClientParameters myInfo_;
		
		/// Address of the sim we're connected.
		std::string serverAddress_;	
		
		/// Port of the sim we're connected.
		int serverPort_;		
		
        /// Name of the sim we're connected.
        std::string simName_;		
        
        /// Is client connected to a server.
        bool connected_;
        /// Type of the connection.
        ConnectionType connection_type_;
        
        /// State of the connection procedure.
        OpenSimProtocol::Connection::State state_;
        
        /// State of the connection procedure thread.
        OpenSimProtocol::ConnectionThreadState threadState_;
    };
}

#endif
