// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ConnectionInterface_h
#define incl_Comm_ConnectionInterface_h

#include <QObject>

#include "CommunicationModuleFwd.h"

namespace Communication
{
    /**
     *  A connection to IM server. This class do the most of the work. It provides 
     *  state information about connection, contact list, allow to set presence status and 
     *  provides methods to open communication sessions such as chat and voice.
     *
     *  It also signals about incoming friend request and communication sessions.
     *
     *  @todo Add methods to remove a friend from friend list
     *  @todo Add method to open voice session
     *  @todo Add method to open video session
     */
    class ConnectionInterface : public QObject
    {
        Q_OBJECT
    public:
        //! The state options of Connection object
        //! ConnectionReady and ConnectionClosed signals are emited when state
        //! changes to STATE_OPEN or STATE_CLOSED
        enum State {STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR};

        virtual ~ConnectionInterface() {};

        //! Provides name of the connection
        virtual QString GetName() const = 0;

        //! Connection protocol
        virtual QString GetProtocol() const = 0;

        //! Connection state
        virtual State GetState() const = 0;

        //! Provides server address of this IM server connection
        virtual QString GetServer() const = 0;

        //! @return user id associated with this connection
        virtual QString GetUserID() const = 0;

        //! Provides textual descriptions about error
        //! If state is not STATE_ERROR then return empty
        virtual QString GetReason() const = 0;

        //! Provides contact list associated with this IM server connection
        virtual ContactGroupInterface& GetContacts() = 0;

        //! @return list of presence status opstions contacts might have
        virtual QStringList GetPresenceStatusOptionsForContact() const = 0;

        //! @return list of presence status options user can set
        virtual QStringList GetPresenceStatusOptionsForUser() const = 0;

        //! Open new chat session with given contact
        //! @param contact Chat partner target
        virtual ChatSessionInterface* OpenPrivateChatSession(const ContactInterface &contact) = 0;

        //! Open new chat session with given user
        //! @param user_id The user id of chat partner target
        virtual ChatSessionInterface* OpenPrivateChatSession(const QString& user_id) = 0;

        //! Open new chat session to given room
        virtual ChatSessionInterface* OpenChatSession(const QString &channel) = 0;

        //! OPen a new voice chat session with given contact
        //! @param contact Voice chat partner 
        virtual VoiceSessionInterface* OpenVoiceSession(const ContactInterface &contact) = 0;

        //! Send a friend request to target address
        virtual void SendFriendRequest(const QString &target, const QString &message) = 0;

        //! Provides all received friend requests with in this connection session
        //! FriendRequest object state must be checked to find out new ones.
        //! If friend request is not answered the server will resend it on next 
        //! connection
        virtual FriendRequestVector GetFriendRequests() const = 0;

        //! Removed given contact from friend list 
        virtual void RemoveContact(const Communication::ContactInterface &contact) = 0;

        //! Set presence status state of user
        //! @param status Allowed values are returned by GetPresenceStatusOptionsForUser methos
        virtual void SetPresenceStatus(const QString &status) = 0;

        //! Set presene status message of user
        //! @param message Any text is accepted
        virtual void SetPresenceMessage(const QString &message) = 0;

        //! Get presene status state of user
        virtual QString GetPresenceStatus() = 0;

        //! Get presene status message of user
        virtual QString GetPresenceMessage() = 0;

        //! Check and emit all pending friend requests
        virtual void CheckPendingFriendRequests() = 0;

        //! Closes the connection
        virtual void Close() = 0;

    signals:
        //! When connection become state where communication sessions can 
        //! be opened and contact list is fethed from server.
        void ConnectionReady(Communication::ConnectionInterface& connection);

        //! When connection is closed by user or server
        void ConnectionClosed(Communication::ConnectionInterface& connection);

        //! When connection state become error
        void ConnectionError(Communication::ConnectionInterface& connection);

        //! When a chat session is initialized by IM server
        void ChatSessionReceived(Communication::ChatSessionInterface& session);

        //! When a voice session is initialized by IM server
        void VoiceSessionReceived(Communication::VoiceSessionInterface& session);

        //! When a new contact is added to contact list
        //! Basically this happens when someone accept friend request
        void NewContact(const Communication::ContactInterface& contact);

        //! When contact on contact list removes user from his/her contact list
        //! then that contact will be automatically removed from user's contact list
        void ContactRemoved(const Communication::ContactInterface& contact);

        //! When contact status changes
        void ContactStatusUpdated(const ContactInterface& contact);

        //! When a friend request is received from IM server
        void FriendRequestReceived(Communication::FriendRequestInterface& request); 

        //! When target have accepted the friend request
        void FriendRequestAccepted(const QString &target); 

        //! If the protocol doesn't support this then no
        //! notification is send back about rejecting the friend request
        void FriendRequestRejected(const QString &target); 
    };

}

#endif

