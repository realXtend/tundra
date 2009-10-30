#include "Connection.h"
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/StreamedMediaChannel>

namespace TelepathyIM
{
	Connection::Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials) : tp_connection_manager_(tp_connection_manager), name_("Gabble"), protocol_("jabber"), state_(STATE_INITIALIZING), friend_list_("friend list"), self_contact_(0)
	{
		CreateTpConnection(credentials);
	}

	void Connection::CreateTpConnection(const Communication::CredentialsInterface &credentials)
	{
		QVariantMap params;

		params.insert("account", credentials.GetUserID());
		params.insert("password", credentials.GetPassword());
		params.insert("server", credentials.GetServer());
		params.insert("port", QVariant( (unsigned int)credentials.GetPort() ));
		
		Tp::PendingConnection *pending_connection = tp_connection_manager_->requestConnection(credentials.GetProtocol(), params);
		QObject::connect(pending_connection, SIGNAL( finished(Tp::PendingOperation *) ), SLOT( OnConnectionCreated(Tp::PendingOperation *) ));

		server_ = credentials.GetServer();
		user_id_ = credentials.GetUserID();
	}

	Connection::~Connection()
	{
		if (!tp_connection_.isNull())
			Tp::PendingOperation* op = tp_connection_->requestDisconnect();
		 // we do NOT connect the signal because the connection object doesn't exist when signal is emitted

		for (ChatSessionVector::iterator i = private_chat_sessions_.begin(); i != private_chat_sessions_.end(); ++i)
		{
			(*i)->Close();
			SAFE_DELETE(*i);
		}
		private_chat_sessions_.clear();

		for (ChatSessionVector::iterator i = public_chat_sessions_.begin(); i != public_chat_sessions_.end(); ++i)
		{
			(*i)->Close();
			SAFE_DELETE(*i);
		}
		public_chat_sessions_.clear();

		for (FriendRequestVector::iterator i = received_friend_requests_.begin(); i != received_friend_requests_.end(); ++i)
		{
			SAFE_DELETE(*i);
		}
		received_friend_requests_.clear();

		for (OutgoingFriendRequestVector::iterator i = sent_friend_requests_.begin(); i != sent_friend_requests_.end(); ++i)
		{
			SAFE_DELETE(*i);
		}
		sent_friend_requests_.clear();

		for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
		{
			SAFE_DELETE(*i);
		}
		contacts_.clear();
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
		return user_id_;
	}

	QString Connection::GetReason() const
	{
		return reason_;
	}

	Communication::ContactGroupInterface& Connection::GetContacts()
	{
		return friend_list_;
	}

	QStringList Connection::GetPresenceStatusOptionsForContact() const
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");

		QStringList options;
		Tp::SimpleStatusSpecMap map = tp_connection_->allowedPresenceStatuses();
		for (Tp::SimpleStatusSpecMap::iterator i = map.begin(); i != map.end(); ++i)
		{
			QString o = i.key();
			options.append(o);
		}
		return options;
	}

	QStringList Connection::GetPresenceStatusOptionsForUser() const
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");

		QStringList options;
		Tp::SimpleStatusSpecMap map = tp_connection_->allowedPresenceStatuses();
		for (Tp::SimpleStatusSpecMap::iterator i = map.begin(); i != map.end(); ++i)
		{
			QString o = i.key();
			if ( o.compare("offline") == 0 || o.compare("unknown") == 0 || o.compare("error") == 0 )
				continue; // HACK: Gabble crash if presence status is set to 'offline', 'unknown' or 'error'
			options.append(o);
		}
		return options;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");

		ChatSession* session = new ChatSession(*self_contact_, (Contact&)contact, tp_connection_);
		private_chat_sessions_.push_back(session);
		return session;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString& user_id)
	{
		//! @todo IMPLEMENT
		throw Core::Exception("NOT IMPLEMENTED");
	}

	Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel_id)
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");

		ChatSession* session = new ChatSession(channel_id, tp_connection_);
		public_chat_sessions_.push_back(session);
		return session;
	}

	Communication::VoiceSessionInterface* Connection::OpenVoiceSession(const Communication::ContactInterface &contact)
	{
		VoiceSession* session = new VoiceSession(dynamic_cast<Contact const*>(&contact)->GetTpContact());
		voice_sessions_.push_back(session);
		return session;
	}

	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");
		OutgoingFriendRequest* request = new OutgoingFriendRequest(target, message, tp_connection_);
		sent_friend_requests_.push_back(request);
		connect(request, SIGNAL( Error(OutgoingFriendRequest*) ), SLOT( OnSendingFriendRequestError(OutgoingFriendRequest*) ));
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		if (state_ != STATE_OPEN)
			throw Core::Exception("Connection is not open.");

		Communication::FriendRequestVector requests;
		for (FriendRequestVector::const_iterator i = received_friend_requests_.begin(); i != received_friend_requests_.end(); ++i)
		{
			requests.push_back(*i);
		}
		return requests;
	}

	void Connection::SetPresenceStatus(const QString &status)
	{
		if (state_ != STATE_OPEN )
			throw Core::Exception("Connection is not open.");

		presence_status_ = status;
		Tp::PendingOperation* op = tp_connection_->setSelfPresence(presence_status_,presence_message_);
		//! @todo Check success
	}

	void Connection::SetPresenceMessage(const QString &message)
	{
		if (state_ != STATE_OPEN )
			throw Core::Exception("Connection is not open.");

		presence_message_ = message;
		Tp::PendingOperation* op = tp_connection_->setSelfPresence(presence_status_,presence_message_);
		//! @todo Check success
	}

	void Connection::Close()
	{
		if ( tp_connection_.isNull() )
			return; // nothing to close

		Tp::PendingOperation* op = tp_connection_->requestDisconnect();
		connect(op, SIGNAL( finished(Tp::PendingOperation*) ), SLOT( OnConnectionClosed(Tp::PendingOperation*) ));
	}

	void Connection::OnConnectionCreated(Tp::PendingOperation *op)
	{
	    if (op->isError())
		{
			state_ = STATE_ERROR;
			reason_ = op->errorMessage();
			emit( ConnectionError(*this) );
			return;
		}
		
		Tp::PendingConnection *c = qobject_cast<Tp::PendingConnection *>(op);
		tp_connection_ = c->connection();

		QObject::connect(tp_connection_->requestConnect(),
					     SIGNAL(finished(Tp::PendingOperation *)),
						 SLOT(OnConnectionConnected(Tp::PendingOperation *)));

		QObject::connect(tp_connection_.data(),
			             SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
						 SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));
	}

	void Connection::OnConnectionConnected(Tp::PendingOperation *op)
	{
		if (op->isError())
		{
			state_ = STATE_ERROR;
			reason_ = op->errorMessage();
			emit( ConnectionError(*this) );
			return;
		}

		Tp::Features features;
		features.insert(Tp::Connection::FeatureSimplePresence);
		features.insert(Tp::Connection::FeatureRoster);
		features.insert(Tp::Connection::FeatureSelfContact);
		features.insert(Tp::Connection::FeatureCore);
		if ( !tp_connection_->isReady(features) )
		{
			LogDebug("Waiting for these features:");
			if ( !tp_connection_->isReady(Tp::Connection::FeatureSimplePresence) )
				LogDebug("  * SimplePresence ");
			if ( !tp_connection_->isReady(Tp::Connection::FeatureRoster) )
				LogDebug("  * Roster ");
			if ( !tp_connection_->isReady(Tp::Connection::FeatureSelfContact) )
				LogDebug("  * SelfContact ");
			if ( !tp_connection_->isReady(Tp::Connection::FeatureCore) )
				LogDebug("  * Core ");
		}
		QObject::connect(tp_connection_->becomeReady(features),
		                 SIGNAL(finished(Tp::PendingOperation *)),
						 SLOT(OnConnectionReady(Tp::PendingOperation *)));

	    if (tp_connection_->interfaces().contains(TELEPATHY_INTERFACE_CONNECTION_INTERFACE_CAPABILITIES))
		{
			Tp::CapabilityPair capability = {
				TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA,
				Tp::ChannelMediaCapabilityAudio |
				Tp::ChannelMediaCapabilityVideo |
                Tp::ChannelMediaCapabilityNATTraversalSTUN |
                Tp::ChannelMediaCapabilityNATTraversalGTalkP2P
			};
        
			tp_connection_->capabilitiesInterface()->AdvertiseCapabilities( Tp::CapabilityPairList() << capability, QStringList());
		}

		if( tp_connection_->interfaces().contains(TELEPATHY_INTERFACE_CONNECTION_INTERFACE_REQUESTS) )
		{
			QObject::connect(tp_connection_->requestsInterface(),
                SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
                SLOT(OnNewChannels(const Tp::ChannelDetailsList&)));
		}
	}

	void Connection::OnConnectionReady(Tp::PendingOperation *op)
	{
	    if (op->isError())
		{
			state_ = STATE_ERROR;
			reason_ = op->errorMessage();
			emit( ConnectionError(*this) );
			return;
		}

		self_contact_ = new Contact(tp_connection_->selfContact());

		connect(tp_connection_.data(), SIGNAL( statusChanged(uint, uint) ), SLOT( OnTpConnectionStatusChanged(uint, uint) ));
		connect(tp_connection_->contactManager(), SIGNAL( presencePublicationRequested(const Tp::Contacts &) ), SLOT( OnPresencePublicationRequested(const Tp::Contacts &) ));
		HandleAllKnownTpContacts();

		presence_status_ = tp_connection_->selfContact()->presenceStatus();
		presence_message_ = tp_connection_->selfContact()->presenceMessage();

		state_ = STATE_OPEN;
		emit( ConnectionReady(*this) );
	}

	void Connection::OnTpConnectionStatusChanged(uint new_status, uint reason)
	{
		switch (new_status)
		{
		case Tp::Connection::StatusConnected:
			break;
		case Tp::Connection::StatusUnknown:
			break;
		case Tp::Connection::StatusDisconnected:
			break;
		case Tp::Connection::StatusConnecting:
			break;
		}
		
	}

	void Connection::HandleAllKnownTpContacts()
	{
		ContactVector new_contacts;

		//! Check every known telepathy contact and determinate their nature by state of subscribtionState and publistState values
		//! Combinations are:
		//!                                  subscription:   publish:
		//! - Normal contact                 YES             YES
		//! - friend request (sended)        
		//! - friend request (received)      ASK             YES
		//! - banned contact                 NO              *
		//! - unknow                         (all the other combinations)
		foreach (const Tp::ContactPtr &tp_contact, tp_connection_->contactManager()->allKnownContacts())
		{
			Contact& contact = GetContact(tp_contact);

			switch ( tp_contact->subscriptionState() )
			{
			case Tp::Contact::PresenceStateNo:
				// User have already make a decicion to not accpet this contact to the a part of the friend list..
				break;

			case Tp::Contact::PresenceStateYes:
				{
					// A friend list item
					switch ( tp_contact->publishState() )
					{
					case Tp::Contact::PresenceStateNo:
						{
						//! We have subsribed presence status of this contact
						//! but we have not published our own presence!
						//! -> We unsubsribe this contact
						Tp::PendingOperation* pending_remove_subscription = tp_contact->removePresenceSubscription();
						//! check result of this
						}
						break;

					case Tp::Contact::PresenceStateYes:
						//! This is a normal state 
						new_contacts.push_back(&contact);
						break;

					case Tp::Contact::PresenceStateAsk:
						//! We have subscribed presence of this contact
						//! but we don't publish our?
						//! Publicity level should be same to the both directions
						Tp::PendingOperation* op = tp_contact->authorizePresencePublication("");
						new_contacts.push_back(&contact);
						//! todo: check the end result of this operation
						break;
					}
				}
				break;

			case Tp::Contact::PresenceStateAsk:
				{
					// User have not yet made the decision to accept or reject presence subscription 
					// So we create a FriendRequest obeject

					//! @todo IMPLEMENT

					//FriendRequest* request = new FriendRequest(contact);
					//received_friend_requests_.push_back(request);
					//emit ReceivedFriendRequest(request);
				}
				break;
			}
        }

		if ( new_contacts.size() > 0 )
		{
			for (ContactVector::iterator i = new_contacts.begin(); i != new_contacts.end(); ++i)
			{
				friend_list_.AddContact(*i);
			}
			//! @todo emit signal about contact list change
		}
	}

	void Connection::OnNewChannels(const Tp::ChannelDetailsList& channels)
	{
		foreach (const Tp::ChannelDetails &details, channels) 
		{
			QString channelType = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType")).toString();
			bool requested = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".Requested")).toBool();

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT && !requested)
			{
				LogDebug("Text chat request received.");
				Tp::TextChannelPtr tp_text_channel = Tp::TextChannel::create(tp_connection_, details.channel.path(), details.properties);
				LogDebug("Text channel object created.");
				
				ChatSession* session = new ChatSession(*self_contact_, tp_text_channel);
				private_chat_sessions_.push_back(session);
				connect(session, SIGNAL( Ready(ChatSession*) ), SLOT( IncomingChatSessionReady(ChatSession*) ));
			}

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_CONTACT_LIST && !requested)
			{
				LogDebug("Contact list channel");
			}

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA && !requested)
			{
				LogDebug("Voice chat hat request received.");
				Tp::StreamedMediaChannelPtr tp_streamed_media_channel = Tp::StreamedMediaChannel::create(tp_connection_, details.channel.path(), details.properties);
				
				if ( !tp_streamed_media_channel->initiatorContact().isNull() )
				{
					Contact &initiator = GetContact(tp_streamed_media_channel->initiatorContact());
					//! @todo get the actual contact object
					VoiceSession* session = new VoiceSession(tp_streamed_media_channel);
					voice_sessions_.push_back(session);
					emit( VoiceSessionReceived(*session) );
				}
				else
				{
					//Contact* null_contact = new Contact(tp_text_channel->initiatorContact());
					//ChatSession* session = new ChatSession(*null_contact, tp_text_channel);
					//private_chat_sessions_.push_back(session);
					//emit( ChatSessionReceived(*session) );
				}
			}
		}
	}

	void Connection::OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
	{
		//! We don't have to report about error here.
		//! OnConnectionReady() will be called after this with error information

		//state_ = STATE_ERROR;
		//reason_ = errorMessage;
		//emit( ConnectionError(*this) );
	}

	void Connection::OnConnectionClosed(Tp::PendingOperation *op)
	{
		state_ = STATE_CLOSED;
		emit( ConnectionClosed(*this) );
	}

	void Connection::OnPresencePublicationRequested(const Tp::Contacts &contacts)
	{
		//! @todo IMPLEMENT
		//! * check the status of the contact object
		//! * create a FriendRequest object
		//! * emit signal about friend request
	}

	Contact& Connection::GetContact(Tp::ContactPtr tp_contact)
	{
		for (ContactVector::iterator i = contacts_.begin(); i != contacts_.end(); ++i)
		{
			if ((*i)->GetID().compare(tp_contact->id()) == 0)
				return *(*i);
		}
		Contact* c = new Contact(tp_contact);
		contacts_.push_back(c);
		return *c;
	}

	void Connection::OnSendingFriendRequestError(OutgoingFriendRequest* request)
	{
		QString message = "Cannot send a friend request to ";
		LogError(message.toStdString());
	}

	void Connection::IncomingChatSessionReady(ChatSession* session)
	{
		emit( ChatSessionReceived(*session) );
	}

} // end of namespace: TelepathyIM
