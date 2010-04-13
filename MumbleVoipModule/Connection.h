// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_Connection_h
#define incl_MumbleVoipModule_Connection_h

#include <QObject>
#include <QList>
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
        enum State { INITIALIZING, OPEN, CLOSED };
        Q_OBJECT
    public:
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

        //! \return list of channels available
        virtual QList<QString> Channels();
    private:
        void InitializeCELT();
        void UninitializeCELT();

        MumbleClient::MumbleClient* client_;
        bool authenticated_;
        QString join_request_;
        QList<PCMAudioFrame*> playback_queue_;
        QList<PCMAudioFrame*> sending_queue_;
        CELTMode* celt_mode_;
        CELTEncoder* celt_encoder_;
        CELTDecoder* celt_decoder_;
        static const int SAMPLE_RATE_ = 48000; // always 48000 in mumble
        QList<Channel*> channels_;

    public slots:
        void OnAuthCallback();
        void OnTextMessageCallback(QString text);
        void OnRawUdpTunnelCallback(int32_t length, void* buffer);
//        void OnRelayTunnel(std::string &s);
        void OnPlayAudioData(char* data, int size);
        void OnChannelAddCallback(const MumbleClient::Channel& channel);
        void OnChannelRemoveCallback(const MumbleClient::Channel& channel);

        void HandleIncomingCELTFrame(char* data, int size);

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