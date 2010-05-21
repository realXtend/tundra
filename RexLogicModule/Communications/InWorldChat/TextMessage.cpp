#include "StableHeaders.h"
#include "TextMessage.h"

namespace RexLogic
{
    namespace InWorldChat
    {
        TextMessage::TextMessage(const QString& author, const QString& text, bool own_message)
        {
            author_ = author;
            text_ = text;
            time_stamp_ = QDateTime::currentDateTime();
            own_message_ = own_message;
        }

        TextMessage::~TextMessage()
        {

        }

        const QString& TextMessage::Author() const
        {
            return author_;
        }

        const QString& TextMessage::Text() const
        {
            return text_;
        }

        const QDateTime& TextMessage::TimeStamp() const
        {
            return time_stamp_;
        }

        bool TextMessage::IsOwnMessage() const
        {
            return own_message_;
        }

    } // InWorldChat

} // RexLogic
