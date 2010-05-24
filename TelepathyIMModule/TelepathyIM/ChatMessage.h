// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_ChatMessage_h
#define incl_Communication_TelepathyIM_ChatMessage_h

//#include "Foundation.h"
//#include "NetworkEvents.h"
//#include "interface.h"
//#include "ChatSessionParticipant.h"
#include "TelepathyIMModuleFwd.h"
#include "ChatMessageInterface.h"

namespace TelepathyIM
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
        bool acknowledged_; //! @todo IMPLEMENT
    };    
} // end of namespace: TelepathyIM 

#endif // incl_Communication_TelepathyIM_ChatMessage_h