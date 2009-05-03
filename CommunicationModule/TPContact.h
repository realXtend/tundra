#ifndef incl_TPContact_h
#define incl_TPContact_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPContact
	{
		friend class TelepathyCommunication;
	public:
		void SetName(std::string name);
		std::string GetName();
		PresenceStatusPtr GetPresenceStatus();
		ContactInfoList GetContactInfos();
	private:
		void UpdatePresence(PresenceStatusPtr p); // called by TelepathyCommunication

		std::string name_;
		PresenceStatusPtr presence_status_;
		ContactInfoList contact_infos_;
	};

	typedef boost::shared_ptr<Contact> ContactPtr;
	typedef std::list<ContactPtr> ContactList;

} // end of namespace: Communication

#endif // incl_TPContact_h