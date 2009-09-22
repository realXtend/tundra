#ifndef incl_Comm_Contact_h
#define incl_Comm_Contact_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include <QObject>
#include <TelepathyQt4/Contact>
#include "PresenceStatus.h"
#include "Communication.h"

namespace TpQt4Communication
{
	/**
	 *  Contact item in user's contact lisdt
	 *
	 */
	class Contact : QObject
	{
		Q_OBJECT

		friend class Connection;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	private:
		Contact(Tp::ContactPtr tp_contact);
		void ConnectSignals();
	public:
		Address GetAddress();
		std::string GetRealName();
		PresenceStatusWeakPtr GetPresenceStatus();
		//void SetRealName(std::string name);
	private:
		Address address_;
		std::string real_name_;
		PresenceStatusWeakPtr presence_status_;
		Tp::ContactPtr tp_contact_;

	signals:
		void StateChanged();

	private Q_SLOTS:
		void OnContactChanged();
	};
	typedef boost::weak_ptr<Contact> ContactWeakPtr;
	typedef std::vector<Contact*> ContactVector;


	/**
	 * DO WE ACTUALLY NEED THIS CLASS ???
	 *
	 * User's contact list aka Friendlist.
	 * Current implementation is flat but might be better to change to support hierarcy.
	 * EVENTS: 
	 *	- FriendRequestResponse
     * - FriendRequest
	 */
	class ContactList
	{
	public:
	private:
		
	};

} // end of namespace: Communication

#endif // incl_Comm_Contact_h