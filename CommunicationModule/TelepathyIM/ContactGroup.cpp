#include "ContactGroup.h"

namespace TelepathyIM
{
	QString ContactGroup::GetName() const
	{
		//! @todo IMPLEMENT
		return "";
	}

	void ContactGroup::SetName(const QString &name)
	{
		//! @todo IMPLEMENT
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
		//! @todo IMPLEMENT
		Communication::ContactGroupVector empty_vector;
		return empty_vector;
	}

	void ContactGroup::AddContact(Contact* contact)
	{
		contacts_.push_back(contact);
	}

} // end of namespace: TelepathyIM
