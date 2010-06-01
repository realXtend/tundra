// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OSIMConnection_h
#define incl_Communication_OSIMConnection_h

//#include <Foundation.h>
#include <QStringList>
//#include "interface.h"
#include "ChatSession.h"
#include "Contact.h"
#include "FriendRequest.h"
#include "ContactGroup.h"
#include "ModuleLoggingFunctions.h"

//#include "NetworkMessages/NetInMessage.h"

#include "TelepathyIMModuleFwd.h"
#include "ConnectionInterface.h"

namespace Foundation
{
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class NetInMessage;
}

namespace OpensimIM
{

    /**
     *  A connection presentation to Opensim based world server.
     *  The actual udp connections is already established when connecting to wrold
     *  this class just encapsultates the IM functionality of that udp connection
     *  Close method of this class does not close the underlaying udp connections
     *  but just set this object to logical closed state
     *
     *  @todo Implement friend request receiving from Opensim server
     *  @todo Implement Friend list fetch from Rex authentication server
     *  
     */
    class Connection : public Communication::ConnectionInterface
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "Communication(OpensimIM)"; } // for logging functionality

        static const int GlobalChatChannelID = 0;
        static const int SayDistance = 30;
        static const int ShoutDistance = 100;
        static const int WhisperDistance = 10;
        enum ChatType { Whisper = 0, Say = 1, Shout = 2, StartTyping = 4, StopTyping = 5, DebugChannel = 6, Region = 7, Owner = 8, Broadcast = 0xFF };
        enum ChatAudibleLevel { Not = -1, Barely = 0, Fully = 1 };
        enum ChatSourceType { System = 0, Agent = 1, Object = 2 };
        enum IMDialogTypes { DT_MessageFromAgent = 0, DT_MessageFromObject = 19, DT_FriendshipOffered = 38, DT_FriendshipAccepted = 39, DT_FriendshipDeclined = 40, DT_StartTyping = 41, DT_StopTyping = 42};

    public:
        Connection(Foundation::Framework* framework, const QString &agentID);
        virtual ~Connection();

        //! Provides name of the connection
        virtual QString GetName() const;

        //! Connection protocol
        virtual QString GetProtocol() const;

        //! Connection state
        virtual State GetState() const;

        //! Provides server address of this IM server connection
        virtual QString GetServer() const;

        //! @return user id associated with this connection
        virtual QString GetUserID() const;

        //! Provides textual descriptions about error
        //! If state is not STATE_ERROR then return empty
        virtual QString GetReason() const;

        //! Provides contact list associated with this IM server connection
        virtual Communication::ContactGroupInterface& GetContacts();

        //! User cannot set presence status in Opensim
        //! @return list of presence status opstions contacts might have
        virtual QStringList GetPresenceStatusOptionsForContact() const;

        //! User cannot set presence status in Opensim
        //! @return list of presence status options user can set
        virtual QStringList GetPresenceStatusOptionsForUser() const;

        //! Open new chat session with given contact
        //! This is opensim IM chat
        virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const Communication::ContactInterface &contact);

        //! Open new chat session to given room
        virtual Communication::ChatSessionInterface* OpenChatSession(const QString &channel);

        virtual Communication::VoiceSessionInterface* OpenVoiceSession(const Communication::ContactInterface &contact);

        //! Send a friend request to target address
        virtual void SendFriendRequest(const QString &target, const QString &message);

        //! Provides all received friend requests with in this connection session
        //! FriendRequest object state must be checked to find out new ones.
        //! If friend request is not answered the server will resend it on next 
        //! connection
        virtual Communication::FriendRequestVector GetFriendRequests() const;

        void RemoveContact(const Communication::ContactInterface &contact);

        //! Set presence status state of user
        //! @param status Allowed values are returned by GetPresenceStatusOptionsForSelf methos
        virtual void SetPresenceStatus(const QString &status);

        //! Set presene status message of user
        //! @param message Any text is accepted
        virtual void SetPresenceMessage(const QString &message);

        //! Closes the connection
        virtual void Close();

        bool HandleNetworkEvent(Foundation::EventDataInterface* data);

        bool HandleNetworkStateEvent(Foundation::EventDataInterface* data);

        //! Handle incoming improved instant messages
        //! this includes instant messages and friend requests and friend requestt responses responses
        bool HandleRexNetMsgImprovedInstantMessage(ProtocolUtilities::NetInMessage& msg);

        virtual Communication::ChatSessionInterface* OpenPrivateChatSession(const QString &user_id);

        //! Get presene status state of user
        virtual QString GetPresenceStatus() { return ""; };

        //! Get presene status message of user
        virtual QString GetPresenceMessage() { return ""; };

        //! Check and emit all pending friend requests
        /// No need for implementation in OpenSim Connection, only used in TelepathyIM::Connection
        virtual void CheckPendingFriendRequests() {}
    
    protected:
        //! Add console commands: 
        virtual void RegisterConsoleCommands();

        virtual void RequestFriendlist();

        //!
        virtual bool HandleOSNEChatFromSimulator(ProtocolUtilities::NetInMessage& msg);
        virtual bool HandleOnlineNotification(ProtocolUtilities::NetInMessage& msg);
        virtual bool HandleOfflineNotification(ProtocolUtilities::NetInMessage& msg);


        //! Opensim based servers have one global chat channel (id = "0")
        //! We create ChatSession object automatically when connection is established
        virtual void OpenWorldChatSession();

        //! Called when Instant message has been received from Opensim server.
        virtual void OnIMMessage(const QString &from_id, const QString &from_name, const QString &text);

        virtual void OnFriendshipAccepted(const QString &from_id);
        virtual void OnFriendshipDeclined(const QString &from_id);

        virtual ChatSession* GetPrivateChatSession(const QString &user_id);

    private:
        Foundation::Framework* framework_;
        QString name_;
        QString protocol_;
        State state_;
        QString server_;
        QString reason_;
        ChatSessionVector public_chat_sessions_;
        ChatSessionVector im_chat_sessions_;
        QString agent_uuid_; //! UUID of current user 
        ContactVector contacts_;
        FriendRequestVector friend_requests_;
        ContactGroup friend_list_;
    public slots:
        void OnWorldChatMessageReceived(const Communication::ChatMessageInterface &message);
    };
    typedef std::vector<Connection*> ConnectionVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OSIMConnection_h
