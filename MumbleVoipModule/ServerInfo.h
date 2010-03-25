// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ServerInfo_h
#define incl_MumbleVoipModule_ServerInfo_h

namespace MumbleVoip
{
    /**
     *  Information for connecting to a mumble server and using link plugin.
     */
    class ServerInfo
    {
    public:
        //! For connecting to server
        QString server;
        QString channel;
        QString user_name;
        QString password;
        QString version;

        //! For positional audio
        QString avatar_id;
        QString context_id;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
