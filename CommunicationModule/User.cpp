#include "User.h"

namespace TpQt4Communication
{
	User::User(Tp::ConnectionPtr tp_connection): user_id_(""),name_(""), protocol_(""), tp_connection_(tp_connection)
	{
		tp_contact_ = tp_connection->selfContact();
		user_id_ = tp_contact_->id().toStdString();
		name_ = tp_contact_->alias().toStdString();
		presence_status_ = tp_contact_->presenceStatus().toStdString();
		presence_message_ = tp_contact_->presenceMessage().toStdString();
	}

	void User::SetPresenceStatus(std::string status)
	{
		presence_status_ = status;
		QString s = presence_status_.c_str();
		QString m = presence_message_.c_str();
		tp_connection_->setSelfPresence(s,m);
	}

	void User::SetPresenceMessage(std::string message)
	{
		presence_message_ = message;
		QString s = presence_status_.c_str();
		QString m = presence_message_.c_str();
		tp_connection_->setSelfPresence(s,m);
	}

	std::string User::GetUserID()
	{
		return user_id_;
	}

	std::string User::GetName()
	{
		return name_;
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

	void User::RemoveContact()
	{
		//tp_connection_->reve
	}

} // end of TpQt4Communication: 
