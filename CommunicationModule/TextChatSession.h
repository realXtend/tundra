#ifndef incl_Comm_TextChatSession_h
#define incl_Comm_TextChatSession_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>

#include "Communication.h"
#include "IMMessage.h"

namespace TpQt4Communication
{
	/**
	EVENTS:
		- ParticipantLeft
		- ParticipantJoined
		- TextMessageReceived
		- SessionClosed
	*/
	class TextChatSession : QObject
	{
		Q_OBJECT 
		friend class TextChatSession;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality
		
	public:
		enum State {STATE_INITIALIZING, STATE_READY, STATE_ERROR, STATE_CLOSED};
		TextChatSession();
		TextChatSession(Tp::TextChannelPtr tp_text_channel);
		void Invite(Address a);
		void SendTextMessage(std::string text);
		void Close();
		MessageVector GetMessageHistory(); // todo: return value
		State GetState();
	private:
		MessageVector messages_;
		Tp::TextChannelPtr tp_text_channel_;
		State state_;
	private Q_SLOTS:
		void OnChannelReady(Tp::PendingOperation*);
		void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
		void OnMessageReceived(const Tp::ReceivedMessage &message);
		void OnTextChannelCreated(Tp::PendingOperation* op);
	signals:
		void Ready();
		void Error(QString &reason);
		void Closed();
		
	};
	typedef boost::shared_ptr<TextChatSession> TextChatSessionPtr;
	typedef boost::weak_ptr<TextChatSession> TextChatSessionWeakPtr;

	/**
	 * Holds received TextChannel until user have made decision to accept or reject.
	 *
	 */
	class TextChatSessionRequest : QObject
	{
		Q_OBJECT
			friend class Connection;
	protected:
		TextChatSessionRequest(TextChatSession* session );
		TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel);
	public:

		//! Create a new TextChatSession object and return it
		TextChatSession* Accept();

		//! Terminates the session request
		void Reject();

		//! Return IM address of originator of request
		Address GetOriginator();

		//! Return a message if one is attached to request
		std::string GetMessage();

	private:
		TextChatSession* session_; // NECESSARY ?

		//! 
		std::string message_;

		//!
		Tp::TextChannelPtr tp_text_channel_;
	//slots:
		//void OnChannelReady(Tp::PendingOperation*);
		//void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
	signals:
		//! Request was canceled by it's originator
		void Canceled();
	};
	typedef boost::weak_ptr<TextChatSessionRequest> TextChatSessionRequestWeakPtr;
	typedef std::vector<TextChatSessionRequest*> TextChatSessionRequestVector;
}

#endif // incl_Comm_TextChatSession_h
