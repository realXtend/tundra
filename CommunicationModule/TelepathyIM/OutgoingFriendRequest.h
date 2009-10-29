#ifndef incl_Communication_TelepathyIM_OutgoingFriendRequest_h
#define incl_Communication_TelepathyIM_OutgoingFriendRequest_h

#include "..\interface.h"
#include <QObject>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>

namespace TelepathyIM
{
	/**
	 *
	 *
	 *
	 */
	class OutgoingFriendRequest : public QObject
	{
		Q_OBJECT
	public:
		enum State {STATE_PENDING, STATE_SENT, STATE_ACCEPTED, STATE_REJECTED, STATE_ERROR};
		OutgoingFriendRequest(const QString &target_id, const QString &message, Tp::ConnectionPtr tp_connection);
		QString GetTargetId();
		QString GetTargetName();
	protected:
		State state_;

		// Textual description about error if state_ == STATE_ERROR
		QString reason_;
		Tp::ConnectionPtr tp_connection_;
		QString message_;
		QString target_id_;
		QString target_name_;

	protected slots:
		virtual void OnContactRetrievedForFriendRequest(Tp::PendingOperation *op);
		virtual void OnPresenceSubscriptionResult(Tp::PendingOperation *op);
	signals:
		void Sent(OutgoingFriendRequest *request);
		void Error(OutgoingFriendRequest *request);
		void Accepted(OutgoingFriendRequest *request); // @todo needed?
		void Rejected(OutgoingFriendRequest *request); // @todo needed?
	};
	typedef std::vector<OutgoingFriendRequest*> OutgoingFriendRequestVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_OutgoingFriendRequest_h