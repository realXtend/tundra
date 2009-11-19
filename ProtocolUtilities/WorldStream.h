// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_WorldStream_h
#define incl_ProtocolUtilities_WorldStream_h

#include "Interfaces/ProtocolModuleInterface.h"
#include "NetworkEvents.h"
#include "RexUUID.h"

#include <QObject>

namespace OpenSimProtocol
{
    class ProtocolModuleOpenSim;
}

namespace TaigaProtocol
{
    class ProtocolModuleTaiga;
}

namespace ProtocolUtilities
{
    typedef boost::shared_ptr<ProtocolUtilities::WorldStream> WorldStreamPtr;

    /// Connection type enumeration.
    enum ConnectionType
    {
        DirectConnection = 0,
        AuthenticationConnection
    };

    class WorldStream : public QObject
    {
        friend class NetworkEventHandler;

        Q_OBJECT

    public:
        MODULE_LOGGING_FUNCTIONS;
        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return LoggerName; }
        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;

        /// Constructor.
        /// @param framework Pointer to framework.
        WorldStream(Foundation::Framework *framework);

        /// Destructor.
        virtual ~WorldStream();

    public slots:
        /// Creates the UDP connection after a succesfull XML-RPC login.
        /// @return True, if success.
        bool CreateUdpConnection();

        /// Request logout from the server.
        void RequestLogout();

        /// Disconnects from the server. Doesn't send the LogOutRequest packet.
        void ForceServerDisconnect();

        /// Set the connection type.
        /// @param ConnectionType enum.
        void SetConnectionType(const ConnectionType &type) { connection_type_ = type; }

        /// Set credential info
        /// @param identity Identity
        /// @param password Password (if known)
        /// @param authentication Authentication url (if known)
        void StoreCredentials(const std::string& identity, const std::string& password, const std::string& authentication)
        {
            username_ = identity;
            password_ = password;
            auth_server_address_ = authentication;
        }

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

        /// Sends a RexStartup state generic message
        void SendRexStartupPacket(const std::string& state);

        /// Sends a message requesting logout from the server. The server is then going to flood us with some
        /// inventory UUIDs after that, but we'll be ignoring those.
        void SendLogoutRequestPacket();

        /// Sends a message which creates a default prim the in world.
        /// @param position Position in the world.
        void SendObjectAddPacket(const RexTypes::Vector3 &position);

        /// Sends a message which requests object removal.
        /// @param local_id Local ID.
        /// @param force God trying to force delete.
        void SendObjectDeletePacket(const uint32_t &local_id, const bool &force = false);

        /// Sends a message which requests object removal.
        /// @param local_id_list List of Local ID's.
        /// @param force God trying to force delete.
        void SendObjectDeletePacket(const std::vector<uint32_t> &local_id_list, const bool &force = false);

        // Sends the basic movement message
        void SendAgentUpdatePacket(
            Core::Quaternion bodyrot,
            Core::Quaternion headrot,
            uint8_t state,
            RexTypes::Vector3 camcenter,
            RexTypes::Vector3 camataxis,
            RexTypes::Vector3 camleftaxis,
            RexTypes::Vector3 camupaxis,
            float far,
            uint32_t controlflags,
            uint8_t flags);

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
         *  @param parent_id The parent folder (folder where we want to move another folder) UUID.
         *  @param folder_id UUID of the folder to be moved.
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
        //void SendMoveInventoryFolderPacket(
        //    std::list<ProtocolUtilities::InventoryFolderSkeleton *> folders,
        //    const bool &re_timestamp = true);

        /// Send a packet which deletes inventory folder.
        /// Works when to folder is in the Trash folder.
        /// @param folder_id Folder ID.
        void SendRemoveInventoryFolderPacket(const RexTypes::RexUUID &folder_id);

        /// Send a packet which deletes inventory folders.
        /// Works when to folder is in the Trash folder.
        /// @param folders List of folders to be deleted.
        //void SendRemoveInventoryFolderPacket(
        //    std::list<ProtocolUtilities::InventoryFolderSkeleton *> folders);

