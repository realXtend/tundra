#ifndef incl_Communication_TelepathyIM_Contact_h
#define incl_Communication_TelepathyIM_Contact_h

#include <TelepathyQt4/Connection>
#include <TelepathyQt4/Contact>
#include <Foundation.h>
//#include "interface.h"
#include "ContactInterface.h"

namespace TelepathyIM
{
    /**
     *  Do NOT use this class directly. Only classes in TelepathyIM namespace
     *  will use this.
     *
     *  A contact item on ContactList object. Presents a real life person.
     *
     */
    class Contact : public Communication::ContactInterface 
    {
        Q_OBJECT
    public:
        Contact(Tp::ContactPtr tp_contact);
        virtual ~Contact();

        //! Provides ID of the contact. This can be eg. "myjabberid@myjabberserver"
        virtual QString GetID() const;

        //! Provides name of this contact item
        virtual    QString GetName() const;

        //! Define name of this contact item
        //! If the protocol doesn't support redefinition of the name then 
        //! this method has affect only on current IM server connection and 
        //! the name is reseted when new connection is established.
        virtual void SetName(const QString& name); 

        //! Provides presence status of this contact item. 
        //! eg. "chat", "offline", "free"
        virtual QString GetPresenceStatus() const;

        //! Provides presence message of this contact item.
        //! This is freely textual information about current state eg. "At lunch, be back in 30 minutes..." 
        virtual QString GetPresenceMessage() const;

        //! Used by ChatSession class
        virtual Tp::ContactPtr GetTpContact() const;
    protected:
        Tp::ContactPtr tp_contact_;
        QString id_;
        QString name_;
    protected slots:
        void OnSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage);
        void OnContactChanged();
    signals:
        void PresenceSubscriptionCanceled(Contact* contact);

    };
//    typedef std::vector<Contact*> ContactVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_Contact_h
