// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KristalliProtocolModule_UserConnection_h
#define incl_KristalliProtocolModule_UserConnection_h

#include "Foundation.h"

namespace kNet
{
class MessageConnection;
}

//! Interface class for user's currently replicated scene state
struct ISyncState
{
    virtual ~ISyncState() {}
};

namespace KristalliProtocol
{
    //! Connection on the Kristalli server
    struct UserConnection
    {
        UserConnection() :
            connection(0),
            userID(0),
            authenticated(false)
        {
        }
        
        kNet::MessageConnection* connection;
        u8 userID;
        bool authenticated;
        std::string userName;
        /// Extra properties such as password
        std::map<std::string, std::string> properties;
        boost::shared_ptr<ISyncState> syncState;
    };
}

#endif

