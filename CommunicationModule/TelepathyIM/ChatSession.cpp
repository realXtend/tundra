// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <TelepathyQt4/ContactManager>
#include "ChatSession.h"
#include "Connection.h"
#include "ContactGroup.h"
#include "ChatMessage.h"
#include "CoreDefines.h"
#include "CoreException.h"


namespace TelepathyIM
{
    ChatSession::ChatSession(Contact& self_contact, Contact& contact, Tp::ConnectionPtr tp_connection) : state_(STATE_INITIALIZING), self_participant_(&self_contact)
    {
        ChatSessionParticipant* p = new ChatSessionParticipant(&contact);
        participants_.push_back(p);

        QVariantMap params;
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), Tp::HandleTypeContact);
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandle"), contact.GetTpContact()->handle().at(0));

        Tp::PendingChannel* pending_channel = tp_connection->ensureChannel(params);
        connect(pending_channel,
                   SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnTextChannelCreated(Tp::PendingOperation*) ));
    }

    ChatSession::ChatSession(Contact &self_contact, Tp::TextChannelPtr tp_text_channel) : tp_text_channel_(tp_text_channel), state_(STATE_INITIALIZING), self_participant_(&self_contact)
    {
        Tp::Features features;
        features.insert(Tp::TextChannel::FeatureCore);
        features.insert(Tp::TextChannel::FeatureMessageCapabilities);
        features.insert(Tp::TextChannel::FeatureMessageQueue);
        connect(tp_text_channel_->becomeReady(features), SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnIncomingTextChannelReady(Tp::PendingOperation*)) );
    }

    ChatSession::ChatSession(const QString &room_id, Tp::ConnectionPtr tp_connection): state_(STATE_INITIALIZING), self_participant_(NULL)
    {
        QVariantMap params;
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType"), QLatin1String(TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT));
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetHandleType"), Tp::HandleTypeRoom);
        params.insert(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".TargetID"), room_id);

        Tp::PendingChannel* pending_channel = tp_connection->ensureChannel(params);
        connect(pending_channel,
                   SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnTextChannelCreated(Tp::PendingOperation*) ));
    }

    ChatSession::~ChatSession()
    {
        if (state_ == STATE_OPEN)
            Close();

        for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
        {
            SAFE_DELETE(*i);
        }
        message_history_.clear();
    }

    void ChatSession::SendChatMessage(const QString &text)
    {
        if (state_ == STATE_CLOSED)
            throw Exception("Channel closed");

        if (tp_text_channel_.isNull())
        {
            send_buffer_.push_back(text);
            return;
        }

        ChatMessage* m = new ChatMessage(&self_participant_, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);

        Tp::PendingSendMessage* p = tp_text_channel_->send( text );
        LogInfo(QString("Text message send to IM server: \n").append(text).append("\"").toStdString());

        //! @todo Connect this signal to actual message object
        connect(p, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnMessageSendAck(Tp::PendingOperation*) ));
    }

    Communication::ChatSessionInterface::State ChatSession::GetState() const
    {
        return state_;
    }

    void ChatSession::Close()
    {
        if ( tp_text_channel_.isNull() )
        {
            state_ = STATE_CLOSED; 
            return;
        }

        Tp::PendingOperation* op = tp_text_channel_->requestClose(); 
        LogDebug("Text channel is closed.");
        //connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnTextChannelClosed(Tp::PendingOperation*) ));
    }

    Communication::ChatSessionParticipantVector ChatSession::GetParticipants() const
    {
        Communication::ChatSessionParticipantVector participants;
        for (ChatSessionParticipantVector::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            participants.push_back( *i );
        }

        return participants;
    }

    Communication::ChatMessageVector ChatSession::GetMessageHistory()
    {
        Communication::ChatMessageVector messages;
        for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
        {
            messages.push_back( *i );
        }
        return messages;
    }

    ChatSessionParticipant* ChatSession::GetParticipant(Tp::ContactPtr contact)
    {
        if (!contact)
            return 0;

        for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            if ( (*i)->GetID().compare( contact->id() ) == 0)
                return *i;
        }
        return 0;
    }

    ChatSessionParticipant* ChatSession::GetParticipant(uint sender_id)
    {
        for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            if ( (*i)->GetID().compare(QString(sender_id)) == 0)
                return *i;
        }
        return 0;
    }

    void ChatSession::OnTextChannelCreated(Tp::PendingOperation* op)
    {
        if ( op->isError() )
        {
            state_ = STATE_ERROR;    
            return;
        }
        LogDebug("Text channel created.");
        Tp::PendingChannel *pending_channel = qobject_cast<Tp::PendingChannel *>(op);
        Tp::ChannelPtr text_channel = pending_channel->channel();
        tp_text_channel_ = Tp::TextChannelPtr( dynamic_cast<Tp::TextChannel *>(text_channel.data()) );
        //tp_text_channel_ = Tp::TextChannel::create(pending_channel->connection(), pending_channel->objectPath(), pending_channel->immutableProperties());
//        tp_text_channel_ = Tp::TextChannelPtr(dynamic_cast<Tp::TextChannel *>( pending_channel->channel().data() ));

        Tp::Features features;
        features.insert(Tp::TextChannel::FeatureMessageQueue);
        //features.insert(Tp::TextChannel::FeatureCore);
//        features.insert(Tp::TextChannel::FeatureMessageCapabilities);

        connect( tp_text_channel_->becomeReady(features),
                SIGNAL( finished(Tp::PendingOperation*) ),
                SLOT( OnOutgoingTextChannelReady(Tp::PendingOperation*) ));
    }

    void ChatSession::OnChannelInvalidated(Tp::DBusProxy *p, const QString &me, const QString &er)
    {
        state_ = STATE_ERROR;
        LogError("Text channel become invalid!");
    }
    
    void ChatSession::OnIncomingTextChannelReady(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            LogError("Incoming text channel cannot become ready state");
            return;
        }
        LogDebug("Incoming text channel ready.");
        Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
        Tp::TextChannelPtr channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));
        tp_text_channel_ = channel;

        connect(tp_text_channel_.data(),
                SIGNAL( messageReceived(const Tp::ReceivedMessage &) ),
                SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ));

        connect(tp_text_channel_.data(), 
                SIGNAL( pendingMessageRemoved(const Tp::ReceivedMessage &) ), 
                SLOT( OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &) ));

        //! @HACK and memory leak here. We should have a contact object from Connection object!
        ChatSessionParticipant* p = new ChatSessionParticipant( new Contact(tp_text_channel_->initiatorContact()) );
        participants_.push_back(p);

        HandlePendingMessage();
        state_ = STATE_OPEN;
        emit Ready(this);
        emit Opened(this);
        for (QStringList::iterator i = send_buffer_.begin(); i != send_buffer_.end(); ++i)
            SendChatMessage( *i);
    }

    void ChatSession::OnOutgoingTextChannelReady(Tp::PendingOperation* op)
    {
        if (op->isError())
        {
            state_ = STATE_ERROR;
            QString message = "Text channel cannot become ready for ChatSession object.";
            LogError(message.toStdString());
            return;
        }
        LogDebug("Outgoing text channel is ready.");

        Tp::PendingReady *pr = qobject_cast<Tp::PendingReady *>(op);
        Tp::TextChannelPtr channel = Tp::TextChannelPtr(qobject_cast<Tp::TextChannel *>(pr->object()));
        tp_text_channel_ = channel;

        QStringList interfaces = tp_text_channel_->interfaces();
        for (QStringList::iterator i = interfaces.begin(); i != interfaces.end(); ++i)
        {
            QString line = "Text channel have interface: ";
            line.append(*i);
            LogDebug(line.toStdString());
        }

//        Tp::ContactPtr initiator = tp_text_channel_->initiatorContact();

        bool have_message_capabilities = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageCapabilities);
        bool have_message_queue = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageQueue);
        bool have_message_sent_signal = tp_text_channel_->isReady(Tp::TextChannel::FeatureMessageSentSignal);
        //if (!have_message_capabilities)
        //{
        //    LogError("Text channel doesn't have the message capabilities!");
        //    state_ = STATE_ERROR;
        //    return;
        //}

        connect(tp_text_channel_.data(),
                SIGNAL( messageReceived(const Tp::ReceivedMessage &) ),
                SLOT( OnMessageReceived(const Tp::ReceivedMessage &) ));

        connect(tp_text_channel_.data(), 
                SIGNAL( pendingMessageRemoved(const Tp::ReceivedMessage &) ), 
                SLOT( OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &) ));

        HandlePendingMessage();
        state_ = STATE_OPEN;
        emit Opened(this);
        for (QStringList::iterator i = send_buffer_.begin(); i != send_buffer_.end(); ++i)
        {
            QString text = *i;
            SendChatMessage(text);
        }
    }

    void ChatSession::HandlePendingMessage()
    {
        QDBusPendingReply<Tp::PendingTextMessageList> pending_messages = tp_text_channel_->textInterface()->ListPendingMessages(true);
        
        if( !pending_messages.isFinished() )
            pending_messages.waitForFinished();
        if ( pending_messages.isValid() )
        {
            LogDebug("Received pending messages:");
            QDBusMessage m = pending_messages.reply();
            Tp::PendingTextMessageList list = pending_messages.value();
            if (list.size() == 0)
            {
                LogDebug("There is no pending messages.");
            }
            else
            {
                LogDebug(QString("There is ").append(QString::number(list.size())).append(" pending messages.").toStdString());
                for (Tp::PendingTextMessageList::iterator i = list.begin(); i != list.end(); ++i)
                {
                    QString note = QString("* Pending message received: ").append(i->text);
                    LogDebug(note.toStdString());

                    uint type = i->messageType; //! @todo Check if we need value of this parameter
                    if (type != Tp::ChannelTextMessageTypeNormal)
                        continue;
                    ChatSessionParticipant* originator = GetParticipant(i->sender);
                    if (originator == 0)
                    {
                        //! @HACK and memory leak here. We should have a contact object from Connection object!
                        //! @HACK We should search contact object with given i->sender id value
                        originator = new ChatSessionParticipant(new Contact(tp_text_channel_->initiatorContact()));
                        participants_.push_back(originator);
                    }

                    ChatMessage* message = new ChatMessage( originator, QDateTime::fromTime_t(i->unixTimestamp), i->text);
                    message_history_.push_back(message);
                    emit( MessageReceived(*message) );
                }
            }
        }
        else
            LogError("Received invalid pending messages");
    }

    void ChatSession::OnMessageSendAck(Tp::PendingOperation* op)
    {
        //! @todo Move this slot to actual message object
        //! @todo IMPLEMENT
    }

    void ChatSession::OnMessageReceived(const Tp::ReceivedMessage &message)
    {
        QList<Tp::ReceivedMessage> messages;
        messages.append(message);
        tp_text_channel_->acknowledge(messages);

        ChatSessionParticipant* from = GetParticipant(message.sender());
        ChatMessage* m = new ChatMessage(from, message.received(), message.text());
        message_history_.push_back(m);
        emit( MessageReceived(*m) );
    }

    void ChatSession::OnTextChannelClosed(Tp::PendingOperation* op)
    {
        state_ = STATE_CLOSED;
        emit( Closed(this) );
    }

    void ChatSession::OnChannelPendingMessageRemoved(const Tp::ReceivedMessage &message)
    {

    }

} // end of namespace: TelepathyIM
