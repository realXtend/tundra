#include "ContactGroup.h"

namespace TelepathyIM
{
	ContactGroup::ContactGroup(const QString &name) : name_(name)
	{

	}

	ContactGroup::~ContactGroup()
	{
		//! This class isn't responsible about dynamic Contact objects
		//! So we do NOT free momory of those

		for (ContactGroupVector::iterator i = groups_.begin(); i != groups_.end(); ++i)
		{
			SAFE_DELETE(*i);
		}
	}

	QString ContactGroup::GetName() const
	{
		return name_;
	}

	void ContactGroup::SetName(const QString &name)
	{
		name_ = name;
		//! Telepathy doesn't support naming the contact lists
		//! @todo Check this
	}

	Communication::ContactVector ContactGroup::GetContacts()
	{
		Communication::ContactVector contacts;
		for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
		{
			contacts.push_back(*i);
		}
		return contacts;
	}

	Communication::ContactGroupVector ContactGroup::GetGroups()
	{
		Communication::ContactGroupVector groups;
		for (ContactGroupVector::iterator i = groups_.begin(); i != groups_.end(); ++i)
		{
			groups.push_back(*i);
		}
		return groups;
	}

	void ContactGroup::AddContact(Contact* contact)
	{
		contacts_.push_back(contact);
	}

} // end of namespace: TelepathyIM
