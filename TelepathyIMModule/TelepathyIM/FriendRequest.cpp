// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "FriendRequest.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    FriendRequest::FriendRequest(Tp::ContactPtr contact) : state_(STATE_PENDING), tp_contact_(contact)
    {
    }
    
    QString FriendRequest::GetOriginatorName() const
    {
        return tp_contact_->alias();
    }

    QString FriendRequest::GetOriginatorID() const
    {
        return tp_contact_->id();
    }

    Communication::FriendRequestInterface::State FriendRequest::GetState() const
    {
        return state_;
    }

    void FriendRequest::Accept()
    {
        QString message;
        message = ""; // we don't want to send any particular message at this point
        Tp::PendingOperation* p = tp_contact_->authorizePresencePublication(message);
        QObject::connect(p, SIGNAL(finished(Tp::PendingOperation*)), SLOT(OnPresencePublicationAuthorized(Tp::PendingOperation*)) );

        // Request presence subscription from friend request originator so that
        // publicity is on the same level to both directions
        if (tp_contact_->subscriptionState() == Tp::Contact::PresenceStateYes )
        {
            emit Accepted(this);
        }
        else
        {
            message = ""; // we don't want to send any particular message at this point
            Tp::PendingOperation* op = tp_contact_->requestPresenceSubscription(message);
            connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnPresenceSubscriptionResult(Tp::PendingOperation*) ) );
        }
        state_ = STATE_ACCEPTED;
    }    

    void FriendRequest::Reject()
    {
        QString message = ""; // we don't want to send any particular message at this point
        Tp::PendingOperation* p = tp_contact_->removePresencePublication(message);
        //! todo connect signal 
        state_ = STATE_REJECTED;
    }

    void FriendRequest::OnPresencePublicationAuthorized(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            LogError("Cannot publish presence for a friend contact.");
        }
    }

    void FriendRequest::OnPresenceSubscriptionResult(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            //! If the contact doesn't allow presence subscription of the presence 
            //! then we don't want to publish our.
            tp_contact_->removePresencePublication();
            emit Canceled(this);
            return;
        }
        connect(tp_contact_.data(), SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState) ), SLOT( OnPresenceSubscriptionChanged(Tp::Contact::PresenceState) ));
    }

    void FriendRequest::OnPresenceSubscriptionChanged(Tp::Contact::PresenceState state)
    {
        switch (state)
        {
        case Tp::Contact::PresenceStateYes:
            emit Accepted(this);
            break;

        case Tp::Contact::PresenceStateNo: break;
            emit Canceled(this);
            break;
        case Tp::Contact::PresenceStateAsk:
            break;
        }
    }

    Tp::ContactPtr FriendRequest::GetOriginatorTpContact()
    {
        return tp_contact_;
    }


} // end of namespace: TelepathyIM
