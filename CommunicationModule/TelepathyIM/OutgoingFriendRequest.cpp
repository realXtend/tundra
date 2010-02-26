// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "OutgoingFriendRequest.h"
#include "Contact.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    OutgoingFriendRequest::OutgoingFriendRequest(const QString &target_id, const QString &message, Tp::ConnectionPtr tp_connection) : state_(STATE_PENDING), tp_connection_(tp_connection), message_(message)
    {
        target_id_ = target_id;
        Tp::PendingContacts *pending_contacts = tp_connection_->contactManager()->contactsForIdentifiers(QStringList() << target_id);
        connect(pending_contacts,
                SIGNAL( finished(Tp::PendingOperation *) ),
                SLOT( OnContactRetrievedForFriendRequest(Tp::PendingOperation *) ));
    }

    OutgoingFriendRequest::~OutgoingFriendRequest()
    {
        if(comm_contact_handle_!=NULL)
        {
            delete comm_contact_handle_;
            comm_contact_handle_ = NULL;
        }
    }

    QString OutgoingFriendRequest::GetTargetId()
    {
        return target_id_;
    }

    QString OutgoingFriendRequest::GetTargetName()
    {
        return target_name_;
    }

    Contact* OutgoingFriendRequest::GetContact()
    {
        return comm_contact_handle_;
        //return outgoing_contact_handle_;
    }

    void OutgoingFriendRequest::OnContactRetrievedForFriendRequest(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            reason_ = op->errorMessage();
            emit( Error(this) );
            return;
        }
        Tp::PendingContacts *pending_contacts = qobject_cast<Tp::PendingContacts *>(op);
        QList<Tp::ContactPtr> contacts = pending_contacts->contacts();
        
        assert( contacts.size() == 1); // We have request only one contact 
        Tp::ContactPtr contact = contacts.first();
        target_name_ = contact->alias();
        //Tp::ContactPtr cPtr = contacts.first();
        //cPtr.data;
        //connect(tp_connection_->contactManager(), SIGNAL( presencePublicationRequested(const Tp::Contacts &) ), SLOT( OnPresencePublicationRequested(const Tp::Contacts &) ));

        outgoing_contact_handle_ = contacts.first();

        //contacts[0]->addedToGroup


        // Do the presence subscription
        Tp::PendingOperation* p = contact->requestPresenceSubscription(message_);
        connect(p, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnPresenceSubscriptionResult(Tp::PendingOperation*) ));
    }

    void OutgoingFriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation *op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            reason_ = op->errorMessage();
            emit( Error(this) );
            return;
        }
        state_ = STATE_SENT;
        emit( Sent(this) );

        connect(outgoing_contact_handle_.data(), SIGNAL( aliasChanged(const QString &)), SLOT( OnAliasChanged(const QString &)));
        connect(outgoing_contact_handle_.data(), SIGNAL( avatarTokenChanged(const QString &)), SLOT( OnAvatarTokenChanged(const QString &)));
        connect(outgoing_contact_handle_.data(), SIGNAL( simplePresenceChanged(const QString &, uint, const QString &)), SLOT( OnSimplePresenceChanged(const QString &, uint, const QString &)));

        connect(outgoing_contact_handle_.data(), SIGNAL( subscriptionStateChanged(Tp::Contact::PresenceState)), SLOT( OnSubscriptionStateChanged(Tp::Contact::PresenceState)));

        connect(outgoing_contact_handle_.data(), SIGNAL( publishStateChanged(Tp::Contact::PresenceState)), SLOT( OnPublishStateChanged(Tp::Contact::PresenceState)));
        connect(outgoing_contact_handle_.data(), SIGNAL( blockStatusChanged(bool)), SLOT( OnBlockStatusChanged(bool)));
        connect(outgoing_contact_handle_.data(), SIGNAL( addedToGroup(const QString &)), SLOT( OnAddedToGroup(const QString &)));
        connect(outgoing_contact_handle_.data(), SIGNAL( removedFromGroup(const QString &)), SLOT( OnRemovedFromGroup(const QString &)));

    }


    void OutgoingFriendRequest::OnAliasChanged(const QString &alias)
    {
    }
    void OutgoingFriendRequest::OnAvatarTokenChanged(const QString &avatarToken)
    {
    }
    void OutgoingFriendRequest::OnSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage)
    {
    }

    void OutgoingFriendRequest::OnSubscriptionStateChanged(Tp::Contact::PresenceState state)
    {
    }
    void OutgoingFriendRequest::OnPublishStateChanged(Tp::Contact::PresenceState state)
    {
        comm_contact_handle_ = new Contact(outgoing_contact_handle_);
        switch (state) 
        {
        case Tp::Contact::PresenceStateYes:
            emit Accepted(this);
            break;
        case Tp::Contact::PresenceStateNo:
            emit Rejected(this);
            break;
        }
    }
    void OutgoingFriendRequest::OnBlockStatusChanged(bool blocked)
    {
    }

    void OutgoingFriendRequest::OnAddedToGroup(const QString &group)
    {
    }
    void OutgoingFriendRequest::OnRemovedFromGroup(const QString &group)
    {
    }

} // end of namespace: TelepathyIM
