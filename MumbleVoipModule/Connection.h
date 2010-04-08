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