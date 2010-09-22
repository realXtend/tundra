// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IEventData.h"

class MessageConnection;
typedef unsigned long message_id_t;

namespace KristalliProtocol
{
    struct UserConnection;

namespace Events
{
    // This event is posted whenever a new inbound network message is received from a Kristalli peer. KristalliNetMessageIn structure.
    static const event_id_t NETMESSAGE_IN = 1;

    // This event is posted for a new user connection to a server. KristalliUserConnected structure.
    static const event_id_t USER_CONNECTED = 2;
    
    // This event is posted for a new user connection to a server. KristalliUserDisconnected structure.
    static const event_id_t USER_DISCONNECTED = 3;
    
    // This event is posted when a connection attempt has failed so many times that we stop trying. No eventdata.
    static const event_id_t CONNECTION_FAILED = 4;
    
    // The message received from a Kristalli server is wrapped in this Naali event structure.
    class KristalliNetMessageIn : public IEventData
    {
    public:
        KristalliNetMessageIn(MessageConnection *source_, message_id_t id_, const char *data_, size_t numBytes_)
        :source(source_), id(id_), data(data_), numBytes(numBytes_)
        {
        }

        MessageConnection *source;
        message_id_t id;
        const char *data;
        size_t numBytes;
    };
    
    // Event structure for user connected
    class KristalliUserConnected : public IEventData
    {
    public:
        KristalliUserConnected(UserConnection *connection_) :
            connection(connection_)
        {
        }
        
        UserConnection *connection;
    };
    
    // Event structure for user connected
    class KristalliUserDisconnected : public IEventData
    {
    public:
        KristalliUserDisconnected(UserConnection *connection_) :
            connection(connection_)
        {
        }
        
        UserConnection *connection;
    };
} // ~Events namespace

} // ~KristalliProtocolModule namespace