        /// Sends packet which moves an inventory item to another folder within My Inventory.
        /// @param item_id ID of the item to be moved.
        /// @param folder_id ID of the destionation folder.
        /// @param new_name New name for the item. Can be the same as before.
        /// @param re_timestamp Should the server re-timestamp children.
        void SendMoveInventoryItemPacket(
            const RexTypes::RexUUID &item_id,
            const RexTypes::RexUUID &folder_id,
            const std::string &new_name,
            const bool &re_timestamp = true);

        /// Sends packet which requests an inventory item copy.
        /// @param old_agent_id Agent ID.
        /// @param old_item_id Item ID.
        /// @param new_folder_id Destionation folder ID.
        /// @param new_name New name for the item. Can be the same as before.
        void SendCopyInventoryItemPacket(
            const RexTypes::RexUUID &old_agent_id,
            const RexTypes::RexUUID &old_item_id,
            const RexTypes::RexUUID &new_folder_id,
            const std::string &new_name);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id ID of the item to be moved.
        void SendRemoveInventoryItemPacket(const RexTypes::RexUUID &item_id);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id_list List of ID's of the items to be removed.
        void SendRemoveInventoryItemPacket(std::list<RexTypes::RexUUID> item_id_list);

        /// Sends packet which modifies inventory folder's name and/or type.
        /// @param parent_id The parent folder ID.
        /// @param folder_id Folder ID.
        /// @param type New type.
        /// @param name New name.
        void SendUpdateInventoryFolderPacket(
            const RexTypes::RexUUID &folder_id,
            const RexTypes::RexUUID &parent_id,
            const int8_t &type,
            const std::string &name);

        /// Sends packet which modifies inventory item's name, description, type etc.
        /// @param item_id Item ID.
        /// @param folder_id Folder ID.
        /// @param asset_type Asset type.
        /// @param inventory_type Inventory type.
        /// @param name Name.
        /// @param description Description.
        void SendUpdateInventoryItemPacket(
            const RexTypes::RexUUID item_id,
            const RexTypes::RexUUID folder_id,
            const RexTypes::asset_type_t &asset_type,
            const RexTypes::inventory_type_t &inventory_type,
            const std::string &name,
            const std::string &description);

        /** Sends a packet requesting contents of a inventory folder.
         *  @param folder_id Folder UUID.
         *  @param owner_id Owner UUID. If null, we use agent ID.
         *  @param sort_order Sort order, 0 = name, 1 = time.
         *  @param fetch_folders False will omit folders in query.
         *  @param fetch_items False will omit items in query.
         */
        void SendFetchInventoryDescendentsPacket(
            const RexTypes::RexUUID &folder_id,
            const RexTypes::RexUUID &owner_id = RexTypes::RexUUID(),
            const int32_t &sort_order = 0,
            const bool &fetch_folders = true,
            const bool &fetch_items = true);

        /**
         *  Send a packet to Opensim server to accept friend request
         *  @param transaction_id Unknown
         *  @param folder_id Folder for calling card for this friend
         *  @todo Find out the meaning of transaction_id argument
         */
        void SendAcceptFriendshipPacket(const RexTypes::RexUUID &transaction_id, const RexTypes::RexUUID &folder_id);

        /**
         *  Send a packet to Opensim server to decline received friend request
         *  @param transaction_id Unknown
         *  @todo Find out the meaning of transaction_id argument
         */
        void SendDeclineFriendshipPacket(const RexTypes::RexUUID &transaction_id);

        /**
         *  Send friend request to Opensim server
         *  @param dest_id Target id
         */
        void SendFormFriendshipPacket( const RexTypes::RexUUID &dest_id);

        /**
         *  Sends a packet to remove friend from friend list.
         *  @param other_id Unknow
         *  @todo FInd out meaning of the other_id argument
         */
        void SendTerminateFriendshipPacket(const RexTypes::RexUUID &other_id);

        /**
         *
         *
         */
        void SendImprovedInstantMessagePacket(const RexTypes::RexUUID &target, const std::string &text);

        //! Sends a generic message
        /*! \param method Method name
            \param strings Vector of data strings to be sent
         */
        void SendGenericMessage(const std::string& method, const Core::StringVector& strings);

