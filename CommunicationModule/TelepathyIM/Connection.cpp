#include "Connection.h"
#include <TelepathyQt4/ContactManager>

namespace TelepathyIM
{
	Connection::Connection(Tp::ConnectionManagerPtr tp_connection_manager, const Communication::CredentialsInterface &credentials) : tp_connection_manager_(tp_connection_manager), name_("Gabble"), protocol_("jabber"), state_(STATE_INITIALIZING)
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
		
		//std::string message = "Try to open connection to IM server: ";
		//message.append( server.toStdString () );
		//LogInfo(message);
		Tp::PendingConnection *pending_connection = tp_connection_manager_->requestConnection(credentials.GetProtocol(), params);
		QObject::connect(pending_connection, SIGNAL( finished(Tp::PendingOperation *) ), SLOT( OnConnectionCreated(Tp::PendingOperation *) ));
		server_ = credentials.GetServer();
	}

	Connection::~Connection()
	{
		if (!tp_connection_.isNull())
			tp_connection_->requestDisconnect();
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

	Communication::ContactGroupInterface& Connection::GetContacts()
	{
		return friend_list_;
	}

	QStringList Connection::GetPresenceStatusOptionsForContact() const
	{
		//! @todo IMPLEMENT
		QStringList empty;
		return empty;
	}

	QStringList Connection::GetPresenceStatusOptionsForSelf() const
	{
		//! @todo IMPLEMENT
		QStringList empty;
		return empty;
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const Communication::ContactInterface &contact)
	{
		//! @todo IMPLEMENT
		throw Core::Exception("NOT IMPLEMENTED");
	}

	Communication::ChatSessionInterface* Connection::OpenPrivateChatSession(const QString& user_id)
	{
		//! @todo IMPLEMENT
		throw Core::Exception("NOT IMPLEMENTED");
	}

	Communication::ChatSessionInterface* Connection::OpenChatSession(const QString &channel)
	{
		//! @todo IMPLEMENT
		throw Core::Exception("NOT IMPLEMENTED");
	}

	void Connection::SendFriendRequest(const QString &target, const QString &message)
	{
		//! @todo IMPLEMENT
	}

	Communication::FriendRequestVector Connection::GetFriendRequests() const
	{
		//! @todo IMPLEMENT
		Communication::FriendRequestVector requests;
		return requests;
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
		QObject::connect(tp_connection_->becomeReady(features),
		                 SIGNAL(finished(Tp::PendingOperation *)),
						 SLOT(OnConnectionReady(Tp::PendingOperation *)));

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

		connect(tp_connection_->contactManager(), SIGNAL( presencePublicationRequested(const Tp::Contacts &) ), SLOT( OnPresencePublicationRequested(const Tp::Contacts &) ));
		HandleAllKnownTpContacts();

		state_ = STATE_OPEN;
		emit( ConnectionReady(*this) );
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
		foreach (const Tp::ContactPtr &contact, tp_connection_->contactManager()->allKnownContacts())
		{
			switch ( contact->subscriptionState() )
			{
			case Tp::Contact::PresenceStateNo:
				// User have already make a decicion to not accpet this contact to the a part of the friend list..
				break;

			case Tp::Contact::PresenceStateYes:
				{
					// A friend list item
					switch ( contact->publishState() )
					{
					case Tp::Contact::PresenceStateNo:
						{
						//! We have subsribed presence status of this contact
						//! but we have not published our own presence!
						//! -> We unsubsribe this contact
						Tp::PendingOperation* pending_remove_subscription = contact->removePresenceSubscription();
						//! check result of this
						}
						break;

					case Tp::Contact::PresenceStateYes:
						//! This is a normal state 
						break;

					case Tp::Contact::PresenceStateAsk:
						//! We have subscribed presence of this contact
						//! but we don't publish our?
						//! Publicity level should be same to the both directions
						Tp::PendingOperation* op = contact->authorizePresencePublication("");
						//! todo: check the end result of this operation
						break;
					}

					Contact* c = new Contact(contact);
					new_contacts.push_back(c);
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
				contacts_.push_back(*i);
				friend_list_.AddContact(*i);
			}
			//! @todo emit signal about contact list change
		}
	}

	void Connection::OnNewChannels(const Tp::ChannelDetailsList& details)
	{
		//! @todo IMPLEMENT
	}

	void Connection::OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
	{
		state_ = STATE_ERROR;
		reason_ = errorMessage;
		emit( ConnectionError(*this) );
		//! @todo IMPLEMENT
	}

	void Connection::OnConnectionClosed(Tp::PendingOperation *op)
	{
		state_ = STATE_CLOSED;
		emit( ConnectionClosed(*this) );
	}

	void Connection::OnPresencePublicationRequested(const Tp::Contacts &contacts)
	{
		//! @todo IMPLEMENT
	}

} // end of namespace: TelepathyIM
