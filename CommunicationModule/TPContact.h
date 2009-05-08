#ifndef incl_TPContact_h
#define incl_TPContact_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPPresenceStatus.h"

namespace Communication
{
	class TPContact
	{
		friend class TelepathyCommunication;
	public:
		void SetName(std::string name);
		std::string GetName();
		PresenceStatusPtr GetPresenceStatus();
		ContactInfoList GetContactInfoList();
	protected:
//		void UpdatePresence(PresenceStatusPtr p); // called by TelepathyCommunication

		std::string name_;
		PresenceStatusPtr presence_status_;
		ContactInfoList contact_infos_;
	};

//	typedef boost::shared_ptr<TPContact> TPContactPtr;
//	typedef std::vector<TPContactPtr> TPContactList;

	//class TPContactList
	//{
	//public:
	//protected:
	//};

} // end of namespace: Communication

#endif // incl_TPContact_h