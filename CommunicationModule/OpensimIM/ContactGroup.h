// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OpensimIM_ContactGroup_h
#define incl_Communication_OpensimIM_ContactGroup_h

//#include <Foundation.h>
//#include "interface.h"
#include "Contact.h"

#include "CommunicationModuleFwd.h"

namespace OpensimIM
{
    class ContactGroup;
    typedef std::vector<ContactGroup*> ContactGroupVector;

    class ContactGroup : public Communication::ContactGroupInterface
    {
    public:
        ContactGroup();
        virtual ~ContactGroup() {};

        //! Provides name of this contact group
        virtual QString GetName() const ;

        //! Set name for this contact group
        //! If the protocol supports then the given name is saved on server side.
        virtual void SetName(const QString &name);

        //! Provides all Contact objects on this contact group
        virtual Communication::ContactVector GetContacts();

        //! Priovides all sub groups of this contact groups
        virtual Communication::ContactGroupVector GetGroups();

        //! Used by Connection object to fill contact group
        virtual void AddContact(Contact* contact);
    protected:
        QString name_;
        ContactVector contacts_;
        ContactGroupVector groups_;
    };

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_ContactGroup_h
    