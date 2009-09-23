#include "User.h"

namespace TpQt4Communication
{
	User::User(Tp::ConnectionPtr tp_connection): user_id_(""), protocol_(""), tp_connection_(tp_connection)
	{
		tp_contact_ = tp_connection->selfContact();
		//user_id_ = tp_contact_->id().toStdString();
	}


	void User::SetPresenceStatus(std::string status)
	{
		presence_status_ = status;
		QString s = presence_status_.c_str();
		QString m = presence_message_.c_str();
		tp_connection_->setSelfPresence(m,s);
	}

	void User::SetPresenceMessage(std::string message)
	{
		presence_message_ = message;
		QString s = presence_status_.c_str();
		QString m = presence_message_.c_str();
		tp_connection_->setSelfPresence(m,s);
	}


	std::string User::GetUserID()
	{
		return user_id_;
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

	std::string User::GetPresenceStatus()
	{
		return presence_status_;
	}

	std::string User::GetPresenceMessage()
	{
		return presence_message_;
	}


	ContactVector User::GetContacts()
	{
		ContactVector contacts;
		contacts.assign(contacts_.begin(), contacts_.end());
		return contacts;
	}

} // end of TpQt4Communication: 
