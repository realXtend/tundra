#ifndef incl_OpenSimChat_h
#define incl_OpenSimChat_h

#include <StableHeaders.h>
#include <NetworkEvents.h>
#include <UICanvas.h>
#include <QtModule.h>
#include <QtGui>

#include "../interface.h"
#include "../Credentials.h"
#include "../CommunicationService.h"

namespace CommunicationUI
{
	class OpenSimChat : public QObject
	{

	Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "OpensimChat"; } // for logging functionality

	public:
		OpenSimChat(Foundation::Framework *framework, OpenSimProtocol::ClientParameters clientParams);
		virtual ~OpenSimChat();

	public slots:
		void ToggleShow();
		void DestroyThis();
		void ToggleChatVisibility();
		void AdjustInternalWidgets(const QSize& newSize);
		// Connecting
		void OnOpensimUdpConnectionReady(Communication::ConnectionInterface&);
		void OnOpensimUdpConnectionError(Communication::ConnectionInterface&);
		// ChatSession signal handlers
		void MessageRecieved(const Communication::ChatMessageInterface &msg);
		void SendMessage();
	
	private:
		void InitModuleConnections();
		void InitCommConnections();
		void InitUserInterface();
		void ConnectSlotsToChatSession();

		QWidget *internalWidget_;
		QFrame *mainFrame_;
		QLineEdit *chatInput_;
		QPlainTextEdit *chatTextBox_;
		QPushButton *buttonHide_;

		Communication::CommunicationServiceInterface *communicationService_;
		Communication::ConnectionInterface *opensimConnection_;
		Communication::ChatSessionInterface *publicChat_;

		Foundation::Framework *framework_;
		boost::shared_ptr<QtUI::QtModule> qtModule_;
		boost::shared_ptr<QtUI::UICanvas> canvas_;

		OpenSimProtocol::ClientParameters clientParams_;

		bool visible_;

	};
}

#endif // incl_OpenSimChat_h