// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ChatSessionParticipant.h"
#include "Contact.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    //ChatSessionParticipant::ChatSessionParticipant(const QString &id, const QString &name): id_(id), name_(name)
    //{

    //}

    ChatSessionParticipant::ChatSessionParticipant(Contact *contact): contact_(contact)
    {

    }

    ChatSessionParticipant::~ChatSessionParticipant()
    {

    }

    Communication::ContactInterface* ChatSessionParticipant::GetContact() const
    {
        return contact_;
    }

    QString ChatSessionParticipant::GetID() const
    {
        return contact_->GetID();
    }
    
    QString ChatSessionParticipant::GetName() const
    {
        return contact_->GetName();
    }

} // end of namespace: TelepathyIM
