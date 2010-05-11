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
    //! Mumble client application startup
    //!
    class ApplicationManager
    {
        //! Default constructor
        ApplicationManager();
    public:

        //! Start mumble client application with given server url
        //! format: mumble://<user>:<password>@<server>/<channel>/<subchannel>?version=<version>
        static void StartMumbleClient(const QString& server_url);

        //! @return number of StartMumbleClient calls made
        static int StartCount();
    private:
        static int start_count_;
    };

}// namespace MumbleVoip

#endif // incl_MumbleVoipModule_ApplicationManager_h
