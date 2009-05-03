
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
		return presence_status_;
	}

	ContactInfoList TPContact::GetContactInfos()
	{
		return contact_infos_;
	}

} // end of namespace: Communication