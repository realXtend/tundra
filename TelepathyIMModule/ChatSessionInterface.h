// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_ChatSessionInterface_h
#define incl_Comm_ChatSessionInterface_h

#include <QObject>
#include <QString>

#include "TelepathyIMModuleFwd.h"

namespace Communication
{
    /**
     * Text message based communication session with one or more participants.
     * This can represents irc channel or jabber conversation.
     *
     * @todo Getter for originator of the chat session (gui needs id of friend to create tab and store id to local containers with that id).
     *       GUI now needs to go through ChatSessionParticipantVector and do unneccesary looping to get one QString
     */
    class ChatSessionInterface: public QObject
    {
        Q_OBJECT
    public:
        enum State { STATE_INITIALIZING, STATE_OPEN, STATE_CLOSED, STATE_ERROR };

        virtual ~ChatSessionInterface() {};

        //! Send a text message to chat session
        //! @param text The message
        virtual void SendChatMessage(const QString &text) = 0;

        //! @return State of the session
        virtual State GetState() const = 0;

        //! Closes the chat session. No more messages can be send or received. 
        //! Causes Closed signals to be emitted.
        virtual void Close() = 0;

        //! @return all known participants of the chat session inlcuding the user
        virtual ChatSessionParticipantVector GetParticipants() const = 0;

        //! @return the originator of the chat session if there is one
        //!         eg. with public chat session this return 0
//        virtual ChatSessionParticipantInterface* GetOriginator() const = 0;

        //! @return the message history of this chat sessions
        virtual ChatMessageVector GetMessageHistory() = 0;

    signals:
        void MessageReceived(const Communication::ChatMessageInterface &message);
        void ParticipantJoined(const ChatSessionParticipantInterface& participant); // not implemented by any class currently
        void ParticipantLeft(const ChatSessionParticipantInterface& participant); // not implemented by any class currently
        void Opened(ChatSessionInterface*);
        void Closed(ChatSessionInterface*);
    };
}

#endif

