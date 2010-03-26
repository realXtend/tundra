// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <QSet>
#include "Contact.h"

#include "MemoryLeakCheck.h"


namespace TelepathyIM
{

    Contact::Contact(Tp::ContactPtr tp_contact): tp_contact_(tp_contact)
    {
        if (tp_contact.isNull())
            return;
        id_ = tp_contact->id();
        name_ = tp_contact->alias();
        QString avatar = tp_contact->avatarToken();  // todo: use this information

        connect(tp_contact_.data(),
                SIGNAL( simplePresenceChanged(const QString &, uint, const QString &) ),
                SLOT( OnSimplePresenceChanged(const QString &, uint, const QString &) ));
        connect(tp_contact_.data(),
                SIGNAL( subscriptionStateChanged(Tp::Contact::PresenceState) ),
                SLOT( OnContactChanged() ));
        connect(tp_contact_.data(),
                SIGNAL( publishStateChanged(Tp::Contact::PresenceState) ),
                SLOT( OnContactChanged() ));
        connect(tp_contact_.data(),
                SIGNAL( blockStatusChanged(bool) ),
                SLOT( OnContactChanged()) );
    }

    Contact::~Contact()
    {
        // todo: disconnect signals?
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
        if (!tp_contact_)
            return;
        
        //! @todo IMPLEMENT
        //! This feature have not been implemented on telepathy-qt4 yet
    }

    QString Contact::GetPresenceStatus() const
    {
        return tp_contact_->presenceStatus();
    }

    QString Contact::GetPresenceMessage() const
    {
        return tp_contact_->presenceMessage();
    }

    void Contact::OnSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage)
    {
        emit( PresenceStatusChanged(GetPresenceStatus(), GetPresenceMessage()) ); 
    }

    Tp::ContactPtr Contact::GetTpContact() const
    {
        return tp_contact_;
    }

    void Contact::OnContactChanged()
    {
        if (tp_contact_->subscriptionState() == Tp::Contact::PresenceStateNo)
        {
            // This contact doesn't allow to subscribe the presence status
            emit PresenceSubscriptionCanceled(this);
        }
    }



} // end of namespace: TelepathyIM
    