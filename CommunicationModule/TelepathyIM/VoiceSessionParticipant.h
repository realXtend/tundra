// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_VoiceSessionParticipant_h
#define incl_Communication_TelepathyIM_VoiceSessionParticipant_h

//#include "interface.h"
#include "CommunicationModuleFwd.h"
#include "VoiceSessionParticipantInterface.h"

namespace TelepathyIM
{
    /**
     * A participant of one VoiceSession object.
     *
     */
    class VoiceSessionParticipant : public Communication::VoiceSessionParticipantInterface
    {
        Q_OBJECT
    public:
        explicit VoiceSessionParticipant(Contact *contact);
        virtual ~VoiceSessionParticipant();

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
    typedef std::vector<VoiceSessionParticipant*> VoiceSessionParticipantVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_VoiceSessionParticipant_h