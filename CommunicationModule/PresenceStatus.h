#ifndef incl_PresenceStatus_h
#define incl_PresenceStatus_h

#include "Foundation.h"
#include "EventDataInterface.h"

namespace Communication
{
	/**
	 *  Implementation of PresenceStatusInterface
	 */
	class PresenceStatus: PresenceStatusInterface
	{
		friend class CommunicationManager;
	public:
		PresenceStatus();
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
		static std::vector<std::string> online_status_options_;
	};
}

#endif // incl_PresenceStatus_h