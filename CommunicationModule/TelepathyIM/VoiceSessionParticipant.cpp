#include "VoiceSessionParticipant.h"

namespace TelepathyIM
{
	VoiceSessionParticipant::VoiceSessionParticipant(Contact *contact): contact_(contact)
	{

	}

	VoiceSessionParticipant::~VoiceSessionParticipant()
	{

	}

	Communication::ContactInterface* VoiceSessionParticipant::GetContact() const
	{
		return contact_;
	}

	QString VoiceSessionParticipant::GetID() const
	{
		return contact_->GetID();
	}
	
	QString VoiceSessionParticipant::GetName() const
	{
		return contact_->GetName();
	}

} // end of namespace: TelepathyIM
