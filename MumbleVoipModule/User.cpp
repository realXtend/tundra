#include "StableHeaders.h"
#include "User.h"
#include "MumbleVoipModule.h"
#include "stdint.h"

//#define BUILDING_DLL // for dll import/export declarations
//#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/user.h>
//#undef BUILDING_DLL // for dll import/export declarations

namespace MumbleVoip
{
    User::User(const MumbleClient::User& user) : user_(user)
    {
    }

    QString User::Name()
    {
        return QString(user_.name.c_str());
    }
    
    QString User::Hash()
    {
        return QString(user_.hash.c_str());
    }
    
    int User::Session()
    {
        return user_.session;
    }
    
    int User::Id()
    {
        return user_.user_id;
    }
    
    Channel* User::Channel()
    {
        return 0; // \todo: implement
    }
    
    QString User::Comment()
    {
        return QString(user_.comment.c_str());
    }

} // namespace MumbleVoip
