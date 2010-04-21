// For conditions of distribution and use, see copyright notice in license.txt

#include <StableHeaders.h>
#include "DebugOperatorNew.h"
#include <RexLogicModule.h> // chat
#include <RealXtend/RexProtocolMsgIDs.h>

#include "Connection.h"
#include "ConnectionProvider.h"
#include "ConsoleCommandServiceInterface.h"
#include "ModuleManager.h"
#include "WorldStream.h"
#include "OpenSim/BuddyList.h"
#include "ServiceManager.h"
#include "NetworkMessages/NetInMessage.h"
#include "Framework.h"

#include "MemoryLeakCheck.h"

namespace OpensimIM
{

    Connection::Connection(Foundation::Framework* framework, const QString &agentID) 
        : framework_(framework), agent_uuid_(agentID), name_(""), protocol_(OPENSIM_IM_PROTOCOL), server_(""), reason_("")
    {
        // OpensimIM connection is automatically established when connected to world so 
        // initial state is always STATE_READY
        state_ = STATE_OPEN;

        RequestFriendlist();
        RegisterConsoleCommands();
        OpenWorldChatSession();
    }

    Connection::~Connection()
    {
        for( ChatSessionVector::iterator i =  public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
        {
            ChatSession* session = *i;
            SAFE_DELETE(session);
        }
        public_chat_sessions_.clear();

        for( ChatSessionVector::iterator i =  im_chat_sessions_.begin(); i != im_chat_sessions_.end(); ++i)
        {
            ChatSession* session = *i;
            SAFE_DELETE(session);
        }
        im_chat_sessions_.clear();

        for( ContactVector::iterator i =  contacts_.begin(); i != contacts_.end(); ++i)
        {
            Contact* contact = *i;
            SAFE_DELETE(contact);
        }
        contacts_.clear();

        for( FriendRequestVector::iterator i = friend_requests_.begin(); i != friend_requests_.end(); ++i)
        {
            FriendRequest* request = *i;
            SAFE_DELETE(request);
        }

        state_ = STATE_CLOSED;
    }
    
    QString Connection::GetName() const
    {
        return name_;
    }
    
    QString Connection::GetProtocol() const
    {
        return protocol_;
    }

    Communication::ConnectionInterface::State Connection::GetState() const
    {
        return state_;
    }

    QString Connection::GetServer() const
    {
        return server_;
    }

    QString Connection::GetUserID() const
    {
        return agent_uuid_;
    }

    QString Connection::GetReason() const
    {
        return reason_;
    }

    Communication::ContactGroupInterface& Connection::GetContacts()
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        return friend_list_;
    }
        
