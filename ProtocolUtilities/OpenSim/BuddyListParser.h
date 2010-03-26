// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file BuddyListParser.h
 *  @brief Parses buddy list from xmlrpc call
 */

#ifndef incl_Protocol_BuddyListParser_h
#define incl_Protocol_BuddyListParser_h

#include "OpenSim/BuddyList.h"

namespace ProtocolUtilities
{
    class BuddyListParser
    {

    public:
        /**
         *  Extracts buddylist from login reply xml data.
         *  \return BuddlyList object 
         *
         *  XML structure:
         *  "buddy-list"
         *    array:
         *      struct:
         *        string: "buddy_id" 
         *        i4: "buddy_rights_given"
         *        i4: "buddy_rights_has"
         */
        static ProtocolUtilities::BuddyListPtr ExtractBuddyListFromXMLRPCReply(XmlRpcEpi &call);

    };

}

#endif // incl_Protocol_BuddyListParser_h