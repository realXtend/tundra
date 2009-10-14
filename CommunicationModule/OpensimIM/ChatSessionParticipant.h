#ifndef incl_Communication_OpensimIM_ChatSessionParticipant_h
#define incl_Communication_OpensimIM_ChatSessionParticipant_h

#include "Foundation.h"
#include "..\interface.h"

namespace OpensimIM
{
	class ChatSessionParticipant: public Communication::ChatSessionParticipantInterface
	{
	public:

		ChatSessionParticipant(const QString &id, const QString &name);

		//! Return NULL pointer if the participant is not on
		//! contact list
		virtual Communication::ContactInterface* GetContact() const;

		//! Provides id of this participant 
		virtual QString GetID() const;

		//! Provides name of this participant
		virtual QString GetName() const;

		//! NOT IMPLEMENTED
//		virtual bool IsTyping() const;

		//! NOT IMPLEMENTED
//		virtual bool HaveLocation() const;

		//! NOT IMPLEMENTED
//		virtual GetScneneObject() const;
	protected:
		QString name_;
		QString uuid_;
		//RexTypes::RexUUID uuid_;
	};
	typedef std::vector<ChatSessionParticipant*> ChatSessionParticipantVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ChatSessionParticipant_h
