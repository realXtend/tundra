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

namespace MumbleVoip
{
    class PCMAudioFrame
    {
    public:
        PCMAudioFrame(int channels, int sample_rate, int sample_widh, char* data, int data_size);
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
        int sample_widh_;
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
        //virtual QList<QString> ChannelList();
    private:
        MumbleClient::MumbleClient* client_;
        bool authenticated_;
        QString join_request_;
        QList<PCMAudioFrame> pcm_data_in_;
        QList<PCMAudioFrame> pcm_data_out_;

    public slots:
        void OnAuthenticated();
        void OnTextMessage(QString text);
        void OnRelayTunnel(std::string &s);
        void OnPlayAudioData(char* data, int size);

    signals:
//        void Closed();
        void TextMessage(QString &text);
        void RelayTunnelData(char*, int);
//        void UserLeft();
//        void UsetJoined();
//        void ChannelAdded(); 
//        void ChannelRemoved();
    };

} // namespace MumbleVoip

#endif // incl_MumbleVoipModule_Connection_h