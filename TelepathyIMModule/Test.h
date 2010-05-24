// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_Test_h
#define incl_Communication_Test_h

#include <QObject>
#include <Foundation.h>
//#include "interface.h"
#include "ModuleLoggingFunctions.h"
#include "CredentialsInterface.h"
#include "ConnectionInterface.h"

namespace CommunicationTest
{
    /**
     *  Place for tests for communication framework
     *
     *  Idea is keep tests in one place and offer examples how to use Communication classes.
     *
     *  To see current test, type "comm test" in console.
     */ 
    class Test : public QObject
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "Communication"; } // for logging functionality

    public:
        Test(Foundation::Framework* framework);

        //! Console command handler
        Console::CommandResult OnConsoleCommand(const StringVector &params);

        //! Show help text to console
        virtual void ShowHelp();

        //! Connect to jabber server
        virtual void RunTest1();

        //! Send "Hello world!" to public in-world chat (Opensim)
        virtual void RunTest2();

        //! Send text message to all jabber contacts
        virtual void RunTest3();

        //! Start voice chat with first contact
        virtual void RunTest4();

        //! Send a friend request to jabber account
        virtual void RunTest5();

        //! Send a text message to jabber chat room
        virtual void RunTest6();

        //! Fetch friend list and show online statuses
        virtual void RunTest7();

        //! Closes the jabber connection
        virtual void RunTest0();

    protected:
        virtual void OpenConnection(Communication::CredentialsInterface& crederntials);
        Foundation::Framework* framework_;
        Communication::ConnectionInterface* jabber_connection_;
        Communication::ConnectionInterface* opensim_connection_;

    protected slots:
        void OnJabberConnectionReady(Communication::ConnectionInterface&);
        void OnJabberConnectionError(Communication::ConnectionInterface&);
        void OnOpensimUdpConnectionReady(Communication::ConnectionInterface&);
        void OnOpensimUdpConnectionError(Communication::ConnectionInterface&);
        void OnChatSessionReceived( Communication::ChatSessionInterface& chat);
        void ShowContacts(Communication::ConnectionInterface& connection);
        void OnVoiceSessionReceived( Communication::VoiceSessionInterface& session);
    };

} // end of namespace: CommunicationTest

#endif // incl_Communication_Test_h
