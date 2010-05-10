// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ApplicationManager_h
#define incl_MumbleVoipModule_ApplicationManager_h

//#include <mumbleclient/channel.h>

namespace MumbleClient
{
    class Channel;
}

namespace MumbleVoip
{
    //! Channel on Mumble server
    //!
    //! @todo Add signals: UserJoined, UserLeft ???
    //! @todo Add Users() method ???
    class ApplicationManager
    {
    public:
        //! Default constructor
        //! @param channel mumbleclient library Channel object 
        ApplicationManager();

        //! Start mumble client application with given server url
        //! format: mumble://<user>:<password>@<server>/<channel>/<subchannel>?version=<version>
        static void StartMumbleClient(const QString& server_url);

        //! Kill mumble client process
        static void KillMumbleClient();

    private:
    };

}// namespace MumbleVoip

#endif // incl_MumbleVoipModule_ApplicationManager_h
