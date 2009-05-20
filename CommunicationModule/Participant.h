#ifndef incl_Participant_h
#define incl_Participant_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "PresenceStatus.h"

namespace Communication
{
	/**
	 *  Implementation of ParticipantInterface
	 */
	class Participant : public ParticipantInterface
	{
	public:
		Participant(ContactPtr contact);
		virtual ContactPtr GetContact() const;
//		virtual ContactInfoPtr GetCurrentContactInfo() const;
	protected:
		ContactPtr contact_;
//		ParticipantPrt current_contact_info_; 
		IMMessageListPtr im_messages_;
		std::string last_message_time_stamp_;
		int message_count_;
	};

} // end of namespace Communication

#endif // incl_Participant_h