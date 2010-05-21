// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_NetworkEvents_h
#define incl_Protocol_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "NetworkMessages/NetMessage.h"

#include <boost/smart_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace ProtocolUtilities
{
    class NetInMessage;
    class NetOutMessage;
    class ProtocolModuleInterface;
    class WorldStream;
    class InventorySkeleton;
    class BuddyList;

    typedef boost::shared_ptr<InventorySkeleton> InventoryPtr;

      /// Protocol type enumeration.
    enum ProtocolType
    {
        NotSet = 0,
        OpenSim,
        Taiga
    };

    /// Info structure used to pass messages between the main thread and the XMLRPC connect thread.
    struct ClientParameters
    {
    public:
        /// Default constructor.
        ClientParameters();

        /// Resets parameters.
        void Reset();

        RexUUID agentID;
        RexUUID sessionID;
        RexUUID regionID;
        uint32_t circuitCode;
        std::string sessionHash;
        std::string gridUrl;
        std::string avatarStorageUrl;
        std::string seedCapabilities;
        std::string webdavInventoryUrl;
        boost::shared_ptr<InventorySkeleton> inventory;
        boost::shared_ptr<BuddyList> buddy_list;
        uint16_t regionX;
        uint16_t regionY;
    };

    // MapBlock structs for teleport and map related packets
    struct MapBlock
    {
        RexUUID agentID;
        uint32_t flags;
        uint16_t regionX;
        uint16_t regionY;
        std::string regionName;
        uint8_t access;
        uint32_t regionFlags;
        uint8_t waterHeight;
        uint8_t agents;
        RexUUID mapImageID;
    };

    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    /// \ingroup OpenSimProtocolClient 
    namespace Events
    {
        /**
         *  Notify event which is sent when OpenSimProtocol module has disconnect connection between server and client. 
         *  @note this event should never send from outside module. Use DisconnectFromServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_DISCONNECTED = 0x01;

        /** 
         *  Notify event which is send when connection is made to the server. 
         *  @note this event should never send from outside module. Use ConnectToServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_CONNECTED = 0x02;

        /**
         *  Notify event which can be send when connection has failed. 
         *  @note this event should never send from outside module. 
         */
        static const event_id_t EVENT_CONNECTION_FAILED = 0x03;

        /**
         *  Notifies when a ProtocolModule has registered networking category
         *  so other modules can query the event category
         */
        static const event_id_t EVENT_NETWORKING_REGISTERED = 0x04;

        /**
         *  Notifies that new user (not us) has connected to the world.
         */
        static const event_id_t EVENT_USER_CONNECTED = 0x05;

        /**
         *  Notifies that user (not us) has disconnected to the world.
         */
        static const event_id_t EVENT_USER_DISCONNECTED = 0x06;

        /**
         *  Notifies that caps fetch is done. You can query caps from world stream after this is sent.
         */
        static const event_id_t EVENT_CAPS_FETCHED = 0x07;

        static const event_id_t EVENT_LOGIN_INFO_AQQUIRED = 0x08;
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
        volatile Connection::State state;
        ClientParameters parameters;
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

    /// Event data interface for authentication type identification.
    /// \ingroup OpenSimProtocolClient
    class AuthenticationEventData : public Foundation::EventDataInterface
    {
    public:
        AuthenticationEventData(const AuthenticationType &auth_type, const std::string &identity = "", const std::string &host = "") 
            : type(auth_type), webdav_identity(identity), webdav_host(host) {}
        virtual ~AuthenticationEventData() {}
        void SetIdentity(const std::string &identity) { webdav_identity = identity; }
        void SetHost(const std::string &url) { webdav_host = url; }
        AuthenticationType type;
        std::string webdav_identity;
        std::string webdav_host;
        std::string webdav_password;
        InventoryPtr inventorySkeleton;
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

    class NetworkingRegisteredEvent : public Foundation::EventDataInterface
    {
    public:
        explicit NetworkingRegisteredEvent(boost::weak_ptr<ProtocolModuleInterface> pModule);
        virtual ~NetworkingRegisteredEvent();

        // boost weak pointer to the changed protocolmodule
        boost::weak_ptr<ProtocolModuleInterface> currentProtocolModule;
    };

    class WorldStreamReadyEvent : public Foundation::EventDataInterface
    {
    public:
        explicit WorldStreamReadyEvent(boost::shared_ptr<ProtocolUtilities::WorldStream> currentWorldStream);
        virtual ~WorldStreamReadyEvent();
        // Pointer to the current WorldStream
        boost::shared_ptr<ProtocolUtilities::WorldStream> WorldStream;
    };

    /// Event data interface for EVENT_USER_CONNECTED and EVENT_USER_DISCONNECTED
    class UserConnectivityEvent : public Foundation::EventDataInterface
    {
    public:
        /// Constructor
        /// @param agent_id Agent ID of the user.
        explicit UserConnectivityEvent(const RexUUID &agent_id) : agentId(agent_id) {}
        /// Agent ID.
        RexUUID agentId;
        /// Local ID.
        int32_t localId;
        /// Name.
        std::string fullName;
    };

    class LoginDataEvent : public Foundation::EventDataInterface
    {
    public:
        explicit LoginDataEvent(std::string base_address, int port) : base_address_(base_address), port_(port) {}
        virtual ~LoginDataEvent() {}
        std::string base_address_;
        int port_;
    };
}

#endif // incl_Protocol_NetworkEvents_h
