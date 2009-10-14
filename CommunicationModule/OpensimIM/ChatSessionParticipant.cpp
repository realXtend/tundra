#include "ChatSessionParticipant.h"
#include "RexLogicModule.h" 

namespace OpensimIM
{
	ChatSessionParticipant::ChatSessionParticipant(const QString &uuid, const QString &name): uuid_(uuid), name_(name)
	{

	}

	Communication::ContactInterface* ChatSessionParticipant::GetContact() const
	{
		//! \todo implement
		return NULL;
	}

	QString ChatSessionParticipant::GetID() const
	{
		return uuid_;
	}
	
	QString ChatSessionParticipant::GetName() const
	{
		return name_;
	}

} // end of namespace: OpensimIM
