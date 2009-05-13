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
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact_info_->GetProperty("address").c_str());
		args[0] = buf1;
		std::string method = "CAcceptContactRequest"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
		std::string syntax = "s";
		Foundation::ScriptObject* ret = CommunicationManager::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
	}

	void FriendRequest::Deny()
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact_info_->GetProperty("address").c_str());
		args[0] = buf1;
		std::string method = "CDenyContactRequest"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
		std::string syntax = "s";
		Foundation::ScriptObject* ret = CommunicationManager::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
	}

} // end of namespace: Communication

