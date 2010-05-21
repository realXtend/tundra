#include "StableHeaders.h"
#include "Session.h"


namespace RexLogic
{
    namespace InWorldChat
    {
        Session::Session()
        {

        }

        Session::~Session()
        {

        }

        void Session::SendTextMessage(const QString &text)
        {
            //TextMessage message("", text, true);
            //messages_.append(message);

            emit UserEnteredText(text);
        }

        QList<Communications::InWorldChat::TextMessageInterface*> Session::MessageHistory()
        {
            QList<Communications::InWorldChat::TextMessageInterface*> list;
            /// @todo: IMPLEMENT
            return list;
        }

        void Session::HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text)
        {
            TextMessage message(AvatarName(from_uuid), text, IsSelfAvatarUUID(from_uuid));
            messages_.append(message);
            emit Communications::InWorldChat::SessionInterface::TextMessageReceived(message);
        }

        bool Session::IsSelfAvatarUUID(QString uuid)
        {
            /// @todo CHECK
            return false;
        }

        QString Session::AvatarName(QString uuid)
        {
            /// @todo FETCH REAL NAME 
            return uuid;
        }

        //ParticipantList Session::Participants() const
        //{
        //    ParticipantList list;
        //    /// @todo IMPLEMENT
        //    return list;
        //}

    } // InWorldChat

} // RexLogic

