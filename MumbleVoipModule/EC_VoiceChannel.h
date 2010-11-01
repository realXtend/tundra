// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoip_EC_VoiceChannel_h
#define incl_MumbleVoip_EC_VoiceChannel_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "IAttribute.h"

/// Mumble based voice channel
/**
  Defines a mumble channel to join.
  @todo 'enabled' attribute
*/
class EC_VoiceChannel : public IComponent
{
    DECLARE_EC(EC_VoiceChannel)
    Q_OBJECT
public:
    virtual ~EC_VoiceChannel();

    /// IComponent override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

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
    Q_PROPERTY(QString username READ getusername WRITE setusername);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, username);

    /// Human readable name for the channel eg. 'My channel'
    Q_PROPERTY(QString channelname READ getchannelname WRITE setchannelname);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, channelname);

    /// Mumble channel id eg. 'voip/regions/chat/channel24442'
    Q_PROPERTY(QString channelid READ getchannelid WRITE setchannelid);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, channelid);

private:
    /// Constuctor.
    /** @param module Declaring module.
    */
    explicit EC_VoiceChannel(IModule *module);
};

#endif // incl_MumbleVoip_EC_VoiceChannel_h
