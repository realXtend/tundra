#include "StableHeaders.h"
#include "Foundation.h"

#include "Contact.h"


namespace Communication
{
	Contact::Contact(std::string id): name_(""), id_(id)
	{
		contact_infos_ = ContactInfoListPtr( new ContactInfoList() );
		presence_status_ = PresenceStatusPtr( (PresenceStatusInterface*) new PresenceStatus() );
	}

	void Contact::SetName(std::string name)
	{
		name_ = name;
		
	}

	std::string Contact::GetName()
	{
		return name_;
	}

	PresenceStatusPtr Contact::GetPresenceStatus()
	{
		return presence_status_;
	}

	ContactInfoListPtr Contact::GetContactInfoList()
	{
		return contact_infos_;
	}

	ContactInfoPtr Contact::GetContactInfo(std::string protocol)
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
	void Contact::AddContactInfo(ContactInfoPtr contact_info)
	{
		contact_infos_->push_back(contact_info);
	}

} // end of namespace: Communication