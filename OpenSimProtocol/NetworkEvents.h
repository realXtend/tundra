// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NetworkEvents_h
#define incl_NetworkEvents_h

#include "EventDataInterface.h"
#include "RexUUID.h"
#include "NetInMessage.h"
#include "NetOutMessage.h"

namespace OpenSimProtocol
{
    struct ClientParameters
    {
        RexUUID agentID;
        RexTypes::RexUUID sessionID;
        RexTypes::RexUUID regionID;
        uint32_t circuitCode;
        std::string sessionHash;
        std::string gridUrl;
        std::string avatarStorageUrl;
    };
    
    namespace Events
    {
        /// These events belong to the "NetworkState" event category.
        static const Core::event_id_t EVENT_SERVER_DISCONNECTED = 0x01;
        static const Core::event_id_t EVENT_SERVER_CONNECTED = 0x02;
        static const Core::event_id_t EVENT_SERVER_CONNECTION_FAILED = 0x03;
    }
    
    /// Enumeration of the network connection states.
    namespace Connection
    {    
        enum State
        {
            STATE_LOGIN_FAILED = 0,
            STATE_DISCONNECTED,
            STATE_INIT_XMLRPC,
            STATE_WAITING_FOR_XMLRPC_REPLY,
            STATE_XMLRPC_REPLY_RECEIVED,
            STATE_INIT_UDP,
            STATE_CONNECTED
        };
    }
    
    /// Data struct which is passed to the login thread.
    struct ConnectionThreadState
    {
        volatile OpenSimProtocol::Connection::State state;
        OpenSimProtocol::ClientParameters parameters;
    };
        
    /// Event data interface for inbound messages.
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
