// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "VoiceSessionParticipant.h"
#include "Contact.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    VoiceSessionParticipant::VoiceSessionParticipant(Contact *contact): contact_(contact)
    {

    }

    VoiceSessionParticipant::~VoiceSessionParticipant()
    {

    }

    Communication::ContactInterface* VoiceSessionParticipant::GetContact() const
    {
        return contact_;
    }

    QString VoiceSessionParticipant::GetID() const
    {
        return contact_->GetID();
    }
    
    QString VoiceSessionParticipant::GetName() const
    {
        return contact_->GetName();
    }

} // end of namespace: TelepathyIM
