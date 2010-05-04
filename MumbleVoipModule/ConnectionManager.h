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
    class VoiceIndicatorInterface;

    /**
	 *  Handles connections to mumble servers.
     * 
     *  Sends audio data to mumble servers recorded by audioModule.
     *
     *  Future implementation uses integrated mumble client and allow multiple
     *  simultaneous connections to mumble servers.
     */
    class ConnectionManager : public QObject
    {
        Q_OBJECT

    public:
        //! Default constructor
        ConnectionManager(Foundation::Framework* framework);

        virtual ~ConnectionManager();

        //! Open connection to given Mumble server
        virtual void OpenConnection(ServerInfo info);

        //! Closes connection to given Mumble server
        //! If such a connection cannot be found then do nothing
        virtual void CloseConnection(ServerInfo info);

        //! Set audio sending on/off
        //! \throw std::exception if audio recording cannot be started
        virtual void SendAudio(bool send);

        //! \return true if audio is transmitted to mumble server
        virtual bool SendingAudio();

        virtual void ReceiveAudio(bool receive);


        //virtual void ReceiveAudio(bool receive);

        //virtual bool IsAudioSendingEnabled() const;

        //virtual void EnableAudioSending();

        //virtual void DisableAudioSending();

        //! Start mumble client application with given server url
        //! format: mumble://<user>:<password>@<server>/<channel>/<subchannel>?version=<version>
        //!
        //! There is no way to stop mumle client from this module it has to be closed by user.
        static void StartMumbleClient(const QString& server_url);

        //! Kill mumble client process
        static void KillMumbleClient();

        virtual void Update(f64 frametime);

        //! Set user's own position. This value is sent to Mumble server with audio packets
        virtual void SetAudioSourcePosition(Vector3df position);

        //! /param enable If true then voice activity detector is enabled. If false then voice activity detector is disabled
        virtual void EnableVAD(bool enable);

    private:
        void StartMumbleLibrary();
        void StopMumbleLibrary();
        void PlaybackAudioPacket(User* user, PCMAudioFrame* frame);
        boost::shared_ptr<Foundation::SoundServiceInterface> SoundService();

        QMap<QString, Connection*> connections_; // maps: server address - connection object
        LibMumbleThread* lib_mumble_thread_;
        Foundation::Framework* framework_;
        bool sending_audio_;
        bool receiving_audio_;
        std::string recording_device_;
        Vector3df users_position_;
        QMap<int, sound_id_t> audio_playback_channels_;
        VoiceIndicatorInterface* voice_indicator_;

        static const int AUDIO_RECORDING_BUFFER_MS = 200;

    public slots:
        void PlaybackAudio(Connection* connection);

    private slots:
        void OnUserJoined(User* user);
        void OnUserLeft(User* user);

    signals:
        void UserJoined(User* user);
        void UserLeft(User* user);
        void AudioFrameSent(PCMAudioFrame* frame);
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_ConnectionManager_h
