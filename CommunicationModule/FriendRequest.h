#ifndef incl_Comm_FriendRequest_h
#define incl_Comm_FriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include "Communication.h"

/**
 *  Implementation of FriendRequestInterface
 *
 *  Uses CommunicationManagers python script object for sending accept and deny messages to IM server
 */
namespace TpQt4Communication
{
	/**
	 * 
	 *
	 *
	 */
	class FriendRequest
	{
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		Address GetAddressFrom();
		Address GetAddressTo();
		State GetState();
	private:
		State state_;
		Address from_;
		Address to_;
	};
	typedef boost::weak_ptr<FriendRequest> FriendRequestWeakPtr;
	typedef std::vector<FriendRequest*> FriendRequestList;

} // end of namespace: TpQt4Communication

#endif // incl_Comm_FriendRequest_h