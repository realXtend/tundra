// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEvents_h
#define incl_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "NetInMessage.h"
#include "NetOutMessage.h"

namespace OpenSimProtocol
{
    /// Info structure used to pass messages between the main thread and the XMLRPC connect thread.
    struct ClientParameters
    {
        RexUUID agentID;
        RexUUID sessionID;
        RexUUID regionID;
        uint32_t circuitCode;
        std::string sessionHash;
        std::string gridUrl;
        std::string avatarStorageUrl;
    };
    
    /// Defines the events posted by the OpenSimProtocolModule in category <b>NetworkState</b>.
    /// \ingroup OpenSimProtocolClient @{
    namespace Events
    {
        ///\todo Describe.
        static const Core::event_id_t EVENT_SERVER_DISCONNECTED = 0x01;
        static const Core::event_id_t EVENT_INIT_XMLRPC = 0x02;
        static const Core::event_id_t EVENT_WAITING_XMLRPC_REPLY = 0x03;
        static const Core::event_id_t EVENT_XMLRPC_AUTH_REPLY_RECEIVED = 0x04;
        static const Core::event_id_t EVENT_XMLRPC_LOGIN_REPLY_RECEIVED = 0x05;
        static const Core::event_id_t EVENT_XMLRPC_LOGIN_FAILED = 0x06;
        static const Core::event_id_t EVENT_INIT_UPD_CONNECTION= 0x07;
        static const Core::event_id_t EVENT_SERVER_CONNECTED = 0x08;
        static const Core::event_id_t EVENT_CONNECTION_FAILED = 0x09;
    }
    /// @}
    
    /// Enumeration of the network connection states.
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
    }
    
    /// Data struct which is passed to the login thread.
    struct ConnectionThreadState
    {
        volatile OpenSimProtocol::Connection::State state;
        OpenSimProtocol::ClientParameters parameters;
        std::string errorMessage;
    };
        
    /// Event data interface for inbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventInboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventInboundData(NetMsgID id, NetInMessage *msg)
        : message(msg), messageID(id) {}
        virtual ~NetworkEventInboundData() {}
        
        NetMsgID messageID;
        NetInMessage *message;
    };

    /// Event data interface for outbound messages.
    /// \ingroup OpenSimProtocolClient
    class NetworkEventOutboundData : public Foundation::EventDataInterface
    {
    public:
        NetworkEventOutboundData(NetMsgID id, const NetOutMessage *msg)
        : message(msg), messageID(id) {}
        virtual ~NetworkEventOutboundData() {}
        
        NetMsgID messageID;
        const NetOutMessage *message;
    };
}

#endif
