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
    channelid(this, "channelid", ""),
    enabled(this, "enabled", false)
{
    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(AutoDisabelChannel(IAttribute*, AttributeChange::Type)));
}

EC_VoiceChannel::~EC_VoiceChannel()
{
}

void EC_VoiceChannel::AutoDisabelChannel(IAttribute* attribute, AttributeChange::Type type)
{
    /// Would be nice but cannot be done easily now since we get changed signals from attributes that have not changed
    //if (attribute->GetNameString() != "enabled" && getenabled() == true)    
    //    setenabled(false);
}

