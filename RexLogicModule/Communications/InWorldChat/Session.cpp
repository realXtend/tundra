#include "StableHeaders.h"
#include "Session.h"


namespace RexLogic
{
    namespace InWorldChat
    {
        Session::Session(const QString& self_uuid) :
            is_closed_(false),
            self_uuid_(self_uuid)
        {
        }

        Session::~Session()
        {
            ClearHistory();
        }

        void Session::ClearHistory()
        {
            foreach(TextMessage* m, messages_)
            {
                SAFE_DELETE(m);
            }
            messages_.clear();
        }

        void Session::SendTextMessage(const QString &text)
        {
            if (is_closed_)
                return;

            // We do not update message history here, we get all messages from network
            emit UserEnteredText(text);
        }

        QList<Communications::InWorldChat::TextMessageInterface*> Session::MessageHistory() const
        {
            QList<Communications::InWorldChat::TextMessageInterface*> list;
            foreach(TextMessage* m, messages_)
            {
                list.append(m);
            }
            return list;
        }

        void Session::HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text)
        {
            //TextMessage message(AvatarName(from_uuid), text, IsSelfAvatarUUID(from_uuid));
            TextMessage* message = new TextMessage(from_name, text, IsSelfAvatarUUID(from_uuid));
            messages_.append(message);
            emit Communications::InWorldChat::SessionInterface::TextMessageReceived(*message);
        }

        bool Session::IsSelfAvatarUUID(QString uuid)
        {
            if (self_uuid_ == uuid)
                return true;
            else
                return false;
        }

        QString Session::AvatarName(QString uuid)
        {
            /// @todo FETCH REAL NAME 
            return uuid;
        }

        void Session::Close()
        {
            is_closed_ = true;
            emit Closed();
        }

        bool Session::IsClosed() const
        {
            return is_closed_;
        }

        //ParticipantList Session::Participants() const
        //{
        //    ParticipantList list;
        //    /// @todo IMPLEMENT
        //    return list;
        //}

    } // InWorldChat

} // RexLogic

