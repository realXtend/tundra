#ifndef incl_Contact_h
#define incl_Contact_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "PresenceStatus.h"

namespace Communication
{
	/**
	 *  Implementation of ContactInterface
	 */
	class Contact : ContactInterface
	{
		friend class CommunicationManager;
	public:
		Contact(std::string id);
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

#endif // incl_Contact_h