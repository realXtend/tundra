// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ChatMessage.h"
#include "ChatSessionParticipant.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    ChatMessage::ChatMessage(ChatSessionParticipant* originator, const QDateTime& time_stamp, const QString &text) : originator_(originator), time_stamp_(time_stamp), text_(text)
    {

    }

    Communication::ChatSessionParticipantInterface* ChatMessage::GetOriginator() const
    {
        return originator_;
    }

    QDateTime ChatMessage::GetTimeStamp() const
    {
        return time_stamp_;
    }

    QString ChatMessage::GetText() const
    {
        return text_;
    }

} // end of namespace: TelepathyIM 
