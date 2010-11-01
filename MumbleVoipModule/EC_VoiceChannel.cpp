// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_VoiceChannel.h"
#include "MemoryLeakCheck.h"

EC_VoiceChannel::EC_VoiceChannel(IModule *module): 
    IComponent(module->GetFramework()) ,
    protocol(this, "protocol", ""),
    version(this, "version", ""),
    serveraddress(this, "serveraddress", ""),
    serverpassword(this, "serverpassword", ""),
    username(this, "username", ""),
    channelname(this, "channelname", ""),
    channelid(this, "channelid", "")
{
}

EC_VoiceChannel::~EC_VoiceChannel()
{
}
