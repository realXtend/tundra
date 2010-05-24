// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ChatMessageInterface_h
#define incl_Comm_ChatMessageInterface_h

#include <QDateTime>
#include <QString>

namespace Communication
{
    class ChatSessionParticipantInterface;
    /**
     *  A message in chat session.
     *  @note This interface is not currently used!
     *  @todo GetAttachment() method for supporting file attachement feature in future
     */
    class ChatMessageInterface
    {
    public:
        virtual ChatSessionParticipantInterface* GetOriginator() const = 0;
        virtual QDateTime GetTimeStamp() const = 0;
        virtual QString GetText() const = 0;
    };
}

#endif

