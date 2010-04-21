// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Connection_h
#define incl_MumbleVoipModule_Connection_h

#include <QObject>
#include <QList>
#include <QMutex>
#include <QMap>
#include <QPair>

#include "CoreTypes.h"
#include "ServerInfo.h"
#include "stdint.h"
#include "MumbleDefines.h"

class QNetworkReply;
class QNetworkAccessManager;
namespace MumbleClient
{
    class MumbleClient;
    class Channel;
    class User;
}
struct CELTMode;
struct CELTEncoder;
struct CELTDecoder;

namespace MumbleVoip
{
    class Channel;
    class User;
    class PCMAudioFrame;

    typedef QPair<User*, PCMAudioFrame*> AudioPacket;

    //class AudioSourceInterface : QObject
    //{
    //    Q_OBJECT
    //public:
    //    QList<PCMAudioFrame*> GetPCMAudioFrames();
    //signals:
    //    void PCMAudioFramesAvailable();
    //};

    //class AudioSinkInterface : QObject
    //{

    //};

    //class VideoSourceInterface : QObject
    //{
    //    Q_OBJECT
    //public:
    //    QList<PCMAudioFrame*> GetVideoFrames();
    //signals:
    //    void VideoFramesAvailable();
    //};

    //! Connection to a single mumble server.
    //!
    //! \todo Thread safety !!!
    //!
    class Connection : public QObject
    {
        Q_OBJECT
    public:
        enum State { STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

        Connection(ServerInfo &info);
        virtual ~Connection();

        //! Closes connection to Mumble server
        virtual void Close();

        //! Joins to given channel if channels exist
        //! \todo create channels if it doesn't exist
        virtual void Join(QString channel);

        //! \return first <user,audio frame> pair from playback queue
        //! \return <0,0> if playback queue is empty
        //! The caller must delete audio frame object after usage
        virtual AudioPacket GetAudioPacket();

        //! Encode and send given frame to Mumble server
        //! Frame object is NOT deleted by this method 
        virtual void SendAudioFrame(PCMAudioFrame* frame, double x, double y, double z);

        //! \return list of channels available
        virtual QList<QString> Channels();

        virtual void SendAudio(bool send);
        virtual bool SendingAudio();

        //! \param quality 0.0 .. 1.0 where 0.0 means lowest bitrate and worst quality
        //!        and 1.0 meas highest bitrate and best quality.
        virtual void SetEncodingQuality(double quality);

        virtual void SendPosition(bool value) { send_position_ = value; }
        virtual bool IsSendingPosition() { return send_position_; }

        // virtual State State();
        // virtual QString Reason();
    private:
        static const int AUDIO_QUALITY_MAX_ = 90000; 
        static const int AUDIO_QUALITY_MIN_ = 32000; 
        static const int ENCODE_BUFFER_SIZE_ = 4000;
        static const int PLAYBACK_BUFFER_MS_ = 100;

        void InitializeCELT();
        void UninitializeCELT();
        CELTDecoder* CreateCELTDecoder();
        int AudioQuality();
        void HandleIncomingCELTFrame(int session, unsigned char* data, int size);
        bool CheckState(QList<State> allowed_states); // testing

        State state_;
        MumbleClient::MumbleClient* client_;
        QString join_request_; // queued request to join a channel
        QList<PCMAudioFrame*> encode_queue_;
        QList<Channel*> channels_;
        QMap<int, User*> users_;

        CELTMode* celt_mode_;
        CELTEncoder* celt_encoder_;
        CELTDecoder* celt_decoder_;

        bool authenticated_;
        bool sending_audio_;
        double encoding_quality_;
        int frame_sequence_;
        bool send_position_;
        unsigned char encode_buffer_[ENCODE_BUFFER_SIZE_];
        
        QMutex mutex_channels_;
        QMutex mutex_authentication_;
        QMutex mutex_encode_audio_;
        QMutex mutex_encoding_quality_;
        QMutex mutex_raw_udp_tunnel_;
        QMutex mutex_users_;
        QMutex mutex_state_;

    public slots:
        void OnAuthCallback();
        void OnTextMessageCallback(QString text);
        void OnRawUdpTunnelCallback(int32_t length, void* buffer);
//        void OnRelayTunnel(std::string &s);
        void OnChannelAddCallback(const MumbleClient::Channel& channel);
        void OnChannelRemoveCallback(const MumbleClient::Channel& channel);
        void OnUserJoinedCallback(const MumbleClient::User& user);
        void OnUserLeftCallback(const MumbleClient::User& user);
        

    signals:
//        void Closed();
        void TextMessage(QString &text);
        void AudioDataAvailable(short* data, int size);
        void RelayTunnelData(char*, int);
        void AudioFramesAvailable(Connection* connection); // do we need this
        void UserLeft(User* user);
        void UserJoined(User* user);
        void ChannelAdded(Channel* channel); 
        void ChannelRemoved(Channel* channel);
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_Connection_h
