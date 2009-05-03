#ifndef incl_TPPresenceStatus_h
#define incl_TPPresenceStatus_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPPresenceStatus : PresenceStatus
	{
	public:
		TPPresence(void);
		~TPPresence(void);
	public:
		virtual void SetOnlineStatus(bool status) = 0;
		virtual bool GetOnlineStatus() = 0;
		virtual void SetOnlineMessage(std::string message) = 0;
		virtual std::string GetOnlineMessage() = 0;
	protected:
		bool online_status_;
		std::string online_message_
	};

} // end of namespace: Communication

#endif // incl_TPPresenceStatus_h