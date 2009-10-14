#include "Connection.h"
#include "StableHeaders.h"
#include "RexLogicModule.h" // chat
#include "RexProtocolMsgIDs.h"

namespace OpensimIM
{

	Connection::Connection(Foundation::Framework* framework): framework_(framework), name_(""), protocol_(OPENSIM_IM_PROTOCOL), server_(""), reason_("")
	{
		RequestFriendlist();
		RegisterConsoleCommands();
		OpenWorldChatSession();
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
		//! \todo IMPELEMENT
		return NULL;
	}

		
	QStringList Connection::GetAvailablePresenceStatusOptions() const
	{
		//! \todo IMPLEMENT
		return QStringList();
	}

	Communication::ChatSessionInterface* Connection::OpenChatSession(const Communication::ContactInterface &contact)
	{
		//! \todo IMPLEMENT
		return NULL;
	}

	Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel)
	{
		for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
		{
			if ( (*i)->GetID().compare( channel ) == 0 )
			{
				return (*i);
			}
		}

		ChatSession* session = new ChatSession(framework_, channel);
		public_chat_sessions_.push_back(session);
		return session;
	}
	
	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		//! \todo IMPLEMENT

		//! UDP messages:
		//! + AcceptFriendship
		//! + DeclineFriendship
		//! + FormFriendship
		//! + TerminateFriendship
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		//! \todo IMPLEMENT
		return Communication::FriendRequestVector();
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
		//! TODO: Parse login response from opensim server
		//! buddy-list
		//! + array:  
		//!     + buddy_id
		//!     + buddy_rights_given
		//!     + buddy_rights_has
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
		case RexNetMsgChatFromSimulator: HandleOSNEChatFromSimulator(*msg);
            return true;
		}

		return false;
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

} // end of namespace: OpensimIM
