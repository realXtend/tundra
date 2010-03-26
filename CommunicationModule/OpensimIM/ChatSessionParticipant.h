// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OpensimIM_ChatSessionParticipant_h
#define incl_Communication_OpensimIM_ChatSessionParticipant_h

//#include <Foundation.h>
//#include "interface.h"
//#include "Contact.h"

#include "CommunicationModuleFwd.h"
#include "ChatSessionParticipantInterface.h"

namespace OpensimIM
{
    class ChatSessionParticipant: public Communication::ChatSessionParticipantInterface
    {
    public:

        ChatSessionParticipant(Contact *contact);
        ChatSessionParticipant(const QString &id, const QString &name);

        //! Return NULL pointer if the participant is not on
        //! contact list
        virtual Communication::ContactInterface* GetContact() const;

        //! Provides id of this participant 
        virtual QString GetID() const;

        //! Provides name of this participant
        virtual QString GetName() const;

        virtual void SetName(const QString &name);

        //! NOT IMPLEMENTED
//        virtual bool IsTyping() const;

        //! NOT IMPLEMENTED
//        virtual bool HaveLocation() const;

        //! NOT IMPLEMENTED
//        virtual GetScneneObject() const;
    protected:
        QString name_;
        QString uuid_;
        Contact* contact_;
        //RexUUID uuid_;
    };
    typedef std::vector<ChatSessionParticipant*> ChatSessionParticipantVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ChatSessionParticipant_h
