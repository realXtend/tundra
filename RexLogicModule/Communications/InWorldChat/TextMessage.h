// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_Communications_TextMessage_h
#define incl_RexLogic_Communications_TextMessage_h

#include "CommunicationsService.h"

namespace RexLogic
{
    namespace InWorldChat
    {
        /// Text message in OpenSim world chat
        ///
        class TextMessage : public Communications::InWorldChat::TextMessageInterface
        {
        public:
            TextMessage(const QString& author, const QString& text);
            virtual ~TextMessage();
            virtual const QString& Author() const;
            virtual const QString& Text() const;
            virtual const QDateTime& TimeStamp() const;
//            virtual QList<QByteArray> Attachments() const;
//            virtual const ParticipantInterface& AuthorParticipant() const;
        private:
            QString author_;
            QString text_;
            QDateTime time_stamp_;
        };

    } // InWorldChat

} // RexLogic


#endif // incl_RexLogic_Communications_TextMessage_h