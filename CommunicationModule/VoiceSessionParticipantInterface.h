// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_Interface_h
#define incl_Comm_Interface_h

#include <QObject>

namespace Communication
{
    class ContactInterface;

    /**
     * @todo Design issue: Do we need two separaed participant classes?
     *
     */
    class VoiceSessionParticipantInterface : public QObject
    {
        Q_OBJECT
    public:
        //! @return contact object if the participant have one
        //          otherwise return a NULL pointer 
        virtual ContactInterface* GetContact() const = 0;

        //! @return id of this participant 
        virtual QString GetID() const = 0;

        //! Provides name of this participant
        virtual QString GetName() const = 0;

    };

}

#endif

