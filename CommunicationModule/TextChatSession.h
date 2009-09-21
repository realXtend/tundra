#ifndef incl_Comm_TextChatSession_h
#define incl_Comm_TextChatSession_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QObject>
#include <TelepathyQt4/TextChannel>

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
	public:
		TextChatSession();
		TextChatSession(Tp::TextChannelPtr tp_text_channel);
		void Invite(Address a);
		void SendTextMessage(std::string text);
		void Close();
		MessageVector GetMessageHistory(); // todo: return value
	private:
		MessageVector messages_;
		Tp::TextChannelPtr tp_text_channel_;
	private Q_SLOTS:
		void OnChannelReady(Tp::PendingOperation*);
		void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
	};
	typedef boost::shared_ptr<TextChatSession> TextChatSessionPtr;
	typedef boost::weak_ptr<TextChatSession> TextChatSessionWeakPtr;

	/**
	 * Holds received TextChannel until user have made decision to accept or reject.
	 *
	 *
	 *
	 */
	class TextChatSessionRequest : QObject
	{
		Q_OBJECT
	public:
		TextChatSessionRequest(TextChatSessionWeakPtr session );
	private:
		TextChatSessionWeakPtr session_;
	private Q_SLOTS:
		//void OnChannelReady(Tp::PendingOperation*);
		//void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
	};
	typedef boost::weak_ptr<TextChatSessionRequest> TextChatSessionRequestWeakPtr;
	typedef std::vector<TextChatSessionRequestWeakPtr>	TextChatSessionRequestVector;
}

#endif // incl_Comm_TextChatSession_h
