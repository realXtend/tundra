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

namespace Events
{
    // This event is posted whenever a new inbound network message is received from a Kristalli server.
    static const event_id_t NETMESSAGE_IN = 1;

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

    // This event is posted when user's master status on Kristalli server changes
    static const event_id_t MASTER_USER = 2;
    
    class KristalliMasterUser : public Foundation::EventDataInterface
    {
    public:
        KristalliMasterUser(bool isMaster) :
           userIsMaster(isMaster)
        {
        }
       
        bool userIsMaster;
    };
    
    // This event is used to verify validity of assetstore assets. An asset might exist in user's cache,
    // but we consider it valid only if it also is known by the server
    static const event_id_t VALIDATE_ASSET = 3;
    class KristalliValidateAsset : public Foundation::EventDataInterface
    {
    public:
        KristalliValidateAsset(const std::string& name) :
            assetname(name),
            valid(false)
        {
        }
        
        std::string assetname;
        bool valid;
    };
    
} // ~Events namespace

} // ~KristalliProtocolModule namespace
