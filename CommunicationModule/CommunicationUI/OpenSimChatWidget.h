// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CommunicationUI_OpenSimChatWidget_h
#define incl_CommunicationUI_OpenSimChatWidget_h

//#include "StableHeaders.h"
#include "NetworkEvents.h"

#include "ui_OpenSimChatWidget.h"

//#include "interface.h"
//#include "Credentials.h"
//#include "CommunicationService.h"
#include "CommunicationModuleFwd.h"

namespace CommunicationUI
{
    class OpenSimChatWidget : public QWidget
    {

    Q_OBJECT

    public:
        explicit OpenSimChatWidget(ProtocolUtilities::ClientParameters clientParams);
        virtual ~OpenSimChatWidget();

    public slots:
        void OnOpensimUdpConnectionReady(Communication::ConnectionInterface&);
        void OnOpensimUdpConnectionError(Communication::ConnectionInterface&);
        
        void MessageRecieved(const Communication::ChatMessageInterface &msg);
        void SendChatMessage();
    
    private:
        void InitCommunicationConnections();
        void ConnectSlotsToChatSession();

        Ui::OpenSimChatWidget opensim_chat_ui_;

        Communication::CommunicationServiceInterface *communicationService_;
        Communication::ConnectionInterface *opensimConnection_;
        Communication::ChatSessionInterface *publicChat_;

        ProtocolUtilities::ClientParameters clientParams_;
    };
}

#endif // incl_CommunicationUI_OpenSimChatWidget_h