
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPContact.h"


namespace Communication
{
	TPContact::TPContact(): name_("")
	{
		contact_infos_ = ContactInfoListPtr( new ContactInfoList() );
		presence_status_ = PresenceStatusPtr( (PresenceStatus*) new TPPresenceStatus() );
	}

	void TPContact::SetName(std::string name)
	{
		name_ = name;
	}

	std::string TPContact::GetName()
	{
		return name_;
	}

	PresenceStatusPtr TPContact::GetPresenceStatus()
	{
		return presence_status_;
	}

	ContactInfoListPtr TPContact::GetContactInfoList()
	{
		return contact_infos_;
	}

	ContactInfoPtr TPContact::GetContactInfo(std::string protocol)
	{
		for (ContactInfoList::iterator i = contact_infos_->begin(); i < contact_infos_->end(); i++)
		{
			if ( (*i)->GetProperty("protocol").compare(protocol) == 0)
			{
				return *i;
			}
		}

		// contact info for given protocol didn't found
		// we return an empty contact info object
		return ContactInfoPtr(new ContactInfo());
	}

	// todo: might be a good idea to test for duplicates
	void TPContact::AddContactInfo(ContactInfoPtr contact_info)
	{
		contact_infos_->push_back(contact_info);
	}

} // end of namespace: Communication