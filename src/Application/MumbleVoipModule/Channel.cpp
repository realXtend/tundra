// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"

#include "Channel.h"
#include "MumbleVoipModule.h"

#include "LibMumbleClient.h"

#include "MemoryLeakCheck.h"

namespace MumbleLib
{
    Channel::Channel(const ::MumbleClient::Channel* channel) : channel_(channel)
    {
        LogDebug(QString("Mumble channel object created for: %1").arg(channel->name.c_str()));
        channel_name_ = channel_->name.c_str();
    }

    Channel::~Channel()
    {
        // @note channel_ pointer is not safe to use because it might have been uninitialized
        // by mumble client library at this point
        LogDebug(QString("Mumble channel object deleted for: %1").arg(channel_name_));

        channel_ = 0;
    }

    QString Channel::Name() const
    {
        if (!channel_)
            return "";

        return QString(channel_->name.c_str());
    }
    
    QString Channel::FullName() const
    {
        if (!channel_)
            return "";

        QString full_name = Name();
        boost::shared_ptr<::MumbleClient::Channel> c = channel_->parent.lock();
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
        if (!channel_)
            return -1;

        return channel_->id;
    }

    QString Channel::Description() const
    {
        if (!channel_)
            return "";

        return QString(channel_->description.c_str());
    }

} // namespace MumbleLib
