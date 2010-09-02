/** @file
	@author LudoCraft Oy

	Copyright 2009 LudoCraft Oy.
	All rights reserved.

	@brief
*/
#pragma once

#include "EventDataInterface.h"

class MessageConnection;
typedef unsigned long message_id_t;

namespace KristalliProtocol
{
    struct UserConnection;

namespace Events
{
    // This event is posted whenever a new inbound network message is received from a Kristalli peer.
    static const event_id_t NETMESSAGE_IN = 1;

    // This event is posted for a new user connection to a server
    static const event_id_t USER_CONNECTED = 2;
    
    // This event is posted for a new user connection to a server
    static const event_id_t USER_DISCONNECTED = 3;
    
    // The message received from a Kristalli server is wrapped in this Naali event structure.
    class KristalliNetMessageIn : public Foundation::EventDataInterface
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
    class KristalliUserConnected : public Foundation::EventDataInterface
    {
    public:
        KristalliUserConnected(UserConnection *connection_) :
            connection(connection_)
        {
        }
        
        UserConnection *connection;
    };
    
    // Event structure for user connected
    class KristalliUserDisconnected : public Foundation::EventDataInterface
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
