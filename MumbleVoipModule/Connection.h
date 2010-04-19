// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Connection_h
#define incl_MumbleVoipModule_Connection_h

#include <QObject>
#include <QList>
#include <QMutex>
#include <QMap>
#include "CoreTypes.h"
#include "ServerInfo.h"
#include "stdint.h"

class QNetworkReply;
class QNetworkAccessManager;
namespace MumbleClient
{
    class MumbleClient;
    class Channel;
}
struct CELTMode;
struct CELTEncoder;
struct CELTDecoder;

namespace MumbleVoip
{
    class Channel;
    class PCMAudioFrame;

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

        //! \return first audio frame from playback queue
        //! \return null if playback queue is empty
        virtual PCMAudioFrame* GetAudioFrame();

        //! Encode and send given frame to Mumble server
        //! Frame object is deleted by this method
        virtual void SendAudioFrame(PCMAudioFrame* frame);

        //! \return list of channels available
        virtual QList<QString> Channels();

        virtual void SendAudio(bool send);
        virtual bool SendingAudio();

        // virtual State State();
        // virtual QString Reason();
    private:
        void InitializeCELT();
        void UninitializeCELT();
        CELTDecoder* CreateCELTDecoder();

        MumbleClient::MumbleClient* client_;
        bool authenticated_;
        QString join_request_;
        QList<PCMAudioFrame*> playback_queue_;
        QList<PCMAudioFrame*> encode_queue_;
        QMap<int, CELTDecoder*> celt_decoders_; // maps session <-> decoder
        CELTMode* celt_mode_;
        CELTEncoder* celt_encoder_;
        CELTDecoder* celt_decoder_;
        static const int SAMPLE_RATE_ = 48000; // always 48000 in mumble
        static const int AUDIO_QUALITY_ = 60000; // 32000 - 90000
        static const int ENCODE_BUFFER_SIZE_ = 4000;
        static const int FRAMES_PER_PACKET_ = 6;
        static const int CHANNELS = 1;
        static const int SAMPLES_IN_FRAME = 480;
        static const int SAMPLE_WIDTH = 16;
        static const int SENDING_BUFFER_MS = 500;
        static const int PLAYBACK_BUFFER_MS = 500;

        QList<Channel*> channels_;
        bool sending_audio_;
        unsigned char encode_buffer_[ENCODE_BUFFER_SIZE_];
        int frame_sequence_;
        QMutex mutex_channels_;
        QMutex mutex_authentication_;
        QMutex mutex_playback_queue_;
        QMutex mutex_encode_audio_;

    public slots:
        void OnAuthCallback();
        void OnTextMessageCallback(QString text);
        void OnRawUdpTunnelCallback(int32_t length, void* buffer);
//        void OnRelayTunnel(std::string &s);
        void OnChannelAddCallback(const MumbleClient::Channel& channel);
        void OnChannelRemoveCallback(const MumbleClient::Channel& channel);
        void HandleIncomingCELTFrame(int session, unsigned char* data, int size);

    signals:
//        void Closed();
        void TextMessage(QString &text);
        void AudioDataAvailable(short* data, int size);
        void RelayTunnelData(char*, int);
        void AudioFramesAvailable(Connection* connection);
//        void UserLeft();
//        void UsetJoined();
//        void ChannelAdded(); 
//        void ChannelRemoved();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_Connection_h