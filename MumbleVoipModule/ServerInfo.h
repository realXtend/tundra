// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ServerInfo_h
#define incl_MumbleVoipModule_ServerInfo_h

//class EC_VoiceChannel;

namespace MumbleVoip
{
    /**
     *  Information for connecting to a mumble server and using link plugin.
     *  @todo rename to ChannelInfo
     */
    class ServerInfo
    {
    public:
        //ServerInfo();
        //ServerInfo(EC_VoiceChannel* channel);

        //! For connecting to server
        QString server;
        QString channel_id;
        QString user_name; // Value from server/EC is not user since we use avatar's name 
        QString password;
        QString version;

        //! For positional audio
        QString avatar_id; // entity uuid presenting the avatar
        QString context_id;

        //! For UI
        QString channel_name;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
