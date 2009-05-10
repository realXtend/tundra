#ifndef incl_TPParticipant_h
#define incl_TPParticipant_h

#include "Foundation.h"
#include "EventDataInterface.h"
#include "TPPresenceStatus.h"

namespace Communication
{
	class TPParticipant : public ParticipantInterface
	{
	public:
		TPParticipant(ContactPtr contact);
		virtual ContactPtr GetContact();
	protected:
		ContactPtr contact_;
		IMMessageListPtr im_messages_;
		std::string last_message_time_stamp_;
		int message_count_;
	};

} // end of namespace Communication

#endif // incl_TPParticipant_h