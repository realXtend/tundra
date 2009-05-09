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
		TPContact();
		virtual void SetName(std::string name);
		virtual std::string GetName();
		virtual PresenceStatusPtr GetPresenceStatus();
		virtual ContactInfoListPtr GetContactInfoList();
		virtual ContactInfoPtr GetContactInfo(std::string protocol);
		virtual void AddContactInfo(ContactInfoPtr contact_info);


	protected:
//		void UpdatePresence(PresenceStatusPtr p); // called by TelepathyCommunication

		std::string name_;
		PresenceStatusPtr presence_status_;
		ContactInfoListPtr contact_infos_;
	};

} // end of namespace: Communication

#endif // incl_TPContact_h