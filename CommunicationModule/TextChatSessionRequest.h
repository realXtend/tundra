#ifndef incl_Comm_TextChatSessionRequest_h
#define incl_Comm_TextChatSessionRequest_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/PendingOperation>

#include "Communication.h"
//#include "Connection.h"
#include "TextChatSession.h"
#include "Contact.h"

namespace TpQt4Communication
{
	/**
	 * Holds received TextChannel until user have made decision to accept or reject the request.
	 *
	 */
	class TextChatSessionRequest : QObject
	{
		Q_OBJECT
		friend class Connection;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	protected:
		TextChatSessionRequest(TextChatSession* session );
		TextChatSessionRequest(Tp::TextChannelPtr tp_text_channel, Contact* from);
	public:

		//! Create a new TextChatSession object and return it
		TextChatSessionPtr Accept();

		//! Terminates the session request
		void Reject();

		//! Return IM address of originator of request
		Address GetOriginator();
		Contact* GetOriginatorContact();

		//! Return a message if one is attached to request
		std::string GetMessage();

	private:
		TextChatSession* session_; // NECESSARY ?

		//! 
		std::string message_;

		//!
		Tp::TextChannelPtr tp_text_channel_;

		Contact* from_;
	//slots:
		//void OnChannelReady(Tp::PendingOperation*);
		//void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
	//slots:
		void OnTextChannelClosed(Tp::PendingOperation* op);

	signals:
		//! Request was canceled by it's originator
		void Canceled();

	};
	typedef boost::weak_ptr<TextChatSessionRequest> TextChatSessionRequestWeakPtr;
	typedef std::vector<TextChatSessionRequest*> TextChatSessionRequestVector;


} // end of namespace: TpQt4Communication

#endif // incl_Comm_TextChatSessionRequest_h
