// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ChatController.h"

#include "CommunicationService.h"
#include "Credentials.h"
#include "ConnectionInterface.h"
#include "ChatSessionInterface.h"
#include "ChatMessageInterface.h"
#include "ChatSessionParticipantInterface.h"
#include "CoreException.h"

#include <QDebug>

namespace OpensimIM
{
    ChatController::ChatController(ProtocolUtilities::ClientParameters client_parameters) :
        QObject(),
        client_parameters_(client_parameters)
    {
        InitCommunicationConnections();
    }

    ChatController::~ChatController()
    {
        // Communication::CommunicationService singleton will clean up created connections etc on exit
        // If its allowed to remove them during run time, please let me know - Jonne Nauha aka Pforce
    }

    // Private

    bool ChatController::InitCommunicationConnections()
    {
        // Get singleton of the communication service
        Communication::CommunicationServiceInterface *communication_service = 
            Communication::CommunicationService::GetInstance();

        if (!communication_service)
            return false;

        // Credentials with local avatar UUID
        Communication::Credentials os_chat_credentials;
        os_chat_credentials.SetProtocol("opensim_udp");
        os_chat_credentials.SetUserID(QString(client_parameters_.agentID.ToString().c_str()));

        // Open up a opensim chat connection
        os_chat_connection_ = communication_service->OpenConnection(os_chat_credentials);
        
        // Check state of readyness, proceed accordingly
        switch (os_chat_connection_->GetState())
        {
            case Communication::ConnectionInterface::STATE_INITIALIZING:
                connect(os_chat_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), 
                        SLOT( OnOpensimUdpConnectionReady(Communication::ConnectionInterface&) ));
                connect(os_chat_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), 
                        SLOT( OnOpensimUdpConnectionError(Communication::ConnectionInterface&) ));
                break;

            case Communication::ConnectionInterface::STATE_OPEN:
                OnOpensimUdpConnectionReady(*os_chat_connection_);
                break;

            case Communication::ConnectionInterface::STATE_ERROR:
                qDebug() << "OpenSimIM::ChatController >> chat connection is in a error state, cannot continue";
                return false;

            default:
                return false;
        }
        return true;
    }

    void ChatController::OnOpensimUdpConnectionReady(Communication::ConnectionInterface &os_chat_connection)
    {
        try
        {
            os_public_chat_ = os_chat_connection.OpenChatSession("0"); // public chat channel 0
            connect(os_public_chat_, SIGNAL( MessageReceived(const Communication::ChatMessageInterface&) ),
                    SLOT( MessageReceivedFromServer(const Communication::ChatMessageInterface& ) ));
        }
        catch (Exception &e)
        {
            qDebug() << QString("OpenSimIM::ChatController >> Could not open world chat due to: ").append(e.what());
        }
    }

    void ChatController::OnOpensimUdpConnectionError(Communication::ConnectionInterface &os_chat_connection)
    {
        qDebug() << "OpenSimIM::ChatController >> OpenSim udp connect went to a error state, "
                    "cannot proceed with world chat";
    }

    void ChatController::MessageReceivedFromServer(const Communication::ChatMessageInterface &msg)
    {
        if (!os_chat_connection_ || !os_public_chat_)
            return;

        QString sender, timestamp, message;
        bool self_sent_message = false;

        if (os_chat_connection_->GetUserID() == msg.GetOriginator()->GetID())
            self_sent_message = true;
        timestamp = QString("%1 %2").arg(msg.GetTimeStamp().date().toString("dd.MM.yyyy"),msg.GetTimeStamp().time().toString("hh:mm:ss"));
        sender = msg.GetOriginator()->GetName();
        message = msg.GetText();

        emit MessageReceived(self_sent_message, sender, timestamp, message);
    }

    void ChatController::SendChatMessageToServer(const QString &message)
    {
        if (!os_chat_connection_ || !os_public_chat_)
            return;

        os_public_chat_->SendChatMessage(message);
    }
}