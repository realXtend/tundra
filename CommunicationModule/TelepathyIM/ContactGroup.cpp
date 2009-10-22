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
		//! @todo IMPLEMENT
		Communication::ContactVector empty_vector;
		return empty_vector;
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
