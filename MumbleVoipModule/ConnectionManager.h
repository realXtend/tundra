// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ConnectionManager_h
#define incl_MumbleVoipModule_ConnectionManager_h


#include <QObject>
#include "CoreTypes.h"
#include "ServerInfo.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace MumbleVoip
{

    class ServerInfo;

    /**
	 *  Handles connections to mumble server.
     * 
	 *  Current implementation launches mumble client. 
     *
     *  Future implementation uses integrated mumble client and allow multiple
     *  simultaneous connections to mumble servers.
     */
    class ConnectionManager : public QObject
    {
        Q_OBJECT

    public:
        ConnectionManager();
        virtual ~ConnectionManager() {};

        //! NOT IMPLEMENTED
        //!
        //! opens mumble client with auto connect to given server and
        //! starts link plugin
        //!
        //! If mumble client is already running it closes current connections
        //! and open a new connection to given server.
        virtual void OpenConnection(ServerInfo info);

        //! NOT IMPLEMENTED
        //!
        //! @note Cannot stop mumble client
        //! Stops link plugin
        virtual void CloseConnection(ServerInfo info);

        //! Start mumble client application with given server url
        //! format: mumble://<user>:<password>@<server>/<channel>/<subchannel>?version=<version>
        //!
        //! There is no way to stop mumle client from this module it has to be closed by user.
        static void StartMumbleClient(const QString& server_url);
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
