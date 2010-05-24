// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_ChatSessionParticipant_h
#define incl_Communication_TelepathyIM_ChatSessionParticipant_h

//#include "interface.h"
//#include "Contact.h"
#include "TelepathyIMModuleFwd.h"
#include "ChatSessionParticipantInterface.h"

namespace TelepathyIM
{
    /**
     * A participant of one ChatSession object. A participant can be a contact or
     * just a nick on chat room
     */
    class ChatSessionParticipant : public Communication::ChatSessionParticipantInterface
    {
        Q_OBJECT
    public:
        ChatSessionParticipant(Contact *contact);
//        ChatSessionParticipant(const QString &uuid, const QString &name);
        virtual ~ChatSessionParticipant();

        //! @return contact object if the participant have one
        //          otherwise return a NULL pointer 
        virtual Communication::ContactInterface* GetContact() const;

        //! @return id of this participant 
        virtual QString GetID() const;

        //! Provides name of this participant
        virtual QString GetName() const;
    protected:
        QString name_;
        QString id_;
        Contact *contact_;
    };

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ChatSessionParticipant_h
