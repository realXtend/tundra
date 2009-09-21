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
		

		this->tp_connection_manager_ = Tp::ConnectionManager::create(CONNECTION_MANAGER_NAME);


		Tp::PendingReady* p = this->tp_connection_manager_->becomeReady();

		QObject::connect(p,
				SIGNAL(finished(Tp::PendingOperation *)),
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

		for (ConnectionVector::iterator i = connections_.begin(); i != connections_.end(); ++i)
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
		QString user_name = "realxtend@jabber.org";
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
		Tp::PendingConnection *pending_connection = tp_connection_manager_->requestConnection(IM_PROTOCOL, params);
		QObject::connect(pending_connection, SIGNAL(finished(Tp::PendingOperation *)),
				(QObject*)connection, SLOT(OnConnectionCreated(Tp::PendingOperation *)));


		return connection;
	}

	ConnectionVector CommunicationManager::GetAllConnections()
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