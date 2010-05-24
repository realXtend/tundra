// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_Connection_h
#define incl_Communication_TelepathyIM_Connection_h

#include <QStringList>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/PendingConnection>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
//#include <Foundation.h>
//#include "interface.h"
//#include "Contact.h"
#include "ContactGroup.h"
//#include "ChatSession.h"
//#include "FriendRequest.h"
//#include "OutgoingFriendRequest.h"
//#include "VoiceSession.h"
#include "ModuleLoggingFunctions.h"
#include "TelepathyIMModuleFwd.h"
#include "ConnectionInterface.h"

namespace TelepathyIM
{
    /**
     *  Do NOT use this class directly is it used by communication module
     *
     *  Connection to IM server using Telepathy framework
     *  Current implementation uses gabble connection manager and provides jabber protocol
     *
     */
    class Connection : public Communication::ConnectionInterface
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

    public:
        Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials);

        virtual ~Connection();

        //! Provides name of the connection
        virtual QString GetName() const;

        //! Connection protocol
        virtual QString GetProtocol() const;

        //! Connection state
        virtual Communication::ConnectionInterface::State GetState() const;

        //! Provides server address of this IM server connection
        virtual QString GetServer() const;

        //! @return user id associated with this connection
        virtual QString GetUserID() const;

        //! Provides textual descriptions about error
        //! If state is not STATE_ERROR then return empty
        virtual QString GetReason() const;

        //! Provides contact list associated with this IM server connection
        virtual Communication::ContactGroupInterface& GetContacts();

        //! @return list of presence status opstions contacts might have
        virtual QStringList GetPresenceStatusOptionsForContact() const;

        //! @return list of presence status options user can set
        virtual QStringList GetPresenceStatusOptionsForUser() const;

        //! Open new chat session with given contact
        //! @param contact Chat partner target
        virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const Communication::ContactInterface &contact);

        //! Open new chat session with given user
        //! @param user_id The user id of chat partner target
        virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const QString& user_id);

        //! Open new chat session to given room
        virtual Communication::ChatSessionInterface* OpenChatSession(const QString &channel);

        //! OPen a new voice chat session with given contact
        //! @param contact Voice chat partner 
        virtual Communication::VoiceSessionInterface* OpenVoiceSession(const Communication::ContactInterface &contact);

        virtual void RemoveContact(const Communication::ContactInterface &contact);

        //! Send a friend request to target address
        virtual void SendFriendRequest(const QString &target, const QString &message);

        virtual void SetPresenceStatus(const QString &status);

        //! Set presene status message of user
        //! @param message Any text is accepted
        virtual void SetPresenceMessage(const QString &message);

        //! Provides all received friend requests with in this connection session
        //! FriendRequest object state must be checked to find out new ones.
        //! If friend request is not answered the server will resend it on next 
        //! connection
        virtual Communication::FriendRequestVector GetFriendRequests() const;

        //! Get presene status state of user
        virtual QString GetPresenceStatus() { return presence_status_; };

        //! Get presene status message of user
        virtual QString GetPresenceMessage() { return presence_message_; };

        //! Check and emit all pending friend requests
        virtual void CheckPendingFriendRequests();

        //! Closes the connection
        virtual void Close();

    protected:
        virtual void CreateTpConnection(const Communication::CredentialsInterface &credentials);
        virtual ContactVector HandleAllKnownTpContacts();
        virtual Contact* GetContact(Tp::ContactPtr tp_contact);
        virtual void AddContact(Contact* contact);
        virtual void DeleteContacts();
        virtual void DeleteFriendRequests();
        virtual void CloseSessions();
        virtual void DisconnectSignals();

        Tp::ConnectionManagerPtr &tp_connection_manager_;
        Tp::ConnectionPtr tp_connection_;
        Tp::PendingReady* tp_pending_connection_;

        State state_;
        QString name_;
        QString protocol_;
        QString server_;
        QString user_id_;
        QString reason_;
        QString presence_status_;
        QString presence_message_;
        QList<FriendRequest*> pending_friend_requests_;

        ContactGroup friend_list_;
        ContactVector contacts_;
        Contact* self_contact_;
        ChatSessionVector public_chat_sessions_;
        ChatSessionVector private_chat_sessions_;
        VoiceSessionVector voice_sessions_;
        FriendRequestVector received_friend_requests_;
        OutgoingFriendRequestVector sent_friend_requests_;

    protected slots:
        virtual void OnConnectionCreated(Tp::PendingOperation *op);
        virtual void OnConnectionConnected(Tp::PendingOperation *op);
        virtual void OnNewChannels(const Tp::ChannelDetailsList& details);
        virtual void OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage);
        virtual void OnConnectionClosed(Tp::PendingOperation *op);
        virtual void OnConnectionReady(Tp::PendingOperation *op);
        virtual    void OnPresencePublicationRequested(const Tp::Contacts &contacts);
        virtual void OnTpConnectionStatusChanged(uint newStatus, uint newStatusReason);
        virtual void OnSendingFriendRequestError(OutgoingFriendRequest*);
        virtual void IncomingChatSessionReady(ChatSession* session);
        virtual void IncomingFriendRequestAccepted(FriendRequest *request);
        virtual void PresencePublicationFinished(Tp::PendingOperation* op);
        virtual void OnPresenceSubscriptionCanceled(Contact* contact);
        virtual void IncomingVoiceSessionReady(VoiceSession *session);
        virtual void OnConnectionStatusChanged(uint status, uint reason);
        virtual void OnSelfHandleChanged(uint handle) ;

        virtual void OnSendingFriendAccepted(OutgoingFriendRequest* request);
        virtual void OnSendingFriendRejected(OutgoingFriendRequest* request);

        virtual    void OnGroupMembersChanged(const QString &group,
                                           const Tp::Contacts &groupMembersAdded,
                                           const Tp::Contacts &groupMembersRemoved);
    };
    typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_Connection_h
