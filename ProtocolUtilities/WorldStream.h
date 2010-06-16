// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ProtocolUtilities_WorldStream_h
#define incl_ProtocolUtilities_WorldStream_h

#include "Interfaces/ProtocolModuleInterface.h"
#include "Vector3D.h"
#include "Quaternion.h"
#include "NetworkEvents.h"

#include <QObject>

namespace OpenSimProtocol
{
    class ProtocolModuleOpenSim;
}

namespace TaigaProtocol
{
    class ProtocolModuleTaiga;
}

namespace Foundation
{
    class Framework;
}

class EC_OpenSimPrim;
class RexUUID;

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;

    /// Connection type enumeration.
    enum ConnectionType
    {
        DirectConnection = 0,
        AuthenticationConnection
    };

    /// Struct for object name update
    struct ObjectNameInfo
    {
        entity_id_t local_id_;
        std::string name_;
    };

    /// Struct for multipleobject update
    struct MultiObjectUpdateInfo
    {
        entity_id_t local_id_;
        Vector3df position_;
        Quaternion orientation_;
        Vector3df scale_;
    };

    /// Struct for description update
    struct ObjectDescriptionInfo
    {
        entity_id_t local_id_;
        std::string description_;
    };

    class WorldStream : public QObject
    {
        friend class NetworkEventHandler;

        Q_OBJECT

    public:
        /// Constructor.
        /// @param framework Pointer to framework.
        explicit WorldStream(Foundation::Framework *framework);

        /// Destructor.
        virtual ~WorldStream();

    public slots:
        //------------------- Connection managing functions ------------------- //

        /// Creates the UDP connection after a succesfull XML-RPC login.
        /// @return True, if success.
        bool CreateUdpConnection();

        /// Request logout from the server.
        void RequestLogout();

        /// Disconnects from the server. Doesn't send the LogOutRequest packet.
        void ForceServerDisconnect();

        //------------------- Packet sending functions ------------------- //

        /// Send the UDP chat packet.
        void SendChatFromViewerPacket(const std::string &text, s32 channel = 0);

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
        void SendObjectDeletePacket(const uint32_t &local_id, const bool force = false);

        /// Sends a message which requests object removal.
        /// @param local_id_list List of Local ID's.
        /// @param force God trying to force delete.
        void SendObjectDeletePacket(const std::vector<uint32_t> &local_id_list, const bool force = false);

        // Sends the basic movement message
        void SendAgentUpdatePacket(
            Quaternion bodyrot,
            Quaternion headrot,
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
        void SendObjectSelectPacket(const unsigned int object_id);

        /// Sends a packet which indicates selection of a prim.
        /// @param List of local ID's of objects which are selected.
        void SendObjectSelectPacket(std::vector<entity_id_t> object_id_list);

        /// Sends a packet which indicates deselection of prim(s).
        /// @param Local ID of the object which is deselected.
        void SendObjectDeselectPacket(entity_id_t object_id);

        /// Sends an object shape update
        /// @param prim to update
        void SendObjectShapeUpdate(const EC_OpenSimPrim &prim);

        /// Sends a packet which indicates deselection of a group of prims.
        /// @param List of local ID's of objects which are deselected.
        void SendObjectDeselectPacket(std::vector<entity_id_t> object_id_list);

        /// Sends a packet indicating change in Object's position, rotation and scale.
        /// @param List of updated entity id's/pos/rot/scale
        void SendMultipleObjectUpdatePacket(const std::vector<MultiObjectUpdateInfo>& update_info_list);

        // Convienience function for sending only one instead of list
        // @param name info struct
        void SendObjectNamePacket(const ObjectNameInfo& name_info);

        /// Sends a packet indicating change in Object's name.
        /// @param List of updated entity ids/names
        void SendObjectNamePacket(const std::vector<ObjectNameInfo>& name_info_list);

        /// Sends a packet which indicates object has been touched.
        /// @param Local ID of the object which has been touched.
        void SendObjectGrabPacket(entity_id_t object_id);
        
        // Convienience function for sending only one instead of list
        // @param description info struct
        void SendObjectDescriptionPacket(const ObjectDescriptionInfo& description_info);

        /// Sends a packet indicating change in Object's description
        /// @param List of updated entity pointers.
        void SendObjectDescriptionPacket(const std::vector<ObjectDescriptionInfo>& description_info_list);

        /// Sends handshake reply packet
        void SendRegionHandshakeReplyPacket(const RexUUID &agent_id, const RexUUID &session_id, uint32_t flags);

        /// Sends hardcoded agentappearance packet
        void SendAgentSetAppearancePacket();

        /** Sends packet which modifies the world terrain.
         *  @param x World x position
         *  @param y World y position
         *  @param brush Brush size (small = 0, medium = 1, large = 2)
         *  @param action Modify land action type (flatten = 0, raise = 1, lower = 2, smooth = 3, roughen = 4, revert = 5)
         *  @param seconds How long terrain has been modified on viewer (delta time).
         *  @param height Previous height value on spesific world position.
         */
        void SendModifyLandPacket(f32 x, f32 y, u8 brush, u8 action, Real seconds, Real height);

        /** Send a new terrain texture that we want to use.
         *  @param new_texture_id id for asset resouce that we want to use as our terrain texture.
         *  @param texture_index switch texture we want to change currently supports 4 different textures. (0 = lowest and 3 = highest)
         */
        void SendTextureDetail(const RexTypes::RexAssetID &new_texture_id, uint texture_index);

        /** Sends EstateOwnerMessage that will inculde data of terrain texture height, range and corner.
        /*  @param start_height height value where texture start to show (meters).
         *  @param height_range how much up texture will go from the texture_start_height (meters)
         *  @param corner what corner will the texture be used (0 = SW, 1 = NW, 2 = SE and 3 = NE)
         *          Note: in Rex this variable will only tell what texture height values we are changing.
         */
        void SendTextureHeightsMessage(Real start_height, Real height_range, uint corner);

        /// Request new region information from the server(RegionHandshake is sented every client on that server)
        /// ReqionHandshakeMessage will contain all new information about spesific region (e.g. new TerrainBase/TerrainDetail textures,
        /// terrain texture startheights/ranges and WaterHeight)
        void SendTextureCommitMessage();

        /** Sends a packet which creates a new inventory folder.
         *  @param parent_id The parent folder UUID.
         *  @param folder_id UUID of the folder.
         *  @param type Asset type for the folder.
         *  @param name Name of the folder.
         */
        void SendCreateInventoryFolderPacket(
            const RexUUID &parent_id,
            const RexUUID &folder_id,
            const asset_type_t &type,
            const std::string &name);

        /** Sends a which moves inventory folder and its contains to other folder.
         *  Used when deleting/removing folders to the Trash folder.
         *  @param parent_id The parent folder (folder where we want to move another folder) UUID.
         *  @param folder_id UUID of the folder to be moved.
         *  @param re_timestamp Should the server re-timestamp children.
         */
        void SendMoveInventoryFolderPacket(
            const RexUUID &folder_id,
            const RexUUID &parent_id,
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
        void SendRemoveInventoryFolderPacket(const RexUUID &folder_id);

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
            const RexUUID &item_id,
            const RexUUID &folder_id,
            const std::string &new_name,
            const bool &re_timestamp = true);

        /// Sends packet which requests an inventory item copy.
        /// @param old_agent_id Agent ID.
        /// @param old_item_id Item ID.
        /// @param new_folder_id Destionation folder ID.
        /// @param new_name New name for the item. Can be the same as before.
        void SendCopyInventoryItemPacket(
            const RexUUID &old_agent_id,
            const RexUUID &old_item_id,
            const RexUUID &new_folder_id,
            const std::string &new_name);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id ID of the item to be moved.
        void SendRemoveInventoryItemPacket(const RexUUID &item_id);

        /// Sends packet which moves an inventory item to another folder.
        /// @param item_id_list List of ID's of the items to be removed.
        void SendRemoveInventoryItemPacket(std::list<RexUUID> item_id_list);

        /// Sends packet which modifies inventory folder's name and/or type.
        /// @param parent_id The parent folder ID.
        /// @param folder_id Folder ID.
        /// @param type New type.
        /// @param name New name.
        void SendUpdateInventoryFolderPacket(
            const RexUUID &folder_id,
            const RexUUID &parent_id,
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
            const RexUUID &item_id,
            const RexUUID &folder_id,
            const asset_type_t &asset_type,
            const inventory_type_t &inventory_type,
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
            const RexUUID &folder_id,
            const RexUUID &owner_id = RexUUID(),
            const int32_t &sort_order = 0,
            const bool &fetch_folders = true,
            const bool &fetch_items = true);

        /**
         *  Send a packet to Opensim server to accept friend request
         *  @param transaction_id Unknown
         *  @param folder_id Folder for calling card for this friend
         *  @todo Find out the meaning of transaction_id argument
         */
        void SendAcceptFriendshipPacket(const RexUUID &transaction_id, const RexUUID &folder_id);

        /**
         *  Send a packet to Opensim server to decline received friend request
         *  @param transaction_id Unknown
         *  @todo Find out the meaning of transaction_id argument
         */
        void SendDeclineFriendshipPacket(const RexUUID &transaction_id);

        /**
         *  Send friend request to Opensim server
         *  @param dest_id Target id
         */
        void SendFormFriendshipPacket( const RexUUID &dest_id);

        /**
         *  Sends a packet to remove friend from friend list.
         *  @param other_id Unknown
         *  @todo Find out meaning of the other_id argument
         */
        void SendTerminateFriendshipPacket(const RexUUID &other_id);

        /**
         *
         *
         */
        void SendImprovedInstantMessagePacket(const RexUUID &target, const std::string &text);

        //! Sends a generic message
        //! a qt overload version for pythonqt & qtscript
        void SendGenericMessage(const QString &method, const QStringList& strings);

        //! Sends a generic message
        /*! \param method Method name
            \param strings Vector of data strings to be sent
         */
        void SendGenericMessage(const std::string& method, const StringVector& strings);

        //! Sends a generic message with binary data packed to the end
        /*! \param method Method name
            \param strings Vector of data strings to be sent
            \param binary Vector of binary data. Will be split into smaller chunks as necessary
         */
        void SendGenericMessageBinary(
            const std::string& method,
            const StringVector& strings,
            const std::vector<uint8_t>& binary);

        /// Sends a packet informing server that client is blocked (e.g. modal File Open window is blocking main thread).
        void SendAgentPausePacket();

        /// Sends a packet informing server that client is running normal again.
        void SendAgentResumePacket();

        /// sends the derez packet for the entity ent_id (have to use unsigned longs for PythonQt correctness...)
        void SendObjectDeRezPacket(const unsigned long ent_id, const QString &trash_id);

        /// sends the undo packet for the entity ent_id
        void SendObjectUndoPacket(const QString &ent_id);

        /// sends the redo packet for the entity ent_id
        void SendObjectRedoPacket(const QString &ent_id);

        /// duplicate the object (have to use unsigned longs for PythonQt correctness...)
        void SendObjectDuplicatePacket(
            const unsigned long ent_id,
            const unsigned long flags,
            const Vector3df &offset);

        /// same as above but takes the offset vector as three floats (have to use unsigned longs for PythonQt correctness...)
        void SendObjectDuplicatePacket(
            const unsigned long ent_id,
            const unsigned long flags,
            const float offset_x,
            const float offset_y,
            const float offset_z);

        /// without the offset, reverts to Vector.ZERO (have to use unsigned longs for PythonQt correctness...)
        void SendObjectDuplicatePacket(const unsigned long ent_id, const unsigned long flags);

        /// Sends an UUIDNameRequest for UUID-username lookup.
        ///@param user_id User ID.
        void SendUUIDNameRequestPacket(const RexUUID &user_id);

        /// Sends an UUIDNameRequest for UUID-username lookup.
        /// Translate a UUID into first and last names
        ///@param user_ids List of user ID's.
        void SendUUIDNameRequestPacket(const std::vector<RexUUID> &user_ids);

        /// Sends an UUIDGroupNameRequest packet for UUID-group name lookup.
        ///@param group_id Group ID.
        void SendUUIDGroupNameRequestPacket(const RexUUID &group_id);

        /// Sends an UUIDGroupNameRequest packet for UUID-group name lookup.
        ///@param group_ids List of group ID's.
        void SendUUIDGroupNameRequestPacket(const std::vector<RexUUID> &group_ids);

        /// Sends an ObjectLink packet.
        ///@param local_ids List of local entity ID's.
        void SendObjectLinkPacket(const std::vector<entity_id_t> &local_ids);
        void SendObjectLinkPacket(const QStringList& strings);

        /// Sends an ObjectDelink packet.
        ///@param local_ids List of local entity ID's.
        void SendObjectDelinkPacket(const std::vector<entity_id_t> &local_ids);
        void SendObjectDelinkPacket(const QStringList& strings);

        /// Send MapBlockRequest
        void SendMapBlockRequest();

        /// Sends RequestGodlikePowers packet.
        /// @param godlike Do we want to be requesting enablind or disabling god-like powers.
        void SendRequestGodlikePowersPacket(const bool godlike);

        /// Sends GodKickUser packet to kick out user from the server.
        /// @param user_id ID of the user we wan't to kick.
        /// @param reason Reason of the kicking.
        void SendGodKickUserPacket(const RexUUID &user_id, const std::string &reason);

        /// Send GodKickUser packet with params as QString's
        /// @param user_id ID of the user we wan't to kick.
        /// @param reason Reason of the kicking.
        void SendGodKickUserPacket(const QString &user_id, const QString &reason);

        /// Sends EstateInfoRequest packet to get current estate info.
        /// @param method EstateInfoRequest method name
        /// @param paramlist parameters
        void SendEstateOwnerMessage(const QString &method, const QStringList& paramlist);

        //------------------- Utility functions ------------------- //

        /// Set the connection type.
        /// @param ConnectionType enum.
        void SetConnectionType(const ConnectionType &type) { connection_type_ = type; }

        /// Get the connection type.
        /// @return ConnectionType enum.
        ConnectionType GetConnectionType() const { return connection_type_; }

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

        /// @return Name of the sim we're connected to.
        std::string GetSimName() const { return simName_; }

        /// Set sim name
        void SetSimName(const std::string &newSimName) { simName_ = newSimName; }

        /// @return A structure of connection spesific information, e.g. AgentID and SessionID.
        ClientParameters GetInfo() const { return clientParameters_; }

        /// Returns capability URL by name.
        /// @param name Name of the capability.
        /// @return Capability URL or empty string if capability doesn't exist network interface not available.
        QString GetCapability(const QString &name) const;

        /// @return Last used password
        const std::string& GetPassword() const { return password_; }

        /// @return Last used username
        const std::string& GetUsername() const { return username_; }

        /// @return Last used authentication address
        const std::string& GetAuthAddress() const { return auth_server_address_; }

        /// @return True if the client connected to a server.
        const bool IsConnected() const { return connected_; }

        /// @return The state of the connection.
        volatile Connection::State GetConnectionState();

        /// Set new state
        void SetConnectionState(Connection::State newstate);

        /// @return Error message of the connection.
        std::string GetConnectionErrorMessage();

        /// @param protocol type
        void SetCurrentProtocolType(ProtocolType newType);

        /// @return The current Protocol Module type
        boost::shared_ptr<ProtocolModuleInterface> GetCurrentProtocolModule() const;

        /// Get boost::weak_ptr to current Protocol Module
        /// @return boost::weak_ptr to current Protocol Module
        boost::weak_ptr<ProtocolModuleInterface> GetCurrentProtocolModuleWeakPointer() const;

        /// Prepares the network events of current module
        /// @return True if preparations succeeded
        bool PrepareCurrentProtocolModule();

        /// Unregisters the eventmanager from current Protocol Module
        void UnregisterCurrentProtocolModule();

    private:
        Q_DISABLE_COPY(WorldStream);

        /// Sends all the needed packets to server when connection successfull
        void SendLoginSuccessfullPackets();

        /// Sends map request packet when udp connection has been created successfully
        void SendMapBlockPacket();

        /// Convenience function to get the weak pointer when building messages.
        NetOutMessage *StartMessageBuilding(const NetMsgID &message_id);

        /// Convenience function to get the weak pointer when sending messages.
        void FinishMessageBuilding(NetOutMessage *msg);

        /// WriteFloatToBytes
        void WriteFloatToBytes(float value, uint8_t* bytes, int& idx);

        /// The framework we belong to.
        Foundation::Framework *framework_;

        /// Pointer for OpenSim protocol module.
        boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> netInterfaceTaiga_;

        /// Pointer for Taiga protocol module.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> netInterfaceOpenSim_;

        /// Pointer to ModuleImplementation, used in ProtocolModule getter
        boost::shared_ptr<ProtocolModuleInterface> protocolModule_;

        /// Server-spesific info for this client.
        ClientParameters clientParameters_;

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
        Connection::State state_;

        /// State of the connection procedure thread.
        ConnectionThreadState threadState_;

        /// Current ProtocolModule type
        ProtocolType currentProtocolType_;

        /// Block serial number used for AgentPause and AgentResume messages.
        uint32_t blockSerialNumber_;
    };
}

#endif // incl_ProtocolUtilities_WorldStream_h
