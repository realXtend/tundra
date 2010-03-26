// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Contact.h"

#include "MemoryLeakCheck.h"

namespace OpensimIM
{

    Contact::Contact(const QString &id, const QString &name): id_(id), name_(name), online_(false), is_typing_(false)
    {
        
    }

    QString Contact::GetID() const
    {
        return id_;
    }

    QString Contact::GetName() const
    {
        return name_;
    }

    void Contact::SetName(const QString& name)
    {
        // NOT SUPPORTED BY OPENSIM
    }

    QString Contact::GetPresenceStatus() const
    {
        if (online_)
            return "online";
        else
            return "offline";
    }

    QString Contact::GetPresenceMessage() const
    {
        // NOT SUPPORTED BY OPENSIM
        return "";
    }

    void Contact::SetOnlineStatus(bool online)
    {
        online_ = online;
    }

} // end of namespace: OpensimIM
