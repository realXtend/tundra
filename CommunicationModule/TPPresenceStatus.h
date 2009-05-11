#ifndef incl_TPPresenceStatus_h
#define incl_TPPresenceStatus_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	class TPPresenceStatus: PresenceStatusInterface
	{
		friend class TelepathyCommunication;
	public:
		TPPresenceStatus();
		void SetOnlineStatus(std::string status);
		std::string GetOnlineStatus();
		void SetOnlineMessage(std::string message);
		std::string GetOnlineMessage();
		std::vector<std::string> GetOnlineStatusOptions();
	protected:
		void NotifyUpdate(std::string online_status, std::string online_message);
		void UpdateToServer();

		std::string id_; // contact id on python side -> do we need it here ?
		std::string online_status_;
		std::string online_message_;
		std::vector<std::string> online_status_options_;
	};
}

#endif // incl_TPPresenceStatus_h