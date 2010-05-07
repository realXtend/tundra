// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Channel.h"
#include "MumbleVoipModule.h"
#include "stdint.h"

#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/channel.h>
#undef BUILDING_DLL // for dll import/export declarations

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    Channel::Channel(const MumbleClient::Channel& c) : channel_(c)
    {
    }

    QString Channel::Name()
    {
        return QString(channel_.name.c_str());
    }

    int Channel::Id()
    {
        return channel_.id;
    }

    QString Channel::Description()
    {
        return QString(channel_.description.c_str());
    }

} // namespace MumbleVoip
