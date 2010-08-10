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
            Session(const QString& self_uuid);
            virtual ~Session();
            virtual void SendTextMessage(const QString &text);
//            virtual ParticipantList Participants() const;
            virtual QList<Communications::InWorldChat::TextMessageInterface*> MessageHistory() const;

            virtual bool IsClosed() const;

            /// Update message hinstory and emit 'TextMessageReceived' signal
            virtual void HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text);

            /// Change session status to close and emits 'Closed' signal
            virtual void Close();

        signals:
            void UserEnteredText(const QString& text);

        private:
            virtual bool IsSelfAvatarUUID(QString uuid);
            virtual QString AvatarName(QString uuid);
            virtual void ClearHistory();

            bool is_closed_;
            QString self_uuid_;

            QList<TextMessage*> messages_;
        };

    } // InWorldChat

} // RexLogic


#endif // incl_RexLogic_Communications_Session_h
