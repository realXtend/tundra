#ifndef incl_Comm_ChatSessionRequest_h
#define incl_Comm_ChatSessionRequest_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/PendingOperation>

#include "Communication.h"
#include "ChatSession.h"
#include "Contact.h"

namespace TpQt4Communication
{
	/**
	 * Holds received telepathy TextChannel until user have made decision to accept or reject the request.
	 * 
	 */
	class ChatSessionRequest : QObject
	{
		Q_OBJECT
		friend class Connection;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	protected:
//		ChatSessionRequest(ChatSession* session );
		ChatSessionRequest(Tp::TextChannelPtr tp_text_channel);
	public:

		//! Create a new ChatSession object and return it
		ChatSessionPtr Accept();

		//! Terminates the session request
		void Reject();

		//! Return IM address of originator of request
		Address GetOriginator();
		Contact* GetOriginatorContact();

		//! Return a message if one is attached to request
		std::string GetMessage();

	private:
		ChatSession* session_;

		//! 
		std::string message_;

		//!
		Tp::TextChannelPtr tp_text_channel_;

		Contact* from_;
	//slots:
		//void OnChannelReady(Tp::PendingOperation*);
		//void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
	//slots:
	private slots:
		void OnTextChannelClosed(Tp::PendingOperation* op);
		void OnChatSessionReady();

	signals:
		//! Request was canceled by it's originator
		void Canceled();

		//! Fired when actual signaling in under a hood is ready
		void Ready(ChatSessionRequest*);

	};
	typedef boost::weak_ptr<ChatSessionRequest> ChatSessionRequestWeakPtr;
	typedef std::vector<ChatSessionRequest*> ChatSessionRequestVector;


} // end of namespace: TpQt4Communication

#endif // incl_Comm_ChatSessionRequest_h
