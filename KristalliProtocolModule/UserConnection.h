// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_KristalliProtocolModule_UserConnection_h
#define incl_KristalliProtocolModule_UserConnection_h

#include "Foundation.h"

class MessageConnection;

//! Class for holding complex connection-related info, such as currently replicated scene state
struct IUserData
{
    virtual ~IUserData() {}
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
        
        MessageConnection* connection;
        u8 userID;
        bool authenticated;
        std::string userName;
        boost::shared_ptr<IUserData> userData;
    };
}

#endif

