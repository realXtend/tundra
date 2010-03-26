// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ContactInterface_h
#define incl_Comm_ContactInterface_h

#include "CommunicationModuleFwd.h"

#include <QObject>
#include <QString>

namespace Communication
{
    /**
     * A contact item on ContactList object. Presents a real life person.
     *
     */
    class ContactInterface : public QObject
    {
        Q_OBJECT
    public:
        //! Provides ID of the contact. This can be eg. "myjabberid@myjabberserver"
        virtual QString GetID() const = 0;

        //! Provides name of this contact item
        virtual    QString GetName() const = 0;

        //! Define name of this contact item
        //! If the protocol doesn't support redefinition of the name then 
        //! this method has affect only on current IM server connection and 
        //! the name is reseted when new connection is established.
        virtual void SetName(const QString& name) = 0; 

        //! Provides presence status of this contact item. 
        //! eg. "chat", "offline", "free"
        virtual QString GetPresenceStatus() const = 0;

        //! Provides presence message of this contact item.
        //! This is freely textual information about current state eg. "At lunch, be back in 30 minutes..." 
        virtual QString GetPresenceMessage() const = 0;

    signals:
        void PresenceStatusChanged(const QString &status, const QString &message);

        //! When contact item has canceled friendship eg. when removed user from (s)hes contact list
        void FriendshipEnded();
    };
//    typedef std::vector<ContactInterface*> ContactVector;

    /**
     *  List of Contact objects eg. a friend list.
     *  Contact groups can be include Contact and ContactGroup objects.
     *  ContactGroup object is created by Connection object with information from IM server.
     *
     *  Adding or removing items is not allowed. But you can Send a friend request from Connection object
     *  and remove contact from Connection object.
     *  
     */
//    class ContactGroupInterface;
//    typedef std::vector<ContactGroupInterface*> ContactGroupVector;
    class ContactGroupInterface
    {
    public:
        //! Provides name of this contact group
        virtual QString GetName() const = 0;

        //! Set name for this contact group
        //! If the protocol supports then the given name is saved on server side.
        virtual void SetName(const QString &name) = 0;

        //! Provides all Contact objects on this contact group
        virtual ContactVector GetContacts() = 0;

        //! Priovides all sub groups of this contact groups
        virtual ContactGroupVector GetGroups() = 0;
    signals:
    };
}

#endif

