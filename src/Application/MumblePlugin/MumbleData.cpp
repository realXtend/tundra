// For conditions of distribution and use, see copyright notice in LICENSE

#include "MumblePlugin.h"
#include "MumbleData.h"
#include "CoreDefines.h"

// MumbleChannel

MumbleChannel::MumbleChannel(MumblePlugin *owner) :
    owner_(owner)
{
}

MumbleChannel::~MumbleChannel()
{
    foreach(MumbleUser *u, users)
        SAFE_DELETE(u);
    users.clear();
}

MumbleUser *MumbleChannel::User(uint id)
{
    MumbleUser *user = 0;
    foreach(MumbleUser *iter, users)
    {
        if (iter->id == id)
        {
            user = iter;
            break;
        }
    }
    return user;
}

QList<uint> MumbleChannel::MutedUserIds()
{
    QList<uint> mutedIds;
    foreach(MumbleUser *iter, users)
    {
        if (iter->isMuted || iter->isSelfMuted)
            mutedIds << iter->id;
    }
    return mutedIds;
}

bool MumbleChannel::AddUser(MumbleUser *user)
{
    if (!users.contains(user))
    {
        users.push_back(user);
        return true;
    }
    else
        return false;
}

bool MumbleChannel::RemoveUser(uint id)
{
    int index = -1;
    for(int i=0; i<users.count(); i++)
    {
        if (users.at(i)->id == id)
        {
            index = i;
            break;
        }
    }
    if (index != -1)
    {
        users.removeAt(index);
        return true;
    }
    else 
        return false;
}

QString MumbleChannel::toString() const
{
    return QString("MumbleChannel(id = %1 name = \"%2\" fullname = \"%3\" usercount = %5)").arg(id).arg(name).arg(fullName).arg(users.count());
}

// MumbleUser

MumbleUser::MumbleUser(MumblePlugin *owner) :
    owner_(owner),
    pos(float3::zero),
    isPositional(false),
    isMuted(false)
{
}

MumbleUser::~MumbleUser()
{
}

QString MumbleUser::toString() const
{
    return QString("MumbleUser(id = %1 name = \"%2\" muted = %3 deaf = %4 self = %5)").arg(id).arg(name).arg(isSelfMuted).arg(isSelfDeaf).arg(isMe);
}

void MumbleUser::SetMuted(bool muted)
{
    if (owner_)
        owner_->SetMuted(id, muted);
}

void MumbleUser::Mute()
{
    SetMuted(true);
}

void MumbleUser::UnMute()
{
    SetMuted(false);
}

MumbleChannel *MumbleUser::Channel()
{
    if (owner_)
        return owner_->Channel(channelId);
    else
        return 0;
}
