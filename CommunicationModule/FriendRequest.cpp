#include "StableHeaders.h"
#include "Foundation.h"
#include "FriendRequest.h"


namespace TpQt4Communication
{
	FriendRequest::FriendRequest(Tp::ContactPtr contact)
	{
		tp_contact_ = contact;
	}

	void FriendRequest::Accecpt()
	{
		LogInfo("Incoming friend request accepted"); 
		QString message = "123..."; // TEST 
		Tp::PendingOperation* p = tp_contact_->authorizePresencePublication(message);
	}

	void FriendRequest::Reject()
	{
		LogInfo("Incoming friend request rejected");
		QString message = "sorry..."; // TEST
		Tp::PendingOperation* p = tp_contact_->removePresencePublication(message);
	}

} // end of namespace: TpQt4Communication

