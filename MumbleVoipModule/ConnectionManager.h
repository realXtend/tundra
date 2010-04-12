// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_ConnectionManager_h
#define incl_MumbleVoipModule_ConnectionManager_h


#include <QObject>
#include <QString>
#include <QThread>
#include <QMap>
#include "CoreTypes.h"
#include "ServerInfo.h"
#include "Framework.h"

class QNetworkReply;
class QNetworkAccessManager;

namespace MumbleClient
{
    class MumbleClientLib;
}

namespace MumbleVoip
{
    class ServerInfo;
    class Connection;
    class PCMAudioFrame;

    /**
     * Thread for running the lib mumble mainloop
     *
     */
    class LibMumbleMainloopThread : public QThread 
    {
    public:
        virtual void run();
    };

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
        ConnectionManager(Foundation::Framework* framework);
        virtual ~ConnectionManager();

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

        //! Kill mumble client process
        static void KillMumbleClient();
    private:
        void StartMumbleLibrary();
        void StopMumbleLibrary();
        void PlaybackAudioFrame(PCMAudioFrame* frame);

        QMap<QString, Connection*> connections_; // maps: server address - connection object
        MumbleClient::MumbleClientLib* mumble_lib; // @todo: Do we need this pointer?
        LibMumbleMainloopThread lib_thread_;
        Foundation::Framework* framework_;
        sound_id_t audio_playback_channel_;

    public slots:
        void OnAudioFramesAvailable(Connection* connection);
        void OnAudioDataFromConnection(short* data, int size);
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
