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
			// TODO: Error handling
			state_ = STATE_ERROR;
			return;
		}
  
		Tp::PendingConnection *c = qobject_cast<Tp::PendingConnection *>(op);
		tp_connection_ = c->connection();
		
		//conn->requestsInterface(
	    //conn->requestConnect(
		//conn->gotInterfaces
		QObject::connect(tp_connection_->requestConnect(),
            SIGNAL(finished(Tp::PendingOperation *)),
			this,
            SLOT(onConnectionConnected(Tp::PendingOperation *)));
		QObject::connect(tp_connection_.data(),
			SIGNAL(invalidated(Tp::DBusProxy *, const QString &, const QString &)),
			this,
			SLOT(onConnectionInvalidated(Tp::DBusProxy *, const QString &, const QString &)));
	}


	void Connection::OnConnectionConnected(Tp::PendingOperation *op)
	{

	}

	void Connection::OnConnectionInvalidated(Tp::PendingOperation *op)
	{

	}

	CommunicationManager::CommunicationManager(): state_(STATE_INITIALIZING), dbus_daemon_(NULL)
	{
		std::string bus_name = "org.freedesktop.Telepathy.ConnectionManager.";
		bus_name.append(CONNECTION_MANAGER_NAME);
		if ( !IsDBusServiceAvailable(bus_name) )
		{
#ifdef WIN32
			StartDBusDaemon();
#endif
			if ( !IsDBusServiceAvailable(bus_name) )
			{
				state_ = STATE_ERROR;
				std::string reason = "Cannot initialize CommunicationManager: DBus service (";
				reason.append(bus_name);
				reason.append(") is not available.");
				LogError(reason);
				return;
			}
		}

//		instance_ = this;
		//Framework::GetApplicationMainWindowQWidget();
		qDebug() << "TpQt4 Thread:" << this->thread(); 

		QThread* ct = QThread::currentThread();
		QThread* t = this->thread();

		Tp::registerTypes();
		Tp::enableDebug(true);
		Tp::enableWarnings(true);
		//pending_ready_ = NULL;
		this->connection_manager_ = Tp::ConnectionManager::create(CONNECTION_MANAGER_NAME);
		Tp::PendingReady* p = this->connection_manager_->becomeReady();
//		this->pending_ready_ = p;
		//QThread* t = this->thread();
//		connect(this->pending_ready_perse,
		QObject::connect(p,
				SIGNAL(finished(Tp::PendingOperation *)),
				this,
				SLOT(OnConnectionManagerReady(Tp::PendingOperation *)));
	}

	CommunicationManager::~CommunicationManager()
	{
		// todo: close connection
		if (dbus_daemon_)
			dbus_daemon_->close();
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
		if (op->isError())
		{
			LogError("Cannot initialize ConnectionManager.");
			state_ = STATE_ERROR;
//			LogError("Cannot open connection to Telepathy ConnectionManager");
			return;
		}
		LogInfo("ConnectionManager is ready.");
		state_ = STATE_READY;
	}

	Connection* CommunicationManager::OpenConnection(const Credentials &credentials)
	{
		if (!connection_manager_->isReady())
		{
			QDBusConnection dbus_conn = connection_manager_->dbusConnection();
		}

		if (state_ != STATE_READY)
		{
			LogError("Cannot create IM connection because ConnectionManger is not ready.");
			return NULL;
		}

		Connection* connection = new Connection();
		connections_.push_back(connection);

		QVariantMap params;
		QString user_name = "kuonanoja";
		QString pass_word = "jabber666";
		QString server = "jabber.org";
		QString port = "5222";

		params.insert("account", QVariant(user_name));
		params.insert("password", QVariant(pass_word));
		params.insert("server", QVariant(server));
		params.insert("port", QVariant(port));

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
		// name.c_str()
		QString bus_name("");
		QDBusConnection conn = QDBusConnection::connectToBus(QDBusConnection::SessionBus, bus_name);
		if (conn.isConnected())
		{
			conn.disconnectFromBus(bus_name);
			return true;
		}
		else
			return false;
	}

#ifdef WIN32
	void CommunicationManager::StartDBusDaemon()
	{
		QString path = "startdbus.bat";
		dbus_daemon_ = new QProcess(this);
//		dbus_daemon_->addArgument( path );

		connect( dbus_daemon_, SIGNAL(readyReadStandardOutput()),
            this, SLOT(OnDBusDaemonStdout()) );
		connect( dbus_daemon_, SIGNAL(finished(int exitCode, QProcess::ExitStatus exitStatus)),
            this, SLOT(OnDBusDaemonExited()) );

		dbus_daemon_->start(path);
	}

	void CommunicationManager::OnDBusDaemonStdout()
	{
		QByteArray buffer= dbus_daemon_->readAllStandardOutput();
	}

	void CommunicationManager::OnDBusDaemonExited( int exitCode, QProcess::ExitStatus exitStatus )
	{
	}
#endif		

} // namespace TpQt4Communication