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
            TextMessage message("", text);
            messages_.append(message);

            emit UserEnteredText(text);
        }

        void Session::HandleIncomingTextMessage(const QString& from_uuid, const QString& from_name, const QString& text)
        {
            TextMessage message(from_name, text);
            messages_.append(message);
            emit Communications::InWorldChat::SessionInterface::TextMessageReceived(message);
        }

        //ParticipantList Session::Participants() const
        //{
        //    ParticipantList list;
        //    /// @todo IMPLEMENT
        //    return list;
        //}

    } // InWorldChat

} // RexLogic

