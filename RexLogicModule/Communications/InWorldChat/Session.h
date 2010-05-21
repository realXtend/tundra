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
            virtual QList<Communications::InWorldChat::TextMessageInterface*> MessageHistory();

            virtual void HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text);
        signals:
            void UserEnteredText(const QString& text);
//            void TextMessageReceived(const Communications::InWorldChat::TextMessageInterface &message);
            //void ParticipantJoined(ParticipantInterface* participant);
            //void ParticipantLeft(ParticipantInterface* participant);
        private:
            bool IsSelfAvatarUUID(QString uuid);
            QString AvatarName(QString uuid);

            QList<TextMessage> messages_;
        };

    } // InWorldChat

} // RexLogic


#endif // incl_RexLogic_Communications_Session_h
