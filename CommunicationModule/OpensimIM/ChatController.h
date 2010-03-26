// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OpensimIM_ChatController_h
#define incl_Communication_OpensimIM_ChatController_h

#include "NetworkEvents.h"
#include "CommunicationModuleFwd.h"

#include <QObject>

namespace OpensimIM
{
    class ChatController : public QObject
    {
    
    Q_OBJECT

    public:
        ChatController(ProtocolUtilities::ClientParameters client_parameters);
        virtual ~ChatController();

    private slots:
        bool InitCommunicationConnections();

        void OnOpensimUdpConnectionReady(Communication::ConnectionInterface &os_chat_connection);
        void OnOpensimUdpConnectionError(Communication::ConnectionInterface &os_chat_connection);

        void MessageReceivedFromServer(const Communication::ChatMessageInterface &msg);
        void SendChatMessageToServer(const QString &message);

    signals:
        void MessageReceived(bool self_sent_message, QString sender, QString timestamp, QString message);

    private:
        ProtocolUtilities::ClientParameters client_parameters_;

        Communication::ConnectionInterface *os_chat_connection_;
        Communication::ChatSessionInterface *os_public_chat_;
    
    };
}

#endif