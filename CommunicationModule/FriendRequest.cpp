#include "StableHeaders.h"
#include "Foundation.h"
#include "CommunicationManager.h"
#include "FriendRequest.h"


namespace Communication
{
	FriendRequest::FriendRequest(ContactInfoPtr contact_info)
	{
		contact_info_ = contact_info;
	}

	ContactInfoPtr FriendRequest::GetContactInfo()
	{
		return contact_info_;
	}

	void FriendRequest::Accept()
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CAcceptContactRequest", contact_info_->GetProperty("address").c_str() );
		// todo: remove this from list
	}

	void FriendRequest::Deny()
	{
		CommunicationManager::GetInstance()->CallPythonCommunicationObject("CDenyContactRequest", contact_info_->GetProperty("address").c_str() );
		// todo: remove this from list
	}

} // end of namespace: Communication

