#include "FriendRequest.h"

namespace TelepathyIM
{
	FriendRequest::FriendRequest() : state_(STATE_PENDING)
	{
		//! @todo IMPLEMENT
	}
	
	QString FriendRequest::GetOriginatorName() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	QString FriendRequest::GetOriginatorID() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	Communication::FriendRequestInterface::State FriendRequest::GetState() const
	{
		return state_;
	}

	void FriendRequest::Accept()
	{
		//! @todo IMPLEMENT
	}	

	void FriendRequest::Reject()
	{
		//! @todo IMPLEMENT
	}


} // end of namespace: TelepathyIM
