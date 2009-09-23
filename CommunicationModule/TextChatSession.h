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
		~TextChatSession();
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
		void MessageReceived(Message &message);
	};
	typedef boost::shared_ptr<TextChatSession> TextChatSessionPtr;
	typedef boost::weak_ptr<TextChatSession> TextChatSessionWeakPtr;
	typedef std::vector<TextChatSession*> TextChatSessionVector;


} // end of namespace: TpQt4Communication

#endif // incl_Comm_TextChatSession_h