        //! Sends a generic message with binary data packed to the end
        /*! \param method Method name
            \param strings Vector of data strings to be sent
            \param binary Vector of binary data. Will be split into smaller chunks as necessary
         */
        void SendGenericMessageBinary(
            const std::string& method,
            const Core::StringVector& strings,
            const std::vector<uint8_t>& binary);

        /// Sends a packet informing server that client is blocked (e.g. modal File Open window is blocking main thread).
        void SendAgentPausePacket();

        /// Sends a packet informing server that client is running normal again.
        void SendAgentResumePacket();

        /// @return Name of the sim we're connected to.
        std::string GetSimName() const { return simName_; }

        /// Set sim name
        void SetSimName(const std::string &newSimName) { simName_ = newSimName; } 

        /// @return A structure of connection spesific information, e.g. AgentID and SessionID.
        ProtocolUtilities::ClientParameters GetInfo() const { return clientParameters_; }

        /// @return A capability by name
        /// @param name Name of the capability.
        std::string GetCapability(const std::string &name);

        /// @return Last used password
        const std::string& GetPassword() { return password_; }

        /// @return Last used username
        const std::string& GetUsername() { return username_; }

        /// @return Last used authentication address
        const std::string& GetAuthAddress() { return auth_server_address_; }

        /// @return True if the client connected to a server.
        const bool &IsConnected() const { return connected_; }

        /// @return The state of the connection.
        volatile ProtocolUtilities::Connection::State GetConnectionState();

        /// @return Error message of the connection.
        std::string GetConnectionErrorMessage();

        /// @param protocol type
        void SetCurrentProtocolType(ProtocolType newType);

        /// @return The current Protocol Module type
        boost::shared_ptr<ProtocolModuleInterface> GetCurrentProtocolModule();

        /// Get boost::weak_ptr to current Protocol Module
        /// @return boost::weak_ptr to current Protocol Module
        boost::weak_ptr<ProtocolModuleInterface> GetCurrentProtocolModuleWeakPointer();

        /// Prepares the network events of current module
        /// @return True if preparations succeeded
        bool PrepareCurrentProtocolModule();

        /// Unregisters the eventmanager from current Protocol Module
        void UnregisterCurrentProtocolModule();

    public:
        /// Name used for logging.
        static const std::string &LoggerName;

    private:
        /// Sends all the needed packets to server when connection successfull
        void SendLoginSuccessfullPackets();

        /// Convenience function to get the weak pointer when building messages.
        ProtocolUtilities::NetOutMessage *StartMessageBuilding(const ProtocolUtilities::NetMsgID &message_id);

        /// Convenience function to get the weak pointer when sending messages.
        void FinishMessageBuilding(ProtocolUtilities::NetOutMessage *msg);

        /// WriteFloatToBytes
        void WriteFloatToBytes(float value, uint8_t* bytes, int& idx);

        /// The framework we belong to.
        Foundation::Framework *framework_;

        /// Pointer for OpenSim protocol module.
        boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> netInterfaceTaiga_;

        /// Pointer for Taiga protocol module.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> netInterfaceOpenSim_;

        /// Pointer to ModuleImplementation, used in ProtocolModule getter
        boost::shared_ptr<ProtocolUtilities::ProtocolModuleInterface> protocolModule_;

        /// Server-spesific info for this client.
        ProtocolUtilities::ClientParameters clientParameters_;

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

        /// last password used for easy avatar export, probably a bit evil
        std::string password_;

        /// last username used for easy avatar export, probably a bit evil
        std::string username_;

        /// last auth.server used for easy avatar export, probably a bit evil
        std::string auth_server_address_;

        /// State of the connection procedure.
        ProtocolUtilities::Connection::State state_;

        /// State of the connection procedure thread.
        ProtocolUtilities::ConnectionThreadState threadState_;

        /// Current ProtocolModule type
        ProtocolUtilities::ProtocolType currentProtocolType_;

        /// Block serial number used for AgentPause and AgentResume messages.
        uint32_t blockSerialNumber_;
    };
}

#endif // incl_ProtocolUtilities_WorldStream_h
