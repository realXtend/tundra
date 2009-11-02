#ifndef incl_OpenSimChat_h
#define incl_OpenSimChat_h

#include "StableHeaders.h"
#include "Foundation.h"
#include "UICanvas.h"
#include "QtModule.h"

#include <QtGui>

#include "../interface.h"
#include "../Credentials.h"
#include "../CommunicationService.h"
#include "../CommunicationModule.h"

namespace CommunicationUI
{
	class OpenSimChat : public QObject
	{

	Q_OBJECT

	public:
		OpenSimChat(Foundation::Framework *framework);
		virtual ~OpenSimChat();

	public slots:
		void ToggleShow();
		void DestroyThis();
		void ToggleChatVisibility();
		// Connectiong
		void OnOpensimUdpConnectionReady(Communication::ConnectionInterface&);
		void OnOpensimUdpConnectionError(Communication::ConnectionInterface&);
		// ChatSession signal handlers
		void MessageRecieved(const Communication::ChatMessageInterface &msg);
		void MessageRecieved(const QString& text, const Communication::ChatSessionParticipantInterface& participant);
		void SendMessage();
		void ParticipantJoined(const Communication::ChatSessionParticipantInterface& participant);
		void ParticipantLeft(const Communication::ChatSessionParticipantInterface& participant);
	
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

		bool visible_;

	};
}

#endif // incl_OpenSimChat_h