#ifndef incl_Comm_Participant_h
#define incl_Comm_Participant_h

#include "Foundation.h"
#include "EventDataInterface.h"

#include "PresenceStatus.h"

namespace TpQt4Communication
{
	/**
	 *  An participant is session. A Participant object is created when invited person have
	 *  accepted to join to session.
	 *
	 * You can get an Participant object by calling 
	 *
	 */
	class Participant
	{
	public:
		std::string GetRealName();
		std::string GetAddress();
	};



} // end of namespace TpQt4Communication

#endif // incl_Comm_Participant_h