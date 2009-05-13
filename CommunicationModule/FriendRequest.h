#ifndef incl_FriendRequest_h
#define incl_FriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

/*
 * =========================
 * Hello there: This is a test class !
 */
namespace Communication
{
	class FriendRequest : FriendRequestInterface
	{
	public:
		FriendRequest(ContactInfoPtr contact_info);
		virtual ContactInfoPtr GetContactInfo();
		virtual void Accept();
		virtual void Deny();
	protected:
		ContactInfoPtr contact_info_;
	};

} // end of namespace: Communication

#endif // incl_FriendRequest_h