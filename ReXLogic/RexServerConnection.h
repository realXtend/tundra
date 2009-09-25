// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexEntity_h
#define incl_RexEntity_h

#include "NetworkEvents.h"
#include "RexTypes.h"

namespace OpenSimProtocol
{
    class OpenSimProtocolModule;
    class InventoryFolder;
}

namespace RexLogic
{
    class MODULE_API RexServerConnection
    {
        friend class NetworkEventHandler;
    public:

        /// Connection type enumeration.
        enum ConnectionType
        {
            DirectConnection = 0,
            AuthenticationConnection
        };

        /// Constructor.
        /// @param framework Pointer to framework.
        RexServerConnection(Foundation::Framework *framework);

        /// Destructor.
        virtual ~RexServerConnection();

        /// Connects to a reX server.
        /** @param username Username (Firstname Lastname).
          * @param password Password.
          * @param serveraddress Simulator server address and port.
          * @param auth_server_address Authentication server address and port.
          * @param auth_login Authentication server username.
          * @return True if the connection was succesfull. False otherwise.
          */
        bool ConnectToServer(
            const std::string& username,
            const std::string& password,
            const std::string& serveraddress,
            const std::string& auth_server_address = "",
            const std::string& auth_login = "");

        /// Connect to a rex server with cablebeach/openid
        bool ConnectToCableBeachServer(
            const std::string& firstname, 
            const std::string& lastname,
            int port,
            const std::string& serveraddress);

        /// Creates the UDP connection after a succesfull XML-RPC login.
        /// @return True, if success.
        bool CreateUDPConnection();

        /// Request logout from the server.
        void RequestLogout();

        /// Disconnects from the server. Doesn't send the LogOutRequest packet.
        void ForceServerDisconnect();

        /// Set the connection type.
        /// @param ConnectionType enum.
        void SetConnectionType(const ConnectionType &type) { connection_type_ = type; }

        /// Get the connection type.
        /// @return ConnectionType enum.
        ConnectionType GetConnectionType() const { return connection_type_; }

        /// Send the UDP chat packet.
        void SendChatFromViewerPacket(const std::string &text);

        /// Sends the first UDP packet to open up the circuit with the server.
        void SendUseCircuitCodePacket();

        /// Signals that agent is coming into the region. The region should be expecting the agent.
        /// Server starts to send object updates etc after it has received this packet.
        void SendCompleteAgentMovementPacket();

        /// Sends wearables request to the server
        /// In reX mode, this causes the server to send the avatar appearance address
        void SendAgentWearablesRequestPacket();

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

        /** Sends a packet which creates a new inventory folder.
         *  @param parent_id The parent folder UUID.
         *  @param folder_id UUID of the folder.
         *  @param type Asset type for the folder.
         *  @param name Name of the folder.
         */
        void SendCreateInventoryFolderPacket(
            const RexTypes::RexUUID &parent_id,
            const RexTypes::RexUUID &folder_id,
            const RexTypes::asset_type_t &type,
            const std::string &name);

        /** Sends a which moves inventory folder and its contains to other folder.
         *  Used when deleting/removing folders to the Trash folder.
         *  @param parent_id The parent folder (folder where we want to move another foldeR) UUID.
         *  @param folder_id UUID of the folder to be moved..
         *  @param re_timestamp Should the server re-timestamp children.
         */
        void SendMoveInventoryFolderPacket(
            const RexTypes::RexUUID &folder_id,
            const RexTypes::RexUUID &parent_id,
            const bool &re_timestamp = true);

        /** Sends a packet which deletes inventory folder.
         *  @param folders List of new folders.
         *  @param re_timestamp Should the server re-timestamp children.
         */
        void SendMoveInventoryFolderPacket(
            std::list<OpenSimProtocol::InventoryFolder *> folders,
            const bool &re_timestamp = true);

        /// Send a packet which deletes inventory folder.
        /// Works when to folder is in the Trash folder.
        /// @param folder_id Folder ID.
        void SendRemoveInventoryFolderPacket(const RexTypes::RexUUID &folder_id);

        /// Send a packet which deletes inventory folders.
        /// Works when to folder is in the Trash folder.
        /// @param folders List of folders to be deleted.
        void RexServerConnection::SendRemoveInventoryFolderPacket(
            std::list<OpenSimProtocol::InventoryFolder *> folders);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id ID of the item to be moved.
        /// @param folder_id ID of the destionation folder.
        /// @param new_name New name for the item. Can be the same as before.
        /// @param re_timestamp Should the server re-timestamp children.
        void SendMoveInventoryItemPacket(
            const RexTypes::RexUUID &item_id,
            const RexTypes::RexUUID &folder_id,
            const std::string &new_name,
            const bool &re_timestamp = true);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id ID of the item to be moved.
        void SendRemoveInventoryItemPacket(const RexTypes::RexUUID &item_id);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id_list List of ID's of the items to be removed.
        void SendRemoveInventoryItemPacket(std::list<RexTypes::RexUUID> item_id_list);

        /** Sends a packet requesting contents of a inventory folder.
         *  @param folder_id Folder UUID.
         *  @param owner_id Owner UUID.
         *  @param sort_order Sort order, 0 = name, 1 = time.
         *  @param fetch_folders False will omit folders in query.
         *  @param fetch_items False will omit items in query.
         */
        void SendFetchInventoryDescendents(
            const RexTypes::RexUUID &folder_id,
            const RexTypes::RexUUID &owner_id,
            const int32_t &sort_order,
            const bool &fetch_folders,
            const bool &fetch_items);

        /// @return Name of the sim we're connected to.
        std::string GetSimName() const { return simName_; }

        /// @return A structure of connection spesific information, e.g. AgentID and SessionID.
        OpenSimProtocol::ClientParameters GetInfo() const { return myInfo_; }

        /// @return A structure of connection spesific information, e.g. AgentID and SessionID.
        /// @param name Name of the capability.
        std::string GetCapability(const std::string &name);

        /// @return True if the client connected to a server.
        const bool IsConnected() const { return connected_; }

        /// @return The state of the connection.
        volatile OpenSimProtocol::Connection::State GetConnectionState();

    private:
        /// Convenience function to get the weak pointer when building messages.
        NetOutMessage *StartMessageBuilding(const NetMsgID &message_id);

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
