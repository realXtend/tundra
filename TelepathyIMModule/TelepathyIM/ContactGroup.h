// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_ContactGroup_h
#define incl_Communication_TelepathyIM_ContactGroup_h

#include <Foundation.h>
//#include "interface.h"
#include "Contact.h"

namespace TelepathyIM
{
    class ContactGroup;
    typedef std::vector<ContactGroup*> ContactGroupVector;

    /**
     *  Do NOT use this class directly. Only classes in TelepathyIM namespace
     *  will use this.
     */
    class ContactGroup : public Communication::ContactGroupInterface
    {
    public:
        ContactGroup(const QString &name);

        virtual ~ContactGroup();

        //! Provides name of this contact group
        virtual QString GetName() const;

        //! Set name for this contact group
        //! If the protocol supports then the given name is saved on server side.
        virtual void SetName(const QString &name);

        //! Provides all Contact objects on this contact group
        virtual Communication::ContactVector GetContacts();

        //! Priovides all sub groups of this contact groups
        virtual Communication::ContactGroupVector GetGroups();

        virtual void AddContact(Contact* contact);

        virtual void RemoveContact(const Contact* contact);
    protected:
        QString name_;
        ContactVector contacts_;
        ContactGroupVector groups_;
    };

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ContactGroup_h
