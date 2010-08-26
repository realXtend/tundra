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

namespace MumbleLib
{
    Channel::Channel(const MumbleClient::Channel* channel) : channel_(channel)
    {
        QString message = QString("Mumble channel object created for: %1").arg(channel->name.c_str());
        MumbleVoip::MumbleVoipModule::LogDebug(message.toStdString());
        channel_name_ = channel_->name.c_str();
    }

    Channel::~Channel()
    {
        // @note channel_ pointer is not safe to use because it might have been uninitialized
        // by mumble client library at this point
        QString message = QString("Mumble channel object deleted for: %1").arg(channel_name_);
        MumbleVoip::MumbleVoipModule::LogDebug(message.toStdString());
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

} // namespace MumbleLib
