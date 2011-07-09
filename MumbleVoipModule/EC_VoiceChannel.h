// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IComponent.h"
#include "Declare_EC.h"
#include "IAttribute.h"

/// Mumble based voice channel
/**
  Defines a mumble channel to join.
*/
class EC_VoiceChannel : public IComponent
{
    DECLARE_EC(EC_VoiceChannel)
    Q_OBJECT
public:
    virtual ~EC_VoiceChannel();

    /// VOIP protocol used eg. 'mumble'
    Q_PROPERTY(QString protocol READ getprotocol WRITE setprotocol);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, protocol);

    /// Mumbe server version eg. '1.2.2'
    Q_PROPERTY(QString version READ getversion WRITE setversion);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, version);

    /// Mumbe server address eg. 'http://myserver:8000'
    Q_PROPERTY(QString serveraddress READ getserveraddress WRITE setserveraddress);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, serveraddress);

    /// Mumbe server password eg. 'mypassword'
    Q_PROPERTY(QString serverpassword READ getserverpassword WRITE setserverpassword);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, serverpassword);

    /// Mumbe server username eg. 'John'
    /// @note We don't need this, the user name should be allways the client specific
    Q_PROPERTY(QString username READ getusername WRITE setusername);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, username);

    /// Human readable name for the channel eg. 'My channel'
    Q_PROPERTY(QString channelname READ getchannelname WRITE setchannelname);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, channelname);

    /// Mumble channel id eg. 'voip/regions/chat/channel24442'
    Q_PROPERTY(QString channelid READ getchannelid WRITE setchannelid);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, channelid);

    /// Mumble channel id eg. 'voip/regions/chat/channel24442'
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

private:
    /// Constuctor.
    /** @param module Declaring module.
    */
    explicit EC_VoiceChannel(IModule *module);
private slots:
    void AutoDisabelChannel(IAttribute*, AttributeChange::Type);
};

// incl_MumbleVoip_EC_VoiceChannel_h
