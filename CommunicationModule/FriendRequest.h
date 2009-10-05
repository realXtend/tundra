#ifndef incl_Comm_FriendRequest_h
#define incl_Comm_FriendRequest_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include <QObject>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingContacts>
#include "Communication.h"



/**
 *  Implementation of FriendRequestInterface
 *
 *  Uses CommunicationManagers python script object for sending accept and deny messages to IM server
 */
namespace TpQt4Communication
{
	/**
	 * Friend request received from IM server. Friend request can be accepted or rejected.
	 * FriendRequest object is created automatically in Connection object and signalied out
	 * with FriendRqeustReceived signal. Received requests can also be obtained with
	 * GetFriendRequests method of Connection object
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
		enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED, STATE_ERROR };

		//! Returns IM address of originator of friend request
		Address GetOriginator();

		//! Returns IM address of target of friend request
//		Address GetTarget();

		//!
		State GetState();

		//! Accept friend request
		//! Friend will be added to friend list
		void Accept();

		//! Reject friend request
		void Reject();
	private:
		State state_;
		Address from_;
		Address to_;
		Tp::ContactPtr tp_contact_;
	protected slots:
		void OnPresencePublicationAuthorized(Tp::PendingOperation* op);
		void OnPresenceSubscriptionResult(Tp::PendingOperation* op);
	};
	typedef boost::weak_ptr<FriendRequest> FriendRequestWeakPtr;
	typedef std::vector<FriendRequest*> FriendRequestVector;


	/**
	 *  A friend request send by user to given target address.
	 *  When target accpet or reject the request or on error occures then
	 *  Ready signals is emited.
	 *
	 *  Class handles signaling with underlaying telepathy Connection and Contact objects
	 */
	class PendingFriendRequest : QObject
	{
		Q_OBJECT
		friend class Connection;
		PendingFriendRequest(const Tp::ConnectionPtr tp_connection, const Address &target, const std::string &message);
	public:
		enum Result {STATE_WAITING, STATE_ACCEPTED, STATE_REJECTED, STATE_ERROR};
//		void Cancel();
		Address GetTarget();
		Result GetResult();
	private:
		Tp::ConnectionPtr tp_connection_;
		Address target_;
		std::string message_;
	
	private slots:
		void OnContactRetrievedForFriendRequest(Tp::PendingOperation *op);
		void OnPresenceSubscriptionResult(Tp::PendingOperation* op);
	signals:
		void Ready(PendingFriendRequest*, Result);
	};
	typedef std::vector<PendingFriendRequest*> PendingFriendRequestVector;

} // end of namespace: TpQt4Communication


#endif // incl_Comm_FriendRequest_h
