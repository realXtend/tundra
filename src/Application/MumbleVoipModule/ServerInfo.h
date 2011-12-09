// For conditions of distribution and use, see copyright notice in LICENSE

#ifndef incl_MumbleVoipModule_ServerInfo_h
#define incl_MumbleVoipModule_ServerInfo_h

namespace MumbleVoip
{
    /**
     *  Information for connecting to a mumble server and using link plugin.
     *  @todo rename to ChannelInfo
     */
    class ServerInfo
    {
    public:
        ServerInfo() :
            server(""), port(""), channel_id(""), password(""), user_name(""),
            avatar_id(""), context_id(""), channel_name(""), version("")
        {
        }

        QString server;
        QString port;
        QString channel_id;
        QString user_name;  
        QString password;
        QString version;

        QString avatar_id;
        QString context_id;

        QString channel_name;

        int GetPortInteger()
        {
            if (port.isEmpty())
                return -1;

            bool ok = false;
            int iPort = port.toInt(&ok);
            if (!ok)
                iPort = -1;
            return iPort;
        }
    };

}

#endif
