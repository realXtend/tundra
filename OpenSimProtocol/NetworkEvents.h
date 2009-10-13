// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEvents_h
#define incl_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "NetInMessage.h"
#include "NetOutMessage.h"
#include "Inventory.h"

namespace OpenSimProtocol
{
    /// Info structure used to pass messages between the main thread and the XMLRPC connect thread.
    struct ClientParameters
    {
    public:
        /// Default constructor.
        ClientParameters() :
            agentID(RexUUID()),
            sessionID(RexUUID()),
            regionID(RexUUID()),
            circuitCode(0),
            sessionHash(""),
            gridUrl(""),
            avatarStorageUrl(""),
            seedCapabilities("") {}

        /// Resets parameters.
        void Reset()
        {
            agentID = RexUUID();
            sessionID = RexUUID();
            regionID = RexUUID();
            uint32_t circuitCode = 0;
            std::string sessionHash = "";
            std::string gridUrl = "";
            std::string avatarStorageUrl = "";
            std::string seedCapabilities = "";
            //inventory.reset();
        }

        RexUUID agentID;
        RexUUID sessionID;
        RexUUID regionID;
        uint32_t circuitCode;
        std::string sessionHash;
        std::string gridUrl;
        std::string avatarStorageUrl;
        std::string seedCapabilities;
        boost::shared_ptr<InventorySkeleton> inventory;
    };

    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    /// \ingroup OpenSimProtocolClient 
    namespace Events
    {
        /**
         * Notify event which is sent when OpenSimProtocol module has disconnect connection between server and client. 
         * @note this event should never send from outside module. Use DisconnectFromServer() or similar method.
         */
        static const Core::event_id_t EVENT_SERVER_DISCONNECTED = 0x01;

        /** 
         * Notify event which is send when connection is made to the server. 
         * @note this event should never send from outside module. Use ConnectToServer() or similar method.
         */
        static const Core::event_id_t EVENT_SERVER_CONNECTED = 0x02;

        /**
         * Notify event which can be send when connection has failed. 
         * @note this event should never send from outside module. 
         */
        static const Core::event_id_t EVENT_CONNECTION_FAILED = 0x03;
    }

    /// Enumeration of the network connection states.
    /// When modified, update also the connection_strings table of the NetworkStateToString function.
    namespace Connection
    {
        enum State
        {
            STATE_LOGIN_FAILED = 0,
            STATE_DISCONNECTED,
            STATE_INIT_XMLRPC,
            STATE_WAITING_FOR_XMLRPC_REPLY,
            STATE_XMLRPC_AUTH_REPLY_RECEIVED,
            STATE_XMLRPC_REPLY_RECEIVED,
            STATE_INIT_UDP,
            STATE_CONNECTED,
            STATE_ENUM_COUNT
        };

        /// Utility function for converting the connection state enum to string.
        ///@param The connection state enum.
        ///@return The login state as a string.
        static const std::string &NetworkStateToString(State state)
        {
             static const std::string connection_strings[STATE_ENUM_COUNT] = {
                 "Login failed", "Disconnected", "Initializing XML-RPC connection", "Waiting for XML-RPC reply",
                 "XML-RPC authentication reply received", "XML-RPC login reply received", "Initalizing UDP connection",
                 "Connected" };

            assert(state >= 0);
            assert(state < STATE_ENUM_COUNT);
            return connection_strings[state];
        }
    }

    /// Data struct which is passed to the login thread.
    struct ConnectionThreadState
    {
        volatile OpenSimProtocol::Connection::State state;
        OpenSimProtocol::ClientParameters parameters;
        std::string errorMessage;
    };

    /// Enumeration of different authentication methods.
    enum AuthenticationType
    {
        AT_Taiga = 0,
        AT_OpenSim,
        AT_RealXtend,
        AT_Unknown
    };

    ///\todo Move inventory stuff to somewhere else.
    namespace InventoryEvents
    {
        static const Core::event_id_t EVENT_INVENTORY_FOLDER_DESCENDENTS = 0x04;
        static const Core::event_id_t EVENT_INVENTORY_ASSET_DESCENDENTS = 0x05;
    }

    /// Event data interface for inventory folders.
    /// \ingroup OpenSimProtocolClient
    class InventoryFolderEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryFolderEventData() {}
        virtual ~InventoryFolderEventData() {}
        RexUUID folderId;
        RexUUID parentId;
        inventory_type_t type;
        std::string name;
    };

        /// Event data interface for inventory assets.
    /// \ingroup OpenSimProtocolClient
    class InventoryAssetEventData : public Foundation::EventDataInterface
    {
    public:
        InventoryAssetEventData() {}
        virtual ~InventoryAssetEventData() {}
    };

    /// Event data interface for authentication type identification.
    /// \ingroup OpenSimProtocolClient
    class AuthenticationEventData : public Foundation::EventDataInterface
    {
    public:
        AuthenticationEventData(const AuthenticationType &auth_type, const std::string &identity_url = "") :
            type(auth_type), identityUrl(identity_url) {}
        virtual ~AuthenticationEventData() {}
        AuthenticationType type;
        std::string identityUrl;
    };

    /// Event data interface for inbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventInboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventInboundData(NetMsgID id, NetInMessage *msg) :
            message(msg), messageID(id) {}
        virtual ~NetworkEventInboundData() {}

        NetMsgID messageID;
        NetInMessage *message;
    };

    /// Event data interface for outbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventOutboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventOutboundData(NetMsgID id, const NetOutMessage *msg) :
            message(msg), messageID(id) {}
        virtual ~NetworkEventOutboundData() {}

        NetMsgID messageID;
        const NetOutMessage *message;
    };
}

#endif
