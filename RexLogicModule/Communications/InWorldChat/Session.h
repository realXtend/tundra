// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_Communications_Session_h
#define incl_RexLogic_Communications_Session_h

#include "CommunicationsService.h"
#include "TextMessage.h"

namespace RexLogic
{
    namespace InWorldChat
    {
        class Session : public Communications::InWorldChat::SessionInterface
        {
            Q_OBJECT
        public:
            Session();
            virtual ~Session();
            virtual void SendTextMessage(const QString &text);
//            virtual ParticipantList Participants() const;

            virtual void HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text);

        signals:
            void TextMessageReceived(const Communications::InWorldChat::TextMessageInterface &message);
            //void ParticipantJoined(ParticipantInterface* participant);
            //void ParticipantLeft(ParticipantInterface* participant);
        private:
            QList<TextMessage> messages_;
        };

    } // InWorldChat

} // RexLogic


#endif // incl_RexLogic_Communications_Session_h
