// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OutgoingFriendRequest.h"

namespace TelepathyIM
{
	OutgoingFriendRequest::OutgoingFriendRequest(const QString &target_id, const QString &message, Tp::ConnectionPtr tp_connection) : state_(STATE_PENDING), tp_connection_(tp_connection), message_(message)
	{
		target_id_ = target_id;
		Tp::PendingContacts *pending_contacts = tp_connection_->contactManager()->contactsForIdentifiers(QStringList() << target_id);
		connect(pending_contacts,
			    SIGNAL( finished(Tp::PendingOperation *) ),
				SLOT( OnContactRetrievedForFriendRequest(Tp::PendingOperation *) ));
	}

	QString OutgoingFriendRequest::GetTargetId()
	{
		return target_id_;
	}

	QString OutgoingFriendRequest::GetTargetName()
	{
		return target_name_;
	}

	void OutgoingFriendRequest::OnContactRetrievedForFriendRequest(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			reason_ = op->errorMessage();
			emit( Error(this) );
			return;
		}
		Tp::PendingContacts *pending_contacts = qobject_cast<Tp::PendingContacts *>(op);
		QList<Tp::ContactPtr> contacts = pending_contacts->contacts();
		
		assert( contacts.size() == 1); // We have request only one contact 
		Tp::ContactPtr contact = contacts.first();
		target_name_ = contact->alias();

		// Do the presence subscription
		Tp::PendingOperation* p = contact->requestPresenceSubscription(message_);
		connect(p, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnPresenceSubscriptionResult(Tp::PendingOperation*) ));
	}

	void OutgoingFriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			reason_ = op->errorMessage();
			emit( Error(this) );
			return;
		}
		state_ = STATE_SENT;
		emit( Sent(this) );
	}

} // end of namespace: TelepathyIM
