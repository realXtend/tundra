#include "StableHeaders.h"
#include "Foundation.h"
#include "TelepathyCommunication.h"
#include "TPFriendRequest.h"


namespace Communication
{
	TPFriendRequest::TPFriendRequest(ContactInfoPtr contact_info)
	{
		contact_info_ = contact_info;
	}

	ContactInfoPtr TPFriendRequest::GetContactInfo()
	{
		return contact_info_;
	}

	void TPFriendRequest::Accept()
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact_info_->GetProperty("address").c_str());
		args[0] = buf1;
		std::string method = "CAcceptContactRequest"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
		std::string syntax = "s";
		Foundation::ScriptObject* ret = TelepathyCommunication::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
	}

	void TPFriendRequest::Deny()
	{
		char** args = new char*[1];
		char* buf1 = new char[1000];
		strcpy(buf1, contact_info_->GetProperty("address").c_str());
		args[0] = buf1;
		std::string method = "CDenyContactRequest"; // todo: CCreateIMSessionJabber, CCreateIMSessionSIP, etc.
		std::string syntax = "s";
		Foundation::ScriptObject* ret = TelepathyCommunication::GetInstance()->python_communication_object_->CallMethod(method, syntax, args);
	}

} // end of namespace: Communication

