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

	Address FriendRequest::GetAddressFrom()
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

} // end of namespace: TpQt4Communication

