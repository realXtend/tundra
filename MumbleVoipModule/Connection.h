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
}
struct CELTMode;
struct CELTEncoder;
struct CELTDecoder;

namespace MumbleVoip
{
    class PCMAudioFrame
    {
    public:
        PCMAudioFrame(int sample_rate, int sample_widh, int channels, char* data, int data_size);
        virtual ~PCMAudioFrame();
        virtual char* Data();
        virtual int Channels();
        virtual int SampleRate();
        virtual int SampleWidth();
        virtual int Samples();
        virtual int GetLengthMs();
        virtual int GetLengthBytes();

    private:

        int channels_;
        int sample_rate_;
        int sample_width_;
        char* data_;
        int data_size_;
    };

    class AudioSourceInterface : QObject
    {
        Q_OBJECT
    public:
        QList<PCMAudioFrame*> GetPCMAudioFrames();
    signals:
        void PCMAudioFramesAvailable();
    };

    class AudioSinkInterface : QObject
    {

    };

    class VideoSourceInterface : QObject
    {
        Q_OBJECT
    public:
        QList<PCMAudioFrame*> GetVideoFrames();
    signals:
        void VideoFramesAvailable();
    };

    //! Connection to a single mumble server.
    //!
    //!
    //!
    class Connection : public QObject
    {
        Q_OBJECT
    public:
        Connection(ServerInfo &info);
        virtual ~Connection();
        virtual void Close();
        virtual void Join(QString channel);
        //! return null if no frames in playback queue
        virtual PCMAudioFrame* GetAudioFrame();
        //virtual QList<QString> ChannelList();
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

    public slots:
        void OnAuthenticated();
        void OnTextMessage(QString text);
        void OnRawUdpTunnel(char* data, int size);
//        void OnRelayTunnel(std::string &s);
        void OnPlayAudioData(char* data, int size);

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