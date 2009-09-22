#ifndef incl_Comm_FriendRequest_h
#define incl_Comm_FriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QObject>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingOperation>
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
	class FriendRequest : QObject
	{
		Q_OBJECT
		friend class Connection;
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "Communication(FriendRequest)"; } // for logging functionality
	protected:
		FriendRequest(Tp::ContactPtr contact);
	public:
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
		Address GetAddressFrom();
		Address GetAddressTo();
		State GetState();

		void Accecpt();
		void Reject();
	private:
		State state_;
		Address from_;
		Address to_;
		Tp::ContactPtr tp_contact_;
	};
	typedef boost::weak_ptr<FriendRequest> FriendRequestWeakPtr;
	typedef std::vector<FriendRequest*> FriendRequestVector;

} // end of namespace: TpQt4Communication

#endif // incl_Comm_FriendRequest_h