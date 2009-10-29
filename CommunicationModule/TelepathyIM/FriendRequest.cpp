#include "FriendRequest.h"

namespace TelepathyIM
{
	FriendRequest::FriendRequest(Tp::ContactPtr contact) : state_(STATE_PENDING), tp_contact_(contact)
	{
	}
	
	QString FriendRequest::GetOriginatorName() const
	{
		return tp_contact_->alias();
	}

	QString FriendRequest::GetOriginatorID() const
	{
		return tp_contact_->id();
	}

	Communication::FriendRequestInterface::State FriendRequest::GetState() const
	{
		return state_;
	}

	void FriendRequest::Accept()
	{
		QString message;
		message = ""; // we don't want to send any particular message at this point
		Tp::PendingOperation* p = tp_contact_->authorizePresencePublication(message);
		QObject::connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT(OnPresencePublicationAuthorized(Tp::PendingOperation*)) );

		// Request presence subscription from friend request originator so that
		// publicity is on the same level to both directions
		message = ""; // we don't want to send any particular message at this point
		Tp::PendingOperation* op = tp_contact_->requestPresenceSubscription(message);
		connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnPresenceSubscriptionResult(Tp::PendingOperation*) ) );
		state_ = STATE_ACCEPTED;
	}	

	void FriendRequest::Reject()
	{
		QString message = ""; // we don't want to send any particular message at this point
		Tp::PendingOperation* p = tp_contact_->removePresencePublication(message);
		//! todo connect signal 
		state_ = STATE_REJECTED;
	}

	void FriendRequest::OnPresencePublicationAuthorized(Tp::PendingOperation* op)
	{
		//! @todo IMPLEMENT
		//!       * log error messages if operation wasn't success
	}

	void FriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation* op)
	{
		//! @todo IMPLEMENT
		//        * Cancel then friendship because the contact doesn't allow to subscipe own presence.
	}

} // end of namespace: TelepathyIM
