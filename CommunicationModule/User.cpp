#include "User.h"

namespace TpQt4Communication
{
	User::User(Tp::ConnectionPtr tp_connection): user_id_(""), protocol_(""), tp_connection_(tp_connection)
	{
		tp_contact_ = tp_connection->selfContact();
	}

	void User::SetPresenceStatus(std::string status, std::string message)
	{
		QString s(status.c_str());
		QString m(message.c_str());
		tp_connection_->setSelfPresence(s, m);
	}

	std::string User::GetUserID()
	{
		return user_id_;
	}

	PresenceStatus* User::GetPresenceStatus()
	{
		return &presence_status_;
	}

	std::string User::GetProtocol()
	{
		return protocol_;
	}

	void User::AddContacts(ContactVector &contacts)
	{
		contacts_.assign(contacts.begin(), contacts.end());
		emit ContactListChangend();
	}

	ContactVector User::GetContacts()
	{
		ContactVector contacts;
		contacts.assign(contacts_.begin(), contacts_.end());
		return contacts;
	}

} // end of TpQt4Communication: 
