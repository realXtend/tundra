#ifndef incl_Communication_TelepathyIM_OutgoingFriendRequest_h
#define incl_Communication_TelepathyIM_OutgoingFriendRequest_h

#include <QObject>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/PendingContacts>
//#include "interface.h"

namespace TelepathyIM
{
    /**
     *
     *
     *
     */
    class Contact;

    class OutgoingFriendRequest : public QObject
    {
        Q_OBJECT
    public:
        enum State {STATE_PENDING, STATE_SENT, STATE_ACCEPTED, STATE_REJECTED, STATE_ERROR};
        OutgoingFriendRequest(const QString &target_id, const QString &message, Tp::ConnectionPtr tp_connection);
        ~OutgoingFriendRequest();
        QString GetTargetId();
        QString GetTargetName();
        Contact* GetContact();
    protected:
        State state_;

        // Textual description about error if state_ == STATE_ERROR
        QString reason_;
        Tp::ConnectionPtr tp_connection_;
        QString message_;
        QString target_id_;
        QString target_name_;

        Tp::ContactPtr outgoing_contact_handle_;
        Contact* comm_contact_handle_;

    protected slots:
        virtual void OnContactRetrievedForFriendRequest(Tp::PendingOperation *op);
        virtual void OnPresenceSubscriptionResult(Tp::PendingOperation *op);

        virtual void OnAliasChanged(const QString &alias);
        virtual void OnAvatarTokenChanged(const QString &avatarToken);
        virtual void OnSimplePresenceChanged(const QString &status, uint type, const QString &presenceMessage);

        virtual void OnSubscriptionStateChanged(Tp::Contact::PresenceState state);
        virtual void OnPublishStateChanged(Tp::Contact::PresenceState state);
        virtual void OnBlockStatusChanged(bool blocked);

        virtual void OnAddedToGroup(const QString &group);
        virtual void OnRemovedFromGroup(const QString &group);


    signals:
        void Sent(OutgoingFriendRequest *request);
        void Error(OutgoingFriendRequest *request);
        void Accepted(OutgoingFriendRequest *request); // @todo needed?
        void Rejected(OutgoingFriendRequest *request); // @todo needed?
    };
//    typedef std::vector<OutgoingFriendRequest*> OutgoingFriendRequestVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_OutgoingFriendRequest_h