#include "Connection.h"
#include "StableHeaders.h"
#include "RexLogicModule.h" // chat
#include "RexProtocolMsgIDs.h"
#include "OpensimProtocolModule.h"
#include "ConnectionProvider.h"

namespace OpensimIM
{

	Connection::Connection(Foundation::Framework* framework): framework_(framework), name_(""), protocol_(OPENSIM_IM_PROTOCOL), server_(""), reason_("")
	{
		// OpensimIM connection is automatically established when connected to world so 
		// initial state is always STATE_READY
		state_ = STATE_READY;

		RequestFriendlist();
		RegisterConsoleCommands();
		OpenWorldChatSession();

		//emit( ConnectionReady(*this) );
	}

	Connection::~Connection()
	{
		for( ChatSessionVector::iterator i =  public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
		{
			delete (*i);
			*i = NULL;
		}
		public_chat_sessions_.clear();

		for( ChatSessionVector::iterator i =  im_chat_sessions_.begin(); i != im_chat_sessions_.end(); ++i)
		{
			delete *i;
			*i = NULL;
		}
		im_chat_sessions_.clear();

		for( ContactVector::iterator i =  contacts_.begin(); i != contacts_.end(); ++i)
		{
			delete *i;
			*i = NULL;
		}
		contacts_.clear();
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

	QString Connection::GetReason() const
	{
		return reason_;
	}

	Communication::ContactGroupInterface* Connection::GetContacts() const
	{
		if (state_ != STATE_READY)
			throw Core::Exception("The connection is closed.");

		//! \todo IMPELEMENT
		return NULL;
	}
		
	QStringList Connection::GetAvailablePresenceStatusOptions() const
	{
		if (state_ != STATE_READY)
			throw Core::Exception("The connection is closed.");

		QStringList options;
		//! Opensim provides just two online state options
		options.append("online");
		options.append("offline");
		return QStringList();
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
	{
		if (state_ != STATE_READY)
			throw Core::Exception("The connection is closed.");

		//! \todo IMPLEMENT
		return NULL;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString &user_id)
	{
		if (state_ != STATE_READY)
			throw Core::Exception("The connection is closed.");

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
		if (state_ != STATE_READY)
			throw Core::Exception("The connection is closed.");

		for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
		{
			if ( (*i)->GetID().compare( channel ) == 0 )
			{
				return (*i);
			}
		}

		ChatSession* session = new ChatSession(framework_, channel, true);
		public_chat_sessions_.push_back(session);
		return session;
	}
	
	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		if (state_ != STATE_READY)
			throw Core::Exception("Cannot send text message, the connection is closed.");

		RexLogic::RexLogicModule *rexlogic_ = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());

		if (rexlogic_ == NULL)
			throw Core::Exception("Cannot send text message, RexLogicModule is not found");
		RexLogic::RexServerConnectionPtr connection = rexlogic_->GetServerConnection();

		if ( connection == NULL )
			throw Core::Exception("Cannot send text message, rex server connection is not found");

		if ( !connection->IsConnected() )
			throw Core::Exception("Cannot send text message, rex server connection is not established");

		connection->SendFormFriendshipPacket(RexTypes::RexUUID( target.toStdString() ));
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		return Communication::FriendRequestVector();
	}
		
	void Connection::Close()
	{
		//! @todo Send information to server

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
		OpenSimProtocol::OpenSimProtocolModule *opensim_protocol_ = dynamic_cast<OpenSimProtocol::OpenSimProtocolModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_OpenSimProtocol).lock().get());
		if ( !opensim_protocol_ )
			return;
		OpenSimProtocol::BuddyListPtr buddy_list = opensim_protocol_->GetClientParameters().buddy_list;
		OpenSimProtocol::BuddyVector buddies = buddy_list->GetBuddies();
		for (OpenSimProtocol::BuddyVector::iterator i = buddies.begin(); i != buddies.end(); ++i)
		{
			//! @todo Fetch name of this buddy 
			Contact* contact = new Contact(	(*i)->GetID().ToString().c_str(), "" );
			contacts_.push_back(contact);
		}
	}

	bool Connection::HandleNetworkEvent(Foundation::EventDataInterface* data)
	{
		OpenSimProtocol::NetworkEventInboundData *event_data = dynamic_cast<OpenSimProtocol::NetworkEventInboundData *>(data);
        if (!event_data)
            return false;
            
		
        const NetMsgID msgID = event_data->messageID;
        NetInMessage *msg = event_data->message;
        switch(msgID)
        {
		case RexNetMsgChatFromSimulator: return HandleOSNEChatFromSimulator(*msg); break;
		case RexNetMsgFormFriendship: return true; false;
		case RexNetMsgImprovedInstantMessage: return HandleRexNetMsgImprovedInstantMessage(*msg); break;
		case RexNetMsgStartLure: return false; break;
		case RexNetMsgTerminateFriendship: return false; break;
		case RexNetMsgDeclineFriendship: return false;
		case RexNetMsgOnlineNotification: return HandleOnlineNotification(*msg);
		case RexNetMsgOfflineNotification: return HandleOfflineNotification(*msg);
		}

		return false;
	}

	bool Connection::HandleRexNetMsgImprovedInstantMessage(NetInMessage& msg)
	{
		try
		{
			msg.ResetReading();

			RexTypes::RexUUID agent_id = msg.ReadUUID();
			RexTypes::RexUUID session_id = msg.ReadUUID();
			bool is_group_message = msg.ReadBool();
			RexTypes::RexUUID to_agent_id = msg.ReadUUID();
			msg.SkipToNextVariable(); // ParentEstateID
			RexTypes::RexUUID region_id = msg.ReadUUID();
			RexTypes::Vector3 position = msg.ReadVector3();
			int offline = msg.ReadU8();
			int dialog_type = msg.ReadU8();
			RexTypes::RexUUID id = msg.ReadUUID();
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
			}
		}
		catch(NetMessageException)
		{
			return false;
		}
		return true;		
	}

	bool Connection::HandleNetworkStateEvent(Foundation::EventDataInterface* data)
	{
		return false;
	}


	bool Connection::HandleOSNEChatFromSimulator(NetInMessage& msg)
	{
		try
		{
			msg.ResetReading();

			std::size_t size = 0;
			const boost::uint8_t* buffer = msg.ReadBuffer(&size);
			std::string from_name = std::string((char*)buffer);
			RexTypes::RexUUID source = msg.ReadUUID();
			RexTypes::RexUUID object_owner = msg.ReadUUID();
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
					QString message_text = message.c_str();

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
		return true;		
	}

	bool Connection::HandleOnlineNotification(NetInMessage& msg)
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

	bool Connection::HandleOfflineNotification(NetInMessage& msg)
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
		connect( world_chat, SIGNAL(MessageReceived(const QString&, const Communication::ChatSessionParticipantInterface&)), SLOT(OnWorldChatMessageReceived(const QString&, const Communication::ChatSessionParticipantInterface&)) );
	}

	void Connection::OnWorldChatMessageReceived(const QString& text, const Communication::ChatSessionParticipantInterface& participant)
	{
		QString message = "OpensimIM, public chat: ";
		message.append( participant.GetName() );
		message.append(" : ");
		message.append( text );
		LogDebug( message.toStdString() );
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

} // end of namespace: OpensimIM
