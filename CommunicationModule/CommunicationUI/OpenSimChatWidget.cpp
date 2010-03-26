// For conditions of distribution and use, see copyright notice in license.txt

#include <StableHeaders.h>
#include "DebugOperatorNew.h"
#include "CommunicationModule.h"
#include "CommunicationService.h"
#include "Credentials.h"
#include "ConnectionInterface.h"
#include "ChatSessionInterface.h"
#include "ChatMessageInterface.h"
#include "ChatSessionParticipantInterface.h"
#include "CoreException.h"

#include "OpenSimChatWidget.h"

#include "MemoryLeakCheck.h"

namespace CommunicationUI
{
    OpenSimChatWidget::OpenSimChatWidget(ProtocolUtilities::ClientParameters clientParams)
        : QWidget(),
          clientParams_(clientParams)
    {
        try
        {
            opensim_chat_ui_.setupUi(this);
            InitCommunicationConnections();
            Communication::CommunicationModule::LogInfo("OpenSim WorldChat >> Startup successful");
        }
        catch (Exception e)
        {
            QString msg( QString("Initialisations threw an exteption: %1").arg(e.what()));
            Communication::CommunicationModule::LogInfo(msg.toStdString());
        }
    }

    OpenSimChatWidget::~OpenSimChatWidget()
    {

    }

    void OpenSimChatWidget::InitCommunicationConnections()
    {
        communicationService_ = Communication::CommunicationService::GetInstance();
        if (communicationService_ != 0)
        {
            Communication::Credentials opensimCredentials;
            opensimCredentials.SetProtocol("opensim_udp");
            opensimCredentials.SetUserID(QString(clientParams_.agentID.ToString().c_str()));
            opensimConnection_ = communicationService_->OpenConnection(opensimCredentials);
            
            switch ( opensimConnection_->GetState() )
            {
                case Communication::ConnectionInterface::STATE_INITIALIZING:
                    QObject::connect(opensimConnection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionReady(Communication::ConnectionInterface&) ));
                    QObject::connect(opensimConnection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionError(Communication::ConnectionInterface&) ));
                    break;
                case Communication::ConnectionInterface::STATE_OPEN:
                    OnOpensimUdpConnectionReady(*opensimConnection_);
                    break;
                case Communication::ConnectionInterface::STATE_ERROR:
                    throw Exception("OpenSim chat connection is in a error state, canoot continue");
                    break;
            }
        }
        else
            throw Exception("Cannot get CommunicationService object");
    }

    void OpenSimChatWidget::OnOpensimUdpConnectionReady(Communication::ConnectionInterface& connection)
    {
        try
        {
            publicChat_ = connection.OpenChatSession("0"); // public chat channel 0
            ConnectSlotsToChatSession();
        }
        catch (Exception &e)
        {
            QString message = QString("Could not open world chat due to: ").append(e.what());
            Communication::CommunicationModule::LogError(message.toStdString());
        }
    }

    void OpenSimChatWidget::OnOpensimUdpConnectionError(Communication::ConnectionInterface& connection)
    {
        QString message = QString("OpenSim udp connect went to a error state");
        Communication::CommunicationModule::LogDebug(message.toStdString());
    }

    void OpenSimChatWidget::ConnectSlotsToChatSession()
    {
        QObject::connect(publicChat_, SIGNAL( MessageReceived(const Communication::ChatMessageInterface&) ),
                         this, SLOT ( MessageRecieved(const Communication::ChatMessageInterface& ) ));
        QObject::connect(opensim_chat_ui_.sendMessageLineEdit, SIGNAL( returnPressed() ),
                         this, SLOT( SendChatMessage() ));
    }

    void OpenSimChatWidget::MessageRecieved(const Communication::ChatMessageInterface &msg)
    {
        QString timestamp(QString("%1 %2").arg(msg.GetTimeStamp().date().toString("dd.MM.yyyy"),msg.GetTimeStamp().time().toString("hh:mm:ss")));
        QString who(msg.GetOriginator()->GetName());
        QString message(msg.GetText());
        QString htmlcontent("<span style='color:#99b2ff;'>[");
        htmlcontent.append(timestamp);
        if ( opensimConnection_->GetUserID() != msg.GetOriginator()->GetID() )
            htmlcontent.append("]</span> <span style='color:#0099FF;'>");
        else
        {
            htmlcontent.append("]</span> <span style='color:#FF3330;'>");
            who = "Me";
        }
        htmlcontent.append(who);
        htmlcontent.append(": </span><span style='color:black;'>");
        htmlcontent.append(message);
        htmlcontent.append("</span>");
        opensim_chat_ui_.worldChatTextEdit->appendHtml(htmlcontent);
    }

    void OpenSimChatWidget::SendChatMessage()
    {
        if (!opensim_chat_ui_.sendMessageLineEdit->text().isEmpty())
        {
            QString message(opensim_chat_ui_.sendMessageLineEdit->text());
            publicChat_->SendChatMessage(message);
            opensim_chat_ui_.sendMessageLineEdit->clear();
        }
    }


}