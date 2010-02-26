// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "ContactGroup.h"

#include "MemoryLeakCheck.h"

namespace OpensimIM
{
    ContactGroup::ContactGroup() : name_("Friend list")
    {
    }

    QString ContactGroup::GetName() const
    {
        return name_;
    }

    void ContactGroup::SetName(const QString &name)
    {
        //! @note Opensim doesn't support naming of contact lists
        //!       so name change applies only for this session
        name_ = name;
    }

    Communication::ContactVector ContactGroup::GetContacts()
    {
        Communication::ContactVector contacts;
        for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
        {
            contacts.push_back(*i);
        }
        return contacts;
    }

    Communication::ContactGroupVector ContactGroup::GetGroups()
    {
        Communication::ContactGroupVector groups;
        for (ContactGroupVector::iterator i = groups_.begin(); i != groups_.end(); ++i)
        {
            groups.push_back(*i);
        }
        return groups;
    }

    void ContactGroup::AddContact(Contact* contact)
    {
        contacts_.push_back(contact);
    }

} // end of namespace: OpensimIM
