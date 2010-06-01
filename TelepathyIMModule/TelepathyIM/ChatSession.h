// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_ChatSession_h
#define incl_Communication_TelepathyIM_ChatSession_h

#include <string>
#include <QStringList>
#include <TelepathyQt4/TextChannel>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/PendingOperation>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingChannel>
#include <TelepathyQt4/ReceivedMessage>
//#include <Foundation.h>
//#include "interface.h"
//#include "ContactGroup.h"
//#include "ChatMessage.h"
#include "ChatSessionInterface.h"
#include "ChatSessionParticipant.h"
#include "ModuleLoggingFunctions.h"
#include "TelepathyIMModuleFwd.h"

namespace TelepathyIM
{

    /**
     *  Text message based communication session with one or more participants.
     *  This can represents irc channel or jabber conversation.
     * 
     *  This uses Telepathy text channel to communicate with IM server.
     */
    class ChatSession : public Communication::ChatSessionInterface
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "TelepathyIM"; } // for logging functionality

    public:
        //! Used by TelepathyIM::Connection class
        //! when user initiates a new chat sessionj
        ChatSession(Contact& self_contact, Contact& contact, Tp::ConnectionPtr tp_connection);

        //! Used by TelepathyIM:Connection class
        //! When a chat session is initiated by IM server
        ChatSession(Contact& self_contact, Tp::TextChannelPtr tp_text_channel);    

        //! Used by TelepathyIM:Connection class
        //! When user open chat room session
        ChatSession(const QString &room_id, Tp::ConnectionPtr tp_connection);

        virtual ~ChatSession();

        //! Send a text message to chat session
        //! @param text The message
        virtual void SendChatMessage(const QString &text);

        //! @return State of the session
        virtual Communication::ChatSessionInterface::State GetState() const;

        //! Closes the chat session. No more messages can be send or received. 
        //! Causes Closed signals to be emitted.
        virtual void Close();

        //! @return all known participants of the chat session
        virtual Communication::ChatSessionParticipantVector GetParticipants() const;

        //! @return the message history of this chat sessions
        virtual Communication::ChatMessageVector GetMessageHistory();

        virtual Tp::TextChannelPtr GetTpTextChannel() {return tp_text_channel_;}

    protected:
        virtual void HandlePendingMessage();
        virtual ChatSessionParticipant* GetParticipant(Tp::ContactPtr contact);
        virtual ChatSessionParticipant* GetParticipant(uint sender_id);

        State state_;
        Tp::TextChannelPtr tp_text_channel_;
        Tp::Connection* tp_conneciton_;
        QStringList send_buffer_;
        ChatMessageVector message_history_;
        ChatSessionParticipantVector participants_;
        ChatSessionParticipant self_participant_;
    protected slots:
        //! This method is called ONLY when session is established by client 
        //! and it's NOT called when the session is established by server
        virtual void OnTextChannelCreated(Tp::PendingOperation* op);
        virtual void OnIncomingTextChannelReady(Tp::PendingOperation* op);
        virtual void OnOutgoingTextChannelReady(Tp::PendingOperation* op);
        virtual void OnMessageSendAck(Tp::PendingOperation* op);
        virtual void OnChannelInvalidated(Tp::DBusProxy *, const QString &, const QString &);
        virtual void OnMessageReceived(const Tp::ReceivedMessage &message);
        virtual    void OnTextChannelClosed(Tp::PendingOperation* op);
        virtual void OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &message);
    signals:
        void Ready(ChatSession* session);
    };
//    typedef std::vector<ChatSession*> ChatSessionVector;
    
} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ChatSession_h
