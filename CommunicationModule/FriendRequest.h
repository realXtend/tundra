#ifndef incl_FriendRequest_h
#define incl_FriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

/**
 *  Implementation of FriendRequestInterface
 *
 *  Uses CommunicationManagers python script object for sending accept and deny messages to IM server
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