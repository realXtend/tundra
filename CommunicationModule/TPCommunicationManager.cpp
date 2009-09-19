#include "StableHeaders.h"
#include "Foundation.h"

#include "TPCommunicationManager.h"
#include <QtDbus>
#include <QtCore>
#include <QProcess>
#include <QStringList>

namespace TpQt4Communication
{
	CommunicationManager* CommunicationManager::instance_ =  NULL; // static member function initialization

	PresenceStatus::PresenceStatus(): status_text_(""), message_text_("")
	{

	}

	void PresenceStatus::SetStatusText(std::string text)
	{
		status_text_ = text;
	}

	std::string PresenceStatus::GetStatusText()
	{
		return status_text_;
	}

	void PresenceStatus::SetMessageText(std::string text)
	{
		message_text_ = text;
	}

	std::string PresenceStatus::GetMessageText()
	{
		return message_text_;
	}

	Message::Message(std::string text)
	{
		text_ = text;
		author_ = "";
		time_stamp_ = QTime::currentTime();
	}

	void TextChatSession::Invite(Address a)
	{

	}

	TextChatSession::TextChatSession()
	{

	}

	void TextChatSession::SendTextMessage(std::string text)
	{
		Message* m = new Message(text);
		messages_.push_back(m);
	}

	MessageVector TextChatSession::GetMessageHistory()
	{
		return messages_;
	}

	void TextChatSession::Close()
	{
		// todo: 
	}


	Contact::Contact(Tp::Contact* tp_contact)
	{
		LogInfo("Create Contact object");
		tp_contact_ = tp_contact;
		ConnectSignals();
	}

	void Contact::ConnectSignals()
	{
		QObject::connect(tp_contact_,
            SIGNAL(simplePresenceChanged(const QString &, uint, const QString &)),
            SLOT(OnContactChanged()));
		QObject::connect(tp_contact_,
            SIGNAL(subscriptionStateChanged(Tp::Contact::PresenceState)),
            SLOT(OnContactChanged()));
		QObject::connect(tp_contact_,
            SIGNAL(publishStateChanged(Tp::Contact::PresenceState)),
            SLOT(OnContactChanged()));
		QObject::connect(tp_contact_,
            SIGNAL(blockStatusChanged(bool)),
            SLOT(OnContactChanged()));
	}

	void Contact::OnContactChanged()
	{
		LogInfo("Contact state changed");
	}

	User::User(Tp::ConnectionPtr tp_connection): user_id_(""), protocol_(""), tp_connection_(tp_connection)
	{
		tp_contact_ = tp_connection->selfContact();
	}

	void User::SetPresenceStatus(std::string status, std::string message)
	{
		QString s(status.c_str());
		QString m(message.c_str());
		tp_connection_->setSelfPresence(s, m);
	}

	std::string User::GetUserID()
	{
		return user_id_;
	}

	PresenceStatus* User::GetPresenceStatus()
	{
		return &presence_status_;
	}

	std::string User::GetProtocol()
	{
		return protocol_;
	}

	Credentials::Credentials()
	{

	}

	void Credentials::LoadFromFile(std::string path)
	{

	}

	std::string Credentials::GetProtocol() const 
	{
		return protocol_;
	}

	std::string Credentials::GetServer() const
	{
		return server_;
	}

	Connection::Connection(const Credentials &credentials) : user_(NULL), state_(STATE_CONNECTING), id_(""), protocol_( credentials.GetProtocol() ), server_( credentials.GetServer() ) , tp_connection_(NULL)
	{
	}

	Connection::~Connection()
	{
		tp_connection_->requestDisconnect();
	}

	void Connection::Close()
	{
		
		if (state_ != STATE_OPEN )
		{
			std::string message = "Connection is not open.";
			LogError(message);
			return;
		}

		LogError("Try to disconnect IM server connection.");
		Tp::PendingOperation* p = tp_connection_->requestDisconnect();
		// TODO: connect slots
	}
	
