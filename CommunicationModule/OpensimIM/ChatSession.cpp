// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include <RexLogicModule.h>
#include <QTime>

#include "ChatSession.h"
#include "ModuleManager.h"
#include "WorldStream.h"
#include "CoreDefines.h"
#include "CoreException.h"
#include "Framework.h"

#include "MemoryLeakCheck.h"

namespace OpensimIM
{
    ChatSession::ChatSession(Foundation::Framework* framework, const QString &id, bool public_chat): framework_(framework), server_("0", "Server"), private_im_session_(!public_chat), self_("", "You"), state_(STATE_OPEN)
    {
        //! \todo Add support to different channel numbers
        //!       This requires changes to SendChatFromViewerPacket method or 
        //!       chat packet must be construaed by hand.
        if ( public_chat )
        {
            channel_id_ = id;
            if ( channel_id_.compare("0") != 0 )
                throw Exception("Cannot create chat session, channel id now allowed"); 
        }
        else
        {
            ChatSessionParticipant* p = new ChatSessionParticipant(id, "");
            participants_.push_back(p);
        }
    }

    ChatSession::~ChatSession()
    {
        for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            ChatSessionParticipant* participant = *i;
            SAFE_DELETE(participant);
        }
        participants_.clear();

        for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
        {
            ChatMessage* message = *i;
            SAFE_DELETE(message);
        }
        message_history_.clear();
    }

    void ChatSession::SendChatMessage(const QString &text)
    {
        if (private_im_session_)
            SendPrivateIMMessage(text);
        else
            SendPublicChatMessage(text);
    }

    Communication::ChatSessionInterface::State ChatSession::GetState() const
    {
        return state_;
    }

    void ChatSession::SendPrivateIMMessage(const QString &text)
    {
        if (state_ != STATE_OPEN)
            throw Exception("Chat session is closed");

        RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

        if (rexlogic_ == NULL)
            throw Exception("Cannot send IM message, RexLogicModule is not found");
        WorldStreamPtr connection = rexlogic_->GetServerConnection();

        if ( connection == NULL )
            throw Exception("Cannot send IM message, rex server connection is not found");

        if ( !connection->IsConnected() )
            throw Exception("Cannot send IM message, rex server connection is not established");

        ChatMessage* m = new ChatMessage(&self_, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);

        for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            connection->SendImprovedInstantMessagePacket(RexUUID( (*i)->GetID().toStdString() ), text.toStdString() );
        }
    }

    void ChatSession::SendPublicChatMessage(const QString &text)
    {
        RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

        if (rexlogic_ == NULL)
            throw Exception("Cannot send text message, RexLogicModule is not found");
        WorldStreamPtr connection = rexlogic_->GetServerConnection();

        if ( connection == NULL )
            throw Exception("Cannot send text message, rex server connection is not found");

        if ( !connection->IsConnected() )
            throw Exception("Cannot send text message, rex server connection is not established");

        ChatMessage* m = new ChatMessage(&self_, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);

        connection->SendChatFromViewerPacket( std::string(text.toUtf8()) );
    }

    void ChatSession::Close()
    {
        //! \todo IMPLEMENT
        state_ = STATE_CLOSED;
        emit( Closed(this) );
    }

    Communication::ChatMessageVector ChatSession::GetMessageHistory() 
    {
        Communication::ChatMessageVector message_history;
        for (ChatMessageVector::iterator i = message_history_.begin(); i != message_history_.end(); ++i)
        {
            ChatMessage* message = *i;
            assert( message != NULL );
            message_history.push_back( message );
        }
        return message_history;
    }

    void ChatSession::MessageFromAgent(const QString &avatar_id, const QString &from_name, const QString &text)
    {
        ChatSessionParticipant* participant = FindParticipant(avatar_id);
        if ( !participant )
        {
            participant = new ChatSessionParticipant(avatar_id, from_name);
            participants_.push_back(participant);
        }

        if (participant->GetName().size() == 0)
            ((ChatSessionParticipant*)participant)->SetName(from_name); //! @HACK We should get the name from some another source!

        ChatMessage* m = new ChatMessage(participant, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);

        emit MessageReceived(*m);
    }

    void ChatSession::MessageFromServer(const QString &text)
    {
        ChatMessage* m = new ChatMessage(&server_, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);
        emit MessageReceived(*m);
    }

    void ChatSession::MessageFromObject(const QString &object_id, const QString &text)
    {
        ChatSessionParticipant* originator = FindParticipant(object_id);
        if ( !originator )
        {
            //! @todo Find out the name of this object and give it as argument for participant object constructor
            ChatSessionParticipant* originator = new ChatSessionParticipant(object_id, object_id);
            participants_.push_back(originator);
        }
        ChatMessage* m = new ChatMessage(originator, QDateTime::currentDateTime(), text);
        message_history_.push_back(m);
        emit MessageReceived(*m);
    }

    ChatSessionParticipant* ChatSession::FindParticipant(const QString &uuid)
    {
        for (ChatSessionParticipantVector::iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            if ( (*i)->GetID().compare(uuid) == 0 )
                return *i;
        }
        return NULL;
    }

    Communication::ChatSessionParticipantVector ChatSession::GetParticipants() const
    {
        Communication::ChatSessionParticipantVector participants;
        for (ChatSessionParticipantVector::const_iterator i = participants_.begin(); i != participants_.end(); ++i)
        {
            participants.push_back(*i);
        }
        return participants;
    }

    QString ChatSession::GetID() const
    {
        return channel_id_;
    }

    bool ChatSession::IsPrivateIMSession()
    {
        return private_im_session_;
    }

} // end of namespace: OpensimIM