    QStringList Connection::GetPresenceStatusOptionsForContact() const
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        QStringList options;
        //! Opensim provides just two online state options
        options.append("online");
        options.append("offline");
        return QStringList();
    }

    QStringList Connection::GetPresenceStatusOptionsForUser() const
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        // In Opensim user cannot set the presence status 
        QStringList options;
        return QStringList();
    }

    Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        return OpenPrivateChatSession( contact.GetID() );
    }

    Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString &user_id)
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        ChatSession* session = GetPrivateChatSession(user_id);
        if (!session)
        {
            session = new ChatSession(framework_, user_id, false);
            im_chat_sessions_.push_back(session);
        }
        return session;
    }

    Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel)
    {
        if (state_ != STATE_OPEN)
            throw Exception("The connection is closed.");

        for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
        {
            ChatSession* chat_session = *i;

            if ( chat_session->GetID().compare( channel ) == 0 )
            {
                return chat_session;
            }
        }

        ChatSession* session = new ChatSession(framework_, channel, true);
        public_chat_sessions_.push_back(session);
        return session;
    }

    Communication::VoiceSessionInterface* Connection::OpenVoiceSession(const Communication::ContactInterface &contact)
    {
        //! @todo IMPLEMENT
        return NULL;
    }

    void Connection::SendFriendRequest(const QString &target, const QString &message)
    {
        if (state_ != STATE_OPEN)
            throw Exception("Cannot send text message, the connection is closed.");

        RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

        if (rexlogic_ == NULL)
            throw Exception("Cannot send text message, RexLogicModule is not found");
        WorldStreamPtr connection = rexlogic_->GetServerConnection();

        if ( connection == NULL )
            throw Exception("Cannot send text message, rex server connection is not found");

        if ( !connection->IsConnected() )
            throw Exception("Cannot send text message, rex server connection is not established");

        connection->SendFormFriendshipPacket(RexUUID( target.toStdString() ));
    }

    void Connection::RemoveContact(const Communication::ContactInterface &contact)
    {
        //! @todo IMPLEMENT
        throw Exception("Not implemented.");
    }

    Communication::FriendRequestVector Connection::GetFriendRequests() const
    {
        return Communication::FriendRequestVector();
    }

    void Connection::SetPresenceStatus(const QString &status)
    {
        //! Not supported by Opensim
    }

    void Connection::SetPresenceMessage(const QString &message)
    {
        //! Not supported by Opensim
    }
        
    void Connection::Close()
    {
        state_ = STATE_CLOSED;
        emit ConnectionClosed( *this );
    }

    void Connection::RegisterConsoleCommands()
    {
        boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if ( !console_service )
        {
            LogError("Cannot register console commands :command service not found.");
            return;
        }
    }

    void Connection::RequestFriendlist()
    {
        boost::weak_ptr<ProtocolUtilities::ProtocolModuleInterface> currentProtocolModule = framework_->GetModuleManager()->GetModule<RexLogic::RexLogicModule>(Foundation::Module::MT_WorldLogic).lock().get()->GetServerConnection()->GetCurrentProtocolModuleWeakPointer();
        if (currentProtocolModule.lock().get())
        {
            ProtocolUtilities::BuddyListPtr buddy_list = currentProtocolModule.lock()->GetClientParameters().buddy_list;
            ProtocolUtilities::BuddyVector buddies = buddy_list->GetBuddies();
            for (ProtocolUtilities::BuddyVector::iterator i = buddies.begin(); i != buddies.end(); ++i)
            {
                //! @todo Fetch name of this buddy 
                Contact* contact = new Contact(    (*i)->GetID().ToString().c_str(), "" );
                friend_list_.AddContact(contact);
                contacts_.push_back(contact);
            }
        }
    }

    bool Connection::HandleNetworkEvent(Foundation::EventDataInterface* data)
    {
        ProtocolUtilities::NetworkEventInboundData *event_data = dynamic_cast<ProtocolUtilities::NetworkEventInboundData *>(data);
        if (!event_data)
            return false;
            
        
        const ProtocolUtilities::NetMsgID msgID = event_data->messageID;
        ProtocolUtilities::NetInMessage *msg = event_data->message;
        switch(msgID)
        {
        case RexNetMsgChatFromSimulator: return HandleOSNEChatFromSimulator(*msg); break;
        case RexNetMsgImprovedInstantMessage: return HandleRexNetMsgImprovedInstantMessage(*msg); break;
//        case RexNetMsgTerminateFriendship: return false; break;
//        case RexNetMsgDeclineFriendship: return false;
        case RexNetMsgOnlineNotification: return HandleOnlineNotification(*msg); break;
        case RexNetMsgOfflineNotification: return HandleOfflineNotification(*msg); break;
        }
        return false;
    }

    bool Connection::HandleRexNetMsgImprovedInstantMessage(ProtocolUtilities::NetInMessage& msg)
    {
        try
        {
            msg.ResetReading();

            RexUUID agent_id = msg.ReadUUID();
            RexUUID session_id = msg.ReadUUID();
            bool is_group_message = msg.ReadBool();
            RexUUID to_agent_id = msg.ReadUUID();
            msg.SkipToNextVariable(); // ParentEstateID
            RexUUID region_id = msg.ReadUUID();
            RexTypes::Vector3 position = msg.ReadVector3();
            int offline = msg.ReadU8();
            int dialog_type = msg.ReadU8();
            RexUUID id = msg.ReadUUID();
            msg.SkipToNextVariable(); // Timestamp
            std::string from_agent_name = msg.ReadString();
            std::string message = msg.ReadString();
            msg.SkipToNextVariable(); // BinaryBucket

            switch (dialog_type)
            {
            case DT_FriendshipOffered:
                {
                    QString calling_card_folder = ""; //! @todo get the right value
                    FriendRequest* request = new FriendRequest(framework_, agent_id.ToString().c_str(), from_agent_name.c_str(), calling_card_folder);
                    friend_requests_.push_back(request);
                    emit( FriendRequestReceived(*request) );
                }
                break;
            case DT_MessageFromAgent:
                {
                    QString from_id = agent_id.ToString().c_str();
                    OnIMMessage( from_id, QString(from_agent_name.c_str()), QString( message.c_str() ) );
                }
                break;
            case DT_FriendshipAccepted:
                {
                    QString from_id = agent_id.ToString().c_str();
                    OnFriendshipAccepted(from_id);
                }
                break;
                
            case DT_FriendshipDeclined:
                {
                    QString from_id = agent_id.ToString().c_str();
                    OnFriendshipDeclined(from_id);
                }
                break;
            }
        }
        catch(NetMessageException)
        {
            return false;
        }
        return false;        
    }

    bool Connection::HandleNetworkStateEvent(Foundation::EventDataInterface* data)
    {
        return false;
    }


    bool Connection::HandleOSNEChatFromSimulator(ProtocolUtilities::NetInMessage& msg)
    {
        try
        {              

            msg.ResetReading();

            std::size_t size = 0;
            const boost::uint8_t* buffer = msg.ReadBuffer(&size);
            std::string from_name = std::string((char*)buffer);
            RexUUID source = msg.ReadUUID();
            RexUUID object_owner = msg.ReadUUID();
            ChatSourceType source_type = static_cast<ChatSourceType>( msg.ReadU8() );
            ChatType chat_type = static_cast<ChatType>( msg.ReadU8() ); 
            ChatAudibleLevel audible = static_cast<ChatAudibleLevel>( msg.ReadU8() );
            RexTypes::Vector3 position = msg.ReadVector3();
            std::string message = msg.ReadString();
            if ( message.size() > 0 )
            {
                for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
                {
                    if ( (*i)->GetID().compare("0") != 0 )
                        continue;

                    QString source_uuid = source.ToString().c_str();
                    QString source_name = from_name.c_str();
                    QString message_text = QString::fromUtf8(message.c_str(), message.size());

                    switch (source_type)
                    {
                    case Connection::Agent:
                    case Connection::Object:
                    case Connection::System:
                        (*i)->MessageFromAgent(source_uuid, source_name, message_text);
                        break;
                    }
                }
            }
        }
        catch(NetMessageException /*&e*/)
        {
            return false;
        }
        return false;        
    }

    bool Connection::HandleOnlineNotification(ProtocolUtilities::NetInMessage& msg)
    {
        msg.ResetReading();
        size_t instance_count = msg.ReadCurrentBlockInstanceCount();
        for (int i = 0; i < instance_count; ++i)
        {
            QString agent_id = msg.ReadUUID().ToString().c_str();
            for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
            {
                if ((*i)->GetID().compare(agent_id) == 0)
                    (*i)->SetOnlineStatus(true);
            }
        }
        return false;
    }

    bool Connection::HandleOfflineNotification(ProtocolUtilities::NetInMessage& msg)
    {
        msg.ResetReading();
        size_t instance_count = msg.ReadCurrentBlockInstanceCount();
        for (int i = 0; i < instance_count; ++i)
        {
            QString agent_id = msg.ReadUUID().ToString().c_str();
            for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
            {
                if ((*i)->GetID().compare(agent_id) == 0)
                    (*i)->SetOnlineStatus(false);
            }
        }
        return false;
    }

    void Connection::OpenWorldChatSession()
    {
        Communication::ChatSessionInterface* world_chat = OpenChatSession("0");
        connect( world_chat, SIGNAL( MessageReceived(const Communication::ChatMessageInterface &) ), SLOT( OnWorldChatMessageReceived(const Communication::ChatMessageInterface &) ));
    }

    void Connection::OnWorldChatMessageReceived(const Communication::ChatMessageInterface &message)
    {
        // Please no debug prints, its working. These were showing in release mode too, dunno why. - Jonne
        //QString text = "OpensimIM, public chat: ";
        //text.append( message.GetOriginator()->GetName() );
        //text.append(" : ");
        //text.append( message.GetText() );
        //LogDebug( text.toStdString() );
    }

    void Connection::OnIMMessage(const QString &from_id, const QString &from_name, const QString &text)
    {
        ChatSession* session = GetPrivateChatSession(from_id);
        if ( !session )
        {
            // This IM message was first from this user
            session = new ChatSession(framework_, from_id, false);
            im_chat_sessions_.push_back(session);
            
        }
        session->MessageFromAgent(from_id, from_name, text);
    }

    ChatSession* Connection::GetPrivateChatSession(const QString &user_id)
    {
        for (ChatSessionVector::iterator i = im_chat_sessions_.begin(); i != im_chat_sessions_.end(); ++i)
        {
            Communication::ChatSessionParticipantVector participants = (*i)->GetParticipants();
            assert( participants.size() == 1); // Opensim IM chat sessions are always between two user
            Communication::ChatSessionParticipantInterface* participant = participants[0];
            if ( participant->GetID().compare(user_id) == 0 )
            {
                return (*i);
            }
        }
        return NULL;
    }

    void Connection::OnFriendshipAccepted(const QString &from_id)
    {
        Contact* contact = new Contact(from_id, "");
        contacts_.push_back(contact);
        friend_list_.AddContact(contact);

        emit( FriendRequestAccepted(from_id) );
    }

    void Connection::OnFriendshipDeclined(const QString &from_id)
    {
        emit( FriendRequestRejected(from_id) );
    }

} // end of namespace: OpensimIM
