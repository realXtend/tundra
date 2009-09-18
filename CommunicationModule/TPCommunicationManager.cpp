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


	User::User(): user_id_(""), protocol_("")
	{

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

	Connection::Connection() : user_(NULL), state_(STATE_CLOSED), id_(""), protocol_(""), tp_connection_(NULL)
	{
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
		QObject::connect(tp_connection_->requestConnect(), SIGNAL(finished(Tp::PendingOperation *)),
//			this,
			SLOT(OnConnectionConnected(Tp::PendingOperation *)));
		QObject::connect(tp_connection_.data(), SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
		//	this,
		SLOT(OnConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));
	}


	void Connection::OnConnectionConnected(Tp::PendingOperation *op)
	{
		std::string message = "Connection established successfully to IM server.";
		LogInfo(message);
		LogInfo("Create user.");
		user_ = new User();
		state_ = STATE_OPEN;

		// Request friend list
		QObject::connect(tp_connection_->contactManager(),
            SIGNAL(presencePublicationRequested(const Tp::Contacts &)),
            SLOT(OnPresencePublicationRequested(const Tp::Contacts &)));

		//QStringList list;
		//Tp::PendingContacts *pending_contacts = this->tp_connection_->contactManager()->contactsForIdentifiers(list);
		//Tp::Contacts contacts  = this->tp_connection_->contactManager()->allKnownContacts();
		//
		//for (int i = 0; i < contacts.size(); ++i)
		//{
		//	Tp::ContactPtr c = contacts[i];
		//	std::string id = c->id().toStdString();
		//	
		//	LogInfo("***");
		//	LogInfo(id);
		//	//Contact contact = new Contact(address, real_name);
		//	
		//}

		//QObject::connect((QObject*)pending_contacts, SIGNAL(finished(Tp::PendingOperation *)),
		//	SLOT(OnContactRetrieved(Tp::PendingOperation *)));
	}

	void Connection::OnPresencePublicationRequested(const Tp::Contacts &contacts)
	{
		foreach (const Tp::ContactPtr &contact, contacts)
		{
			LogInfo("****");
			//std::string id = contact.id().toStdString();

		}

	}

	void Connection::OnContactRetrieved(Tp::PendingOperation *op)
	{
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

		Connection* connection = new Connection();
		connections_.push_back(connection);

		QVariantMap params;
		QString user_name = "@jabber.org";
		QString pass_word = "";
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
		QObject::connect(pending_connection,
				SIGNAL(finished(Tp::PendingOperation *)),
				(QObject*)connection,
				SLOT(OnConnectionCreated(Tp::PendingOperation *)));

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
		connect( dbus_daemon_, SIGNAL(finished(int exitCode)), this, SLOT(OnDBusDaemonExited(int exitCode)) );

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