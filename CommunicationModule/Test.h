#ifndef incl_Communication_Test_h
#define incl_Communication_Test_h

#include <QObject>
#include "Foundation.h"
#include "interface.h"
#include "Contact.h"

namespace CommunicationTest
{
	/**
	 *  Place for tests for communication framework
	 *
	 *  Current tests:
	 *  1: Login to jabber server, fetch contact list and send a text message to first contact on list
	 *  2: Send "Hello world!" to public in-world chat (Opensim)
	 *
	 */ 
	class Test : public QObject
	{
		Q_OBJECT
		MODULE_LOGGING_FUNCTIONS
		static const std::string NameStatic() { return "Communication"; } // for logging functionality

	public:
		Test();
		//! Test Telepathy IM
		//! Login to given jabber server
		//! Fetch contact list 
		//! Send a text message to first contact on the list
		virtual void RunTest1();

		//! Send "Hello world!" to public in-world chat (Opensim)
		virtual void RunTest2();
	protected:
		virtual void OpenConnection(Communication::CredentialsInterface& crederntials);

		Communication::ConnectionInterface* jabber_connection_;
		Communication::ConnectionInterface* opensim_connection_;

	protected slots:
		void OnJabberConnectionReady(Communication::ConnectionInterface&);
		void OnJabberConnectionError(Communication::ConnectionInterface&);
		void OnOpensimUdpConnectionReady(Communication::ConnectionInterface&);
		void OnOpensimUdpConnectionError(Communication::ConnectionInterface&);
	};
	

} // end of namespace: CommunicationTest

#endif // incl_Communication_Test_h