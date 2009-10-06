#include "StableHeaders.h"
#include "Foundation.h"
#include "FriendRequest.h"


namespace TpQt4Communication
{
	FriendRequest::FriendRequest(Tp::ContactPtr contact)
	{
		LogDebug("FriendRequest object created.");
		tp_contact_ = contact;
	}

	void FriendRequest::Accept()
	{
		LogInfo("Incoming friend request accepted"); 
		QString message = "";
		Tp::PendingOperation* p = tp_contact_->authorizePresencePublication(message);
		QObject::connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT(OnPresencePublicationAuthorized(Tp::PendingOperation*)) );

		// Request presence subscription from friend request originator so that
		// publicity is on same level to both directions
		message = "";
		Tp::PendingOperation* op = tp_contact_->requestPresenceSubscription(message);
		connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnPresenceSubscriptionResult(Tp::PendingOperation*) ) );
		state_ = STATE_ACCEPTED;
	}

	void FriendRequest::Reject()
	{
		LogInfo("Incoming friend request rejected");
		QString message = "sorry..."; // TEST
		Tp::PendingOperation* p = tp_contact_->removePresencePublication(message);
		//! todo connect signal 
		state_ = STATE_REJECTED;
	}

	Address FriendRequest::GetOriginator()
	{
		assert(!tp_contact_.isNull());
		return tp_contact_->id().toStdString();
	}

	void FriendRequest::OnPresencePublicationAuthorized(Tp::PendingOperation* op)
	{
		if ( op->isError() )
		{
			LogInfo("Cannot authorize presence publication.");
			state_ = STATE_ERROR;
			return;
		}
		QString message = "Presence publication authorized successfully for: ";
		message.append(tp_contact_->id());
		LogInfo(message.toStdString());
	}

	void FriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation* op)
	{
		if (op->isError())
		{
			LogInfo("Cannot subscribe presence.");
			state_ = STATE_ERROR;
			return;
		}
		QString message = "Presence state subscribed successfully with: ";
		message.append(tp_contact_->id());
		LogInfo(message.toStdString());
	}

	PendingFriendRequest::PendingFriendRequest(const Tp::ConnectionPtr tp_connection, const Address &target, const std::string &message): tp_connection_(tp_connection), target_(target), message_(message)
	{
		Tp::PendingContacts *pending_contacts = tp_connection_->contactManager()->contactsForIdentifiers(QStringList()<<QString(target.c_str()));
		connect(pending_contacts, SIGNAL( finished(Tp::PendingOperation *) ), SLOT( OnContactRetrievedForFriendRequest(Tp::PendingOperation *) ));
	}

	Address PendingFriendRequest::GetTarget()
	{
		return target_;
	}

	void PendingFriendRequest::OnContactRetrievedForFriendRequest(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			QString message = "Failed to receive contact: ";
			message.append(op->errorMessage());
//			LogError(message.toStdString());
			return;
		}
//		LogInfo("Contact received for friend request.");
		Tp::PendingContacts *pending_contacts = qobject_cast<Tp::PendingContacts *>(op);
		QList<Tp::ContactPtr> contacts = pending_contacts->contacts();
		
		assert( contacts.size() == 1); // We have request only one contact 
		Tp::ContactPtr contact = contacts.first();

		// Do the presence subscription
		Tp::PendingOperation* p = contact->requestPresenceSubscription(message_.c_str());
		connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT(OnPresenceSubscriptionResult(Tp::PendingOperation*)));
	}

	void PendingFriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation* op)
	{
		if (op->isError())
		{
			// Presence subscription request delivery failed
			QString message = "Presence subscription request delivery failed: ";
			message.append(op->errorMessage());
			emit Ready(this, STATE_ERROR);
		}
		// Presence subscription request is successfully delivered to target
	}

} // end of namespace: TpQt4Communication

