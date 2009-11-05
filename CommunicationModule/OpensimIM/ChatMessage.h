#ifndef incl_Communication_OpensimIM_ChatMessage_h
#define incl_Communication_OpensimIM_ChatMessage_h

#include "Foundation.h"
#include "NetworkEvents.h"
#include "../interface.h"
#include "ChatSessionParticipant.h"


namespace OpensimIM
{

	class ChatMessage : public Communication::ChatMessageInterface
	{
	public:
		ChatMessage(ChatSessionParticipant* originator, const QDateTime& time_stamp, const QString &text);
		virtual ~ChatMessage() {};
		virtual Communication::ChatSessionParticipantInterface* GetOriginator() const;
		virtual QDateTime GetTimeStamp() const;
		virtual QString GetText() const;

	private:
		ChatSessionParticipant* originator_;
		const QDateTime time_stamp_;
		const QString text_;
	};
	typedef std::vector<ChatMessage*> ChatMessageVector;
	
} // end of namespace: OpensimIM 

#endif // incl_Communication_OpensimIM_ChatMessage_h