
#include "StableHeaders.h"
#include "Foundation.h"

#include "TPContact.h"


namespace Communication
{
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
		return PresenceStatusPtr( (PresenceStatus*)presence_status_.get() );
	}

	ContactInfoList TPContact::GetContactInfoList()
	{
		return contact_infos_;
	}

} // end of namespace: Communication