#ifndef incl_TPContact_h
#define incl_TPContact_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPPresenceStatus.h"

namespace Communication
{
	class TPContact : Contact
	{
		friend class TelepathyCommunication;
	public:
		TPContact(std::string id);
		virtual void SetName(std::string name);
		virtual std::string GetName();
		virtual PresenceStatusPtr GetPresenceStatus();
		virtual ContactInfoListPtr GetContactInfoList();
		virtual ContactInfoPtr GetContactInfo(std::string protocol);
		virtual void AddContactInfo(ContactInfoPtr contact_info);
	protected:
		std::string id_; // presence id on python side
		std::string name_;
		PresenceStatusPtr presence_status_;
		ContactInfoListPtr contact_infos_;
	};

} // end of namespace: Communication

#endif // incl_TPContact_h