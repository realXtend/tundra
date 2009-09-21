#ifndef incl_Comm_PresenceStatus_h
#define incl_Comm_PresenceStatus_h

#include "Foundation.h"
//#include "EventDataInterface.h"

namespace TpQt4Communication
{


	/**
	 *  Presence status of contact in contact list or current user 
	 *
	 */
	class PresenceStatus
	{
	public:
		PresenceStatus();

		void SetStatusText(std::string text);
		std::string GetStatusText();
		void SetMessageText(std::string text);
		std::string GetMessageText();
		static std::vector<std::string> GetAllowedStatuses();
	private:
		std::string status_text_;
		std::string message_text_;
	};
	typedef boost::weak_ptr<PresenceStatus> PresenceStatusWeakPtr;

}

#endif // incl_Comm_PresenceStatus_h