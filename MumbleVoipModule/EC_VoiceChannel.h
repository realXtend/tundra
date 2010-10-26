// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoip_EC_VoiceAgent_h
#define incl_MumbleVoip_EC_VoiceAgent_h

#include "IComponent.h"
#include "Declare_EC.h"
#include "IAttribute.h"
//#include "ServerInfo.h"
struct MumbleServerInfo
{
    QString address;
    QString version;
    QString user_name;
    QString password;
    QString channel;
};

/// Mumble based voice channel
/**
  Offer basic information for clients to join a mumble voice channel
  (?) Creates
  @todo Define properties
*/
class EC_VoiceChannel : public IComponent
{
    DECLARE_EC(EC_VoiceChannel)
    Q_OBJECT
        
public:
    virtual ~EC_VoiceChannel();

    /// IComponent override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    virtual MumbleServerInfo GetServerInfo() { return server_info_; }
    virtual void SetServerInfo(const MumbleServerInfo& server_info) { server_info_ = server_info; };

signals:
    void ChannelExpired(QString name);

private:
    /// Constuctor.
    /** @param module Declaring module.
    */
    explicit EC_VoiceChannel(IModule *module);

private:
    MumbleServerInfo server_info_;
};

#endif // incl_MumbleVoip_EC_VoiceAgent_h
