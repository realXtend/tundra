#ifndef incl_Communication_TelepathyIM_ConnectionProvider_h
#define incl_Communication_TelepathyIM_ConnectionProvider_h

#include <QProcess>
#include <QStringList>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/PendingOperation>
#include "Foundation.h"
#include "../interface.h"
#include "Connection.h"

namespace TelepathyIM
{
	/**
	 *  This class is only used by CommunicationService object. Do not use this
	 *  directly!
	 *
	 *  Offers IM functionaly using Telepathy framework
	 *
     *  CURRENT IMPLEMENTATION ONLY SUPPORTS GABBLE CONNECTION MANAGER AND JABBER PROTOCOL
	 *  @todo Add support to all protocols Telepathy framework offers through dbus
	 *
	 */
	class ConnectionProvider : public  Communication::ConnectionProviderInterface
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality
	public:
		enum State { STATE_INITIALIZING, STATE_READY, STATE_ERROR };

		//! Initialize the connection provider.
		//! Start dbus daemon and 
		ConnectionProvider(Foundation::Framework* framework);

		virtual ~ConnectionProvider();

		//! Provides list of supported protocols by this provider
		virtual QStringList GetSupportedProtocols() const;

		//! Open a new connection to IM server woth given credentials
		virtual Communication::ConnectionInterface* OpenConnection(const Communication::CredentialsInterface& credentials);

		//! Provides all Connections objects created by this provider
		virtual Communication::ConnectionVector GetConnections() const;

	private:
		//! Start the WinDBus in a QProcess.
		//! We call this only on Windows platform where are no native dbus daemon service
		void StartDBusDaemon();

		//! Stop the WinDBus QProcess
		//! We call this only on Windows platform where are no native dbus daemon service
		void StopDBusDaemon();

		//! Do all initialize steps for Telepathy connection manager
		//! and start waiting for OnConnectionManagerReady signal
		//! @param name The name of the connection manager eg. "gabble"
		void InitializeTelepathyConnectionManager(const QString &name);

		//! Delete all connection objects
		void DeleteConnections();

    private:
		State state_;

		Foundation::Framework* framework_;

		//! ConnectionManager object from Telepathy-Qt4 library
		Tp::ConnectionManagerPtr tp_connection_manager_;

		//! dbus daemon and gabble process for Windows platform
		QProcess* dbus_daemon_;

        //! All connections (open and closed ones)
		ConnectionVector connections_;

		//! The port for dbus daemon service to run
		static const int DBUS_SESSION_PORT_ = 12434;
		QStringList supported_protocols_;

	private slots:
		void OnConnectionManagerReady(Tp::PendingOperation *op);
		void OnDBusDaemonStdout();
		void OnDBusDaemonExited(int);
		void OnConnectionReady(Communication::ConnectionInterface&);
		void OnConnectionClosed(Communication::ConnectionInterface&);
		void OnConnectionError(Communication::ConnectionInterface&);
	};

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_ConnectionProvider_h
