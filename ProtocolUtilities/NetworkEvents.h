// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Protocol_NetworkEvents_h
#define incl_Protocol_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "NetworkMessages/NetMessage.h"
//#include "NetworkMessages/NetInMessage.h"
//#include "NetworkMessages/NetOutMessage.h"
//#include "Inventory/InventorySkeleton.h"
//#include "OpenSim/BuddyList.h"

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
        boost::shared_ptr<InventorySkeleton> inventory;
        boost::shared_ptr<BuddyList> buddy_list;
    };

    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    /// \ingroup OpenSimProtocolClient 
    namespace Events
    {
        /**
         * Notify event which is sent when OpenSimProtocol module has disconnect connection between server and client. 
         * @note this event should never send from outside module. Use DisconnectFromServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_DISCONNECTED = 0x01;

        /** 
         * Notify event which is send when connection is made to the server. 
         * @note this event should never send from outside module. Use ConnectToServer() or similar method.
         */
        static const event_id_t EVENT_SERVER_CONNECTED = 0x02;

        /**
         * Notify event which can be send when connection has failed. 
         * @note this event should never send from outside module. 
         */
        static const event_id_t EVENT_CONNECTION_FAILED = 0x03;

        /**
         * Notifies when a ProtocolModule has registered networking category
         * so other modules can query the event category
         */
        static const event_id_t EVENT_NETWORKING_REGISTERED = 0x04;
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
        AuthenticationEventData(const AuthenticationType &auth_type, const std::string &identity_url = "", const std::string &host_Url = "") 
            : type(auth_type), identityUrl(identity_url), hostUrl(host_Url) {}
        virtual ~AuthenticationEventData() {}
        void SetIdentity(const std::string &url) { identityUrl = url; }
        void SetHost(const std::string &url) { hostUrl = url; }
        AuthenticationType type;
        std::string identityUrl;
        std::string hostUrl;
        InventorySkeleton *inventorySkeleton;
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
}

#endif // incl_Protocol_NetworkEvents_h
