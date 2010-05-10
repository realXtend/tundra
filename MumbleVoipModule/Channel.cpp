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
    Channel::Channel(const MumbleClient::Channel* channel) : channel_(channel)
//    Channel::Channel(const MumbleClient::Channel& c) : channel_(c)
    {
    }

    QString Channel::Name() const
    {
        return QString(channel_->name.c_str());
    }
    
    QString Channel::FullName() const
    {
        QString full_name = Name();
        boost::shared_ptr<MumbleClient::Channel> c = channel_->parent.lock();
        while (c)
        {
            full_name.push_front("/");
            full_name.push_front(c->name.c_str());
            c = c->parent.lock();
        }
        return full_name;
    }

    int Channel::Id() const
    {
        return channel_->id;
    }

    QString Channel::Description() const
    {
        return QString(channel_->description.c_str());
    }

} // namespace MumbleVoip