	Connection::State Connection::GetState()
	{
		return state_;
	}

	std::string Connection::GetID()
	{
		return id_;
	}

	User* Connection::GetUser()
	{
		return user_;
	}

	std::string Connection::GetProtocol()
	{
		return protocol_;
	}

	TextChatSessionPtr Connection::CreateTextChatSession()
	{
		QVariantMap params;

		Tp::PendingChannel* pending_channel = tp_connection_->createChannel(params);
		Tp::ChannelPtr c = pending_channel->channel();
		
		TextChatSession* session = new TextChatSession();
		return TextChatSessionPtr(session);
	}

	void Connection::OnConnectionCreated(Tp::PendingOperation *op)
	{
	    if (op->isError())
		{
			std::string message = "Cannot connect to IM server";
			message.append( op->errorMessage().toStdString() );
			LogError(message);
//			error_message_.append( op->errorMessage() );
			// TODO: Error handling
			state_ = STATE_ERROR;
			return;
		}
		
		Tp::PendingConnection *c = qobject_cast<Tp::PendingConnection *>(op);
		tp_connection_ = c->connection();

		std::string message = "Got response from IM server";
		LogInfo(message);
		
		//conn->requestsInterface(
		//conn->gotInterfaces
		QObject::connect(tp_connection_->requestConnect(), SIGNAL(finished(Tp::PendingOperation *)),SLOT(OnConnectionConnected(Tp::PendingOperation *)));
		QObject::connect(tp_connection_.data(), SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),	SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));


		QObject::connect(tp_connection_->requestsInterface(),
                SIGNAL(NewChannels(const Tp::ChannelDetailsList&)),
                SLOT(OnNewChannels(const Tp::ChannelDetailsList&)));
	}


	void Connection::OnConnectionConnected(Tp::PendingOperation *op)
	{
		std::string message = "Connection established successfully to IM server.";
		LogInfo(message);
		LogInfo("Create user.");
		user_ = new User(tp_connection_);
		state_ = STATE_OPEN;

		// Request friend list
		QObject::connect(tp_connection_->contactManager(),
            SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
            SLOT(OnPresencePublicationRequested(const Tp::Contacts &)));

		//QStringList list;
		
		Tp::Contacts contacts  = this->tp_connection_->contactManager()->allKnownContacts();
		
		
		for (Tp::Contacts::iterator i = contacts.begin(); i != contacts.end(); ++i)
		{
			Tp::ContactPtr c = *i;
			std::string id = c->id().toStdString();
			
			LogInfo("***");
			LogInfo(id);
			//Contact contact = new Contact(address, real_name);
			
		}

//		Tp::PendingContacts *pending_contacts = this->tp_connection_->contactManager()->->contactsForIdentifiers(list);
//		QObject::connect((QObject*)pending_contacts, SIGNAL(finished(Tp::PendingOperation *)),
//			SLOT(OnContactRetrieved(Tp::PendingOperation *)));
	}

	void Connection::OnNewChannels(const Tp::ChannelDetailsList& channels)
	{
		LogInfo("New channel received.");
		foreach (const Tp::ChannelDetails &details, channels) 
		{
			QString channelType = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".ChannelType")).toString();
			bool requested = details.properties.value(QLatin1String(TELEPATHY_INTERFACE_CHANNEL ".Requested")).toBool();
			// qDebug() << " channelType:" << channelType;
			//  qDebug() << " requested  :" << requested;

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_TEXT && !requested)
			{
				Tp::TextChannelPtr channel = Tp::TextChannel::create(tp_connection_, details.channel.path(), details.properties);
				//mCallHandler->addIncomingCall(channel);
				LogInfo("Text channel");
			}

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_CONTACT_LIST && !requested)
			{
				LogInfo("Contact list channel");
			}

			if (channelType == TELEPATHY_INTERFACE_CHANNEL_TYPE_STREAMED_MEDIA && !requested)
			{
				LogInfo("Streamed media channel");
			}
			
		}
	}

	void Connection::OnPresencePublicationRequested(const Tp::Contacts &contacts)
	{
		LogInfo("OnPresencePublicationRequested");
		// TODO: Create Friend request object ?
		foreach (const Tp::ContactPtr &contact, contacts)
		{
			LogInfo("****");
			//std::string id = contact.id().toStdString();
		}
	}

	void Connection::OnContactRetrieved(Tp::PendingOperation *op)
	{
		LogInfo("OnContactRetrieved");
		if (op->isError())
		{
			LogError("Failed to receive friendlist.");
			return;
		}
		LogInfo("Friendlist received.");
		Tp::PendingContacts *pcontacts = qobject_cast<Tp::PendingContacts *>(op);
		QList<Tp::ContactPtr> contacts = pcontacts->contacts();
		
		for (int i = 0; i < contacts.size(); ++i)
		{
			Tp::ContactPtr c = contacts[i];
			std::string id = c->id().toStdString();
			
			LogInfo("***");
			LogInfo(id);
			//Contact contact = new Contact(address, real_name);
			
		}
	}

	void Connection::OnConnectionInvalidated(Tp::DBusProxy *proxy, const QString &errorName, const QString &errorMessage)
	{
		LogError("Connection::OnConnectionInvalidated");
		state_ = STATE_ERROR;
	}

	CommunicationManager::CommunicationManager(): state_(STATE_INITIALIZING), dbus_daemon_(NULL)
	{
		std::string bus_name = "org.freedesktop.Telepathy.ConnectionManager.";
		bus_name.append(CONNECTION_MANAGER_NAME);
//		if ( !IsDBusServiceAvailable(bus_name) )
		{
#ifdef WIN32
			StartDBusDaemon();
#endif
			// HACK: DISABLED temmporarily
			//       Currently IsDBusServiceAvailable method can be only called once
			//       because implementation of QDBus
			//
			//if ( !IsDBusServiceAvailable(bus_name) )
			//{
			//	state_ = STATE_ERROR;
			//	std::string reason = "Cannot initialize CommunicationManager: DBus service (";
			//	reason.append(bus_name);
			//	reason.append(") is not available.");
			//	LogError(reason);
			//	return;
			//}
		}

		qDebug() << "TpQt4 Thread:" << this->thread(); 

		QThread* ct = QThread::currentThread();
		QThread* t = this->thread();

		Tp::registerTypes();
		Tp::enableDebug(true);
		Tp::enableWarnings(true);
		

		this->connection_manager_ = Tp::ConnectionManager::create(CONNECTION_MANAGER_NAME);


		Tp::PendingReady* p = this->connection_manager_->becomeReady();

		QObject::connect(p,
				SIGNAL(finished(Tp::PendingOperation *)),
//				this,
				SLOT(OnConnectionManagerReady(Tp::PendingOperation *)));

	}

	CommunicationManager::~CommunicationManager()
	{
		if (dbus_daemon_)
		{
			dbus_daemon_->kill(); // terminate seems not to be enough
			bool ok = dbus_daemon_->waitForFinished(1000);
			if (!ok)
			{
				LogError("Cannot terminate dbus daemon process.");
			}
			delete dbus_daemon_;
			dbus_daemon_ = NULL;
		}

		for (ConnectionList::iterator i = connections_.begin(); i != connections_.end(); ++i)
		{
			delete *i;
			*i = NULL;
		}
		connections_.clear();
	}

	// Static factory method for communication manager
	CommunicationManager* CommunicationManager::GetInstance()
	{
		if (instance_ == NULL)
		{
			instance_ = new CommunicationManager();
		}
		return instance_;
	}

	CommunicationManager::State CommunicationManager::GetState()
	{
		return state_;
	}

	void CommunicationManager::OnConnectionManagerReady(Tp::PendingOperation *op)
	{
		LogInfo( "ConnectionManager init ready..." );


		if (op->isError())
		{
			LogError( "result: ERROR" );
			std::string message = "Cannot initialize ConnectionManager: ";
			message.append( op->errorMessage().toStdString() );
			LogError( message );
			state_ = STATE_ERROR;
//			LogError("Cannot open connection to Telepathy ConnectionManager");
			return;
		}

		//return; // HACK

		LogInfo("ConnectionManager is ready.");
		state_ = STATE_READY;
	}

	Connection* CommunicationManager::OpenConnection(const Credentials &credentials)
	{
		if (state_ != STATE_READY)
		{
			LogError("Cannot create IM connection because ConnectionManger is not ready.");
			return NULL;
		}

		Connection* connection = new Connection(credentials);
		connections_.push_back(connection);

		QVariantMap params;
		QString user_name = "kuonanoja@jabber.org";
		QString pass_word = "jabber666";
		QString server = "jabber.org";
//		QIn port = 5222;

		params.insert("account", QVariant(user_name));
		params.insert("password", QVariant(pass_word));
		params.insert("server", QVariant(server));
//		params.insert("port", QVariant(port));

		std::string message = "Try to connecto to IM server: ";
		message.append( server.toStdString () );
		LogInfo(message);
		Tp::PendingConnection *pending_connection = connection_manager_->requestConnection(IM_PROTOCOL, params);
		QObject::connect(pending_connection, SIGNAL(finished(Tp::PendingOperation *)),
				(QObject*)connection, SLOT(OnConnectionCreated(Tp::PendingOperation *)));


		return connection;
	}

	ConnectionList CommunicationManager::GetAllConnections()
	{
		return connections_;
	}

	bool CommunicationManager::IsDBusServiceAvailable(std::string name)
	{
		std::string bus_name = "org.freedesktop.Telepathy.ConnectionManager.";
		bus_name.append(CONNECTION_MANAGER_NAME);

		QDBusConnection bus = QDBusConnection::sessionBus();
		//bus.connectToBus(QDBusConnection::SessionBus, bus_name.c_str());
		//if (bus.isConnected())
		//{
		//	bus.disconnectFromBus(bus_name.c_str());
		//}
		QString service_name = "org.realxtend.Naali.dbustest";
		if ( bus.registerService(service_name) )
		{
			bus.unregisterService(service_name);
			return true;
		}
		return false;

		// name.c_str()
//		QString bus_name(name.c_str());
//		QDBusConnection conn = QDBusConnection::connectToBus(QDBusConnection::SessionBus, bus_name);
		//if (bus.isConnected())
		//{
		//	bus.disconnectFromBus(bus_name);
		//	return true;
		//}
		//else
		//	return false;
	}

