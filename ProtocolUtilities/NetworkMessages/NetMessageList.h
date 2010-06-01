// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_ProtocolUtilities_NetMessageList_h
#define incl_ProtocolUtilities_NetMessageList_h

#pragma warning( push )
#pragma warning( disable : 4396 )
#include <boost/unordered_map.hpp>
#pragma warning( pop )

#include "NetMessage.h"

namespace ProtocolUtilities
{
    /// A data structure that contains a list of known network messages.
    class NetMessageList
    {
    public:
        /// @param filename The file to load the message list from.
        NetMessageList(const char *filename);
        ~NetMessageList();

        /// @return The message info structure corresponding to the message with the given ID, or 0 if no such
        ///         message is known.
        const NetMessageInfo *GetMessageInfoByID(NetMsgID id) const;

        /// Generates a C++ header file out of all the IDs of the known message definitions.
        void GenerateHeaderFile(const char *filename) const;

    private:
        NetMessageList(const NetMessageList &);
        void operator=(const NetMessageList &);

        typedef boost::unordered_map<NetMsgID, NetMessageInfo> NetworkMessageMap;

        /// Contains all the messages known by this list.
        NetworkMessageMap messages;

        /// Reads in new messages from the given file.
        void ParseMessageListFromFile(const char *filename);
    };
}

#endif
