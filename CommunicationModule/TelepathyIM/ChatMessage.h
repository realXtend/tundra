#ifndef incl_Communication_TelepathyIM_ChatMessage_h
#define incl_Communication_TelepathyIM_ChatMessage_h

#include "Foundation.h"
#include "NetworkEvents.h"
#include "..\interface.h"
#include "ChatSessionParticipant.h"

namespace TelepathyIM
{
	class ChatMessage : public Communication::ChatMessageInterface
	{
	public:
		ChatMessage(ChatSessionParticipant* originator, const QTime& time_stamp, const QString &text);
		virtual ~ChatMessage() {};
		virtual Communication::ChatSessionParticipantInterface* GetOriginator() const;
		virtual QTime GetTimeStamp() const;
		virtual QString GetText() const;

	private:
		ChatSessionParticipant* originator_;
		const QTime time_stamp_;
		const QString text_;
		bool acknowledged_; //! @todo IMPLEMENT
	};
	typedef std::vector<ChatMessage*> ChatMessageVector;
	
} // end of namespace: TelepathyIM 

#endif incl_Communication_TelepathyIM_ChatMessage_h