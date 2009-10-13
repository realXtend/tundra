#ifndef incl_Comm_ChatSession_h
#define incl_Comm_ChatSession_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/ReceivedMessage>

#include "Communication.h"
#include "ChatMessage.h"

namespace TpQt4Communication
{
	/**
	 *  
	 *
	 *
	 */
	class ChatSession : QObject
	{
		Q_OBJECT 
		friend class Connection;
		friend class ChatSessionRequest;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality
		ChatSession();
		ChatSession(Tp::TextChannelPtr tp_text_channel);		
	public:
		~ChatSession();
		enum State {STATE_INITIALIZING, STATE_READY, STATE_ERROR, STATE_CLOSED};

//		void Invite(Address a);
		void SendTextMessage(std::string text);
		void Close();
		ChatMessageVector GetMessageHistory(); 
		State GetState();

	private:
		ChatMessageVector messages_;
		Tp::TextChannelPtr tp_text_channel_;
		State state_;
		Address originator_;

	private Q_SLOTS:
		void OnTextChannelCreated(Tp::PendingOperation* op);
		void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
		void OnChannelReady(Tp::PendingOperation*);
		void OnMessageReceived(const Tp::ReceivedMessage &message);
		void OnChannelMessageSent(const Tp::Message &, Tp::MessageSendingFlags, const QString &);
		void OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &);
		//void onTextMessageSend(Tp::PendingOperation *op);
	signals:
		void Ready();
		void Error(QString &reason);
		void Closed();
		void MessageReceived(ChatMessage &message);
	};
	typedef boost::shared_ptr<ChatSession> ChatSessionPtr;
	typedef boost::weak_ptr<ChatSession> ChatSessionWeakPtr;
	typedef std::vector<ChatSession*> ChatSessionVector;

} // end of namespace: TpQt4Communication

#endif // incl_Comm_ChatSession_h
