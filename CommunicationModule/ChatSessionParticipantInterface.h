// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ChatSessionParticipantInterface_h
#define incl_Comm_ChatSessionParticipantInterface_h

#include <QObject>
#include <QString>

namespace Communication
{
    class ContactInterface;
    /**
     *  A participant of one ChatSession object. A participant can be a contact or
     *  just a nick on chat room.
     *
     *  @todo Add methods to get related scene object if that exist. So that 
     *        3D in-world chat UI can be made.
     */
    class ChatSessionParticipantInterface : public QObject
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
    signals:
    };

}

#endif

