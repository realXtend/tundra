// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_VoiceChannel.h"
#include "IAttribute.h"
#include "Entity.h"
#include "Settings.h"
#include "ServerInfo.h"
#include "Session.h"

EC_VoiceChannel::EC_VoiceChannel(IModule *module): 
    IComponent(module->GetFramework())
{
}

EC_VoiceChannel::~EC_VoiceChannel()
{
    emit ChannelExpired(Name());
}

