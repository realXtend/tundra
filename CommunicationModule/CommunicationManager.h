#ifndef incl_Comm_CommunicationManager_h
#define incl_Comm_CommunicationManager_h

#include "StableHeaders.h"
#include "Foundation.h"
#include "ModuleInterface.h" // for logger

#include <QObject>
#include <QtGui>
#include <QSharedPointer>
#include <QTime>

#include <TelepathyQt4/Debug__>
#include <TelepathyQt4/Constants>
#include <TelepathyQt4/Types>
#include <TelepathyQt4/ConnectionManager>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/ContactManager>
#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingConnection>
#include <TelepathyQt4/PendingReady>
#include <TelepathyQt4/PendingContacts>
#include <TelepathyQt4/PendingChannel>


#include "Communication.h"
#include "Connection.h"
#include "PresenceStatus.h"
#include "Contact.h"
#include "ChatSession.h"
#include "ChatMessage.h"
#include "User.h"
#include "Contact.h"
#include "Credentials.h"
#include "VoipSession.h"


/**
 * Objects:
 * 
 * - CommunicationManager
 *   - connections
 * - Connection
 *   - sessions
 *   - user
 * - Session
 *   - participants
 * - ContactList(needed?)
 *   - contacts
 * - Contact
 *   - address
 *   - presence_status
 *   - real_name
 * - User
 *   - contact_list
 *   - presence_status
 * - Participant
 *   - address
 *   - message_history (needed?)
 *   
 *
 *
 * Events: All events are sent using framework's event system
 * - TextMessage(address_from, message)
 * - ChatMessage(session_id, address_from, message)
 * 
 * - ConnectionClosed(connection_id)
 *
 * - SessionClosed(session_id)
 * - ParticipantJoined(session_id, participant_id)
 * - ParticipantLeft(session_id, participant_id)
 *
 * - FriendRequest(address_from)
 * - FriendRequestAccepted(address_from)
 * - FriendRequestRejected(address_from)
 *
 * - PresenceUpdate(contact_id, status_text, message_text);
 */ 
namespace TpQt4Communication
{
	/**
	 *  Keeps all Connection objects
	 *  Starts WinDBus id dbus service isn't available on Windows platform
	 *	
	 */
	class CommunicationManager : QObject
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
	private:
		//! Default construnctor
		CommunicationManager();
		
	public:
		static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality
		enum State {STATE_INITIALIZING, STATE_READY, STATE_ERROR};

		//! Static Get singleton object
		static CommunicationManager* GetInstance();

		//! Get state of ConnectionMnager object
		State GetState();

		//! Construnctor
		~CommunicationManager();
		
		//! Open a new connection to IM server
		Connection* OpenConnection(const Credentials &credentials);

		//! Return all Connection objects made by calling OpenConnection
		ConnectionVector GetAllConnections();

		//ConnectionVector GetConnections(std::string protocol);

		//! Close all Connection objects
		void CloseAllConnections();

	private:

		//! return true if DBus service is available 
		//! NOTE: because current QDBus implementation this cannot be uset at the moment.
		//!       (If return false it cannot be used after QApplication is restarted)
		bool IsDBusServiceAvailable(std::string name);

		//! Starts WinDBus process
		void StartDBusDaemon(int port);
		
		//! ConnectionManager object from Telepathy-Qt4 library
		Tp::ConnectionManagerPtr tp_connection_manager_;

		//!
		//bool connection_manager_ready_;
		
		//! Connections to IM server (currently we only open one)
		ConnectionVector connections_;

		//!
		//Tp::PendingReady* pending_ready_;

		//! 
		State state_;

		//! dbus daemon and gabble process for Windows platform
		QProcess* dbus_daemon_;

//	public Q_SIGNALS:

	private Q_SLOTS:
		void OnConnectionManagerReady(Tp::PendingOperation *op);
		void OnDBusDaemonStdout();
		void OnDBusDaemonExited( int );

	public:
		static CommunicationManager* instance_;

	signals:
		void Ready();
		void Error(QString &reason);
	};


} // namespace TpQt4Communication


#endif //incl_Comm_CommunicationManager_h
