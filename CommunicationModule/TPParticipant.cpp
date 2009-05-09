#include "StableHeaders.h"
#include "Foundation.h"
#include "TelepathyCommunication.h"
#include "TPParticipant.h"

namespace Communication
{
	TPParticipant::TPParticipant(ContactPtr contact): contact_(contact), message_count_(0), last_message_time_stamp_("")
	{
		im_messages_ = IMMessageListPtr( new IMMessageList());
	}

	ContactPtr TPParticipant::GetContact()
	{
		return contact_;
	}

} // end of namespace: Communication