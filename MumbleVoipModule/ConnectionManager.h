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
#include "SoundServiceInterface.h"
#include "MumbleDefines.h"

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
    class LibMumbleThread;
    class User;

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
	 *  Handles connections to mumble servers.
     * 
     *  Fetch recorted audio from AudioModule.
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

        //! opens mumble client with auto connect to given server and
        //! starts link plugin
        //!
        //! If mumble client is already running it closes current connections
        //! and open a new connection to given server.
        virtual void OpenConnection(ServerInfo info);

        virtual void CloseConnection(ServerInfo info);

        //! \throw std::exception if audio recording cannot be started
        virtual void SendAudio(bool value);

        //! \return true if sending audio false if not
        virtual bool SendingAudio();

        //! Start mumble client application with given server url
        //! format: mumble://<user>:<password>@<server>/<channel>/<subchannel>?version=<version>
        //!
        //! There is no way to stop mumle client from this module it has to be closed by user.
        static void StartMumbleClient(const QString& server_url);

        //! Kill mumble client process
        static void KillMumbleClient();

        virtual void Update(f64 frametime);

        virtual void SetAudioSourcePosition(double x, double y, double z);

    private:
        void StartMumbleLibrary();
        void StopMumbleLibrary();
        void PlaybackAudioPacket(User* user, PCMAudioFrame* frame);
        boost::shared_ptr<Foundation::SoundServiceInterface> SoundService();

        QMap<QString, Connection*> connections_; // maps: server address - connection object
        LibMumbleThread* lib_mumble_thread_;
        Foundation::Framework* framework_;
//        sound_id_t audio_playback_channel_;
        bool sending_audio_;
        std::string recording_device_;
        double position_x_;
        double position_y_;
        double position_z_;
        QMap<int, sound_id_t> audio_playback_channels_;

        static const int AUDIO_RECORDING_BUFFER_MS = 200;

    public slots:
        void PlaybackAudio(Connection* connection);
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