#ifdef WIN32
	void CommunicationManager::StartDBusDaemon()
	{
		QString path = "dbus\\dbus-daemon.exe --config-file=data\\session.conf";

		dbus_daemon_ = new QProcess(this);
		QStringList env = QProcess::systemEnvironment();
		env << "DBUS_SESSION_BUS_ADDRESS=tcp:host=localhost,port=12434";
		dbus_daemon_->setEnvironment(env);
		
		connect( dbus_daemon_, SIGNAL(readyReadStandardOutput()), this, SLOT(OnDBusDaemonStdout()) );
		connect( dbus_daemon_, SIGNAL(finished(int)), this, SLOT(OnDBusDaemonExited(int)) );

		dbus_daemon_->start(path);
		bool ok = dbus_daemon_->waitForStarted(2000);
		if (!ok)
		{
			state_ = STATE_ERROR;
			LogError("Cannot start dbus daemon process.");
			return;
		}

		// wait some time so that dbus daemon can start up
		QTime dieTime = QTime::currentTime().addSecs(2);
		while( QTime::currentTime() < dieTime )
			QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
	}

	void CommunicationManager::OnDBusDaemonStdout()
	{
		QByteArray buffer= dbus_daemon_->readAllStandardOutput();
	}

	void CommunicationManager::OnDBusDaemonExited( int exitCode )
	{

	}

#endif		

} // namespace TpQt4Communication