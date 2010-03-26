
#include "StableHeaders.h"
#include "XmlRpcEpi.h"

#include "OpenSim/BuddyList.h"
#include "OpenSim/BuddyListParser.h"

namespace ProtocolUtilities
{
    // STATIC
    ProtocolUtilities::BuddyListPtr BuddyListParser::ExtractBuddyListFromXMLRPCReply(XmlRpcEpi &call)
    {
        XmlRpcCall *xmlrpcCall = call.GetXMLRPCCall();
        if (!xmlrpcCall)
            throw XmlRpcException("Failed to read buddy list, no XMLRPC Reply to read!");

        XMLRPC_REQUEST request = xmlrpcCall->GetReply();
        if (!request)
            throw XmlRpcException("Failed to read buddy list, no XMLRPC Reply to read!");

        XMLRPC_VALUE result = XMLRPC_RequestGetData(request);
        if (!result)
            throw XmlRpcException("Failed to read buddy list, the XMLRPC Reply did not contain any data!");

        XMLRPC_VALUE buddy_list_node = XMLRPC_VectorGetValueWithID(result, "buddy-list");

        if (!buddy_list_node || XMLRPC_GetValueType(buddy_list_node) != xmlrpc_vector)
            throw XmlRpcException("Failed to read buddy list, buddy-list in the reply was not properly formed!");

        ProtocolUtilities::BuddyListPtr buddy_list = ProtocolUtilities::BuddyListPtr(new ProtocolUtilities::BuddyList());

        XMLRPC_VALUE item = XMLRPC_VectorRewind(buddy_list_node);

        while(item)
        {
            XMLRPC_VALUE_TYPE type = XMLRPC_GetValueType(item);
            if (type == xmlrpc_vector) // xmlrpc-epi handles structs as arrays.
            {
                RexUUID id;
                int rights_given = 0;
                int rights_has = 0;

                XMLRPC_VALUE val = XMLRPC_VectorGetValueWithID(item, "buddy_id");
                if (val && XMLRPC_GetValueType(val) == xmlrpc_string)
                    id.FromString( XMLRPC_GetValueString(val) );

                val = XMLRPC_VectorGetValueWithID(item, "buddy_rights_given");
                if (val && XMLRPC_GetValueType(val) == xmlrpc_type_int)
                    rights_given = XMLRPC_GetValueInt(val);

                val = XMLRPC_VectorGetValueWithID(item, "buddy_rights_has");
                if (val && XMLRPC_GetValueType(val) == xmlrpc_type_int)
                    rights_has = XMLRPC_GetValueInt(val);

                ProtocolUtilities::Buddy *buddy = new ProtocolUtilities::Buddy(id, rights_given, rights_has);
                buddy_list->AddBuddy(buddy);
            }

            item = XMLRPC_VectorNext(buddy_list_node);
        }

        return buddy_list;
    }

}