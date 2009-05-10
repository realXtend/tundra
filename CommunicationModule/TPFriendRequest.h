#ifndef incl_TPFriendRequest_h
#define incl_TPFriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPFriendRequest : FriendRequestInterface
	{
	public:
		TPFriendRequest(ContactInfoPtr contact_info);
		virtual ContactInfoPtr GetContactInfo();
		virtual void Accept();
		virtual void Deny();
	protected:
		ContactInfoPtr contact_info_;
	};

} // end of namespace: Communication

#endif // incl_TPFriendRequest_h