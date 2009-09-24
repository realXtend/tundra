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
		friend class TextChatSessionRequest;
		friend class TextChatSession;

		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	private:
		Contact(Tp::ContactPtr tp_contact);
		void ConnectSignals();
	public:
		Address GetAddress();
		std::string GetRealName();
		std::string GetPresenceStatus();
		std::string GetPresenceMessage();

	private:
		Address address_;
		std::string real_name_;
		Tp::ContactPtr tp_contact_;
		std::string presence_status_;
		std::string presence_message_;

	signals:
		void StateChanged();

	private Q_SLOTS:
		void OnContactChanged();
	};
	typedef boost::weak_ptr<Contact> ContactWeakPtr;
	typedef std::vector<Contact*> ContactVector;

} // end of namespace: Communication

#endif // incl_Comm_Contact_h