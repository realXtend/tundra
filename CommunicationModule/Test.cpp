// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CoreException.h"
#include "Test.h"
#include "Credentials.h"
#include "CommunicationService.h"
#include "ConsoleCommandServiceInterface.h"
#include "ServiceManager.h"
#include "ContactInterface.h"
#include "ChatSessionInterface.h"
#include "VoiceSessionInterface.h"
#include "ChatSessionParticipantInterface.h"
#include "ChatMessageInterface.h"

#include "MemoryLeakCheck.h"

namespace CommunicationTest
{
    Test::Test(Foundation::Framework* framework): framework_(framework), jabber_connection_(0), opensim_connection_(0)
    {
        boost::shared_ptr<Console::CommandService> console_service = framework_->GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
        if (console_service)
        {
            console_service->RegisterCommand(Console::CreateCommand("comm test", "Run a test for communication service", Console::Bind(this, &Test::OnConsoleCommand)));
        }
    }

    Console::CommandResult Test::OnConsoleCommand(const StringVector &params)
    {
        if (params.size() != 1)
        {
            ShowHelp();
            return Console::ResultSuccess("");
        }

        QString test_id = QString(params[0].c_str());
        switch( test_id.toInt() )
        {
            case 1: RunTest1();break;
            case 2: RunTest2();break;
            case 3: RunTest3();break;
            case 4: RunTest4();break;
            case 5: RunTest5();break;
            case 6: RunTest6();break;
            case 7: RunTest7();break;
            case 0: RunTest0();break;
        }
        return Console::ResultSuccess("");
    }

    void Test::ShowHelp()
    {
        LogInfo("Test functions for Communication module:");
        LogInfo("1 .. Login to jabber server");
        LogInfo("2 .. Send a text message to Opensim in-world chat.");
        LogInfo("3 .. Send a text message to all jabber contacts");
        LogInfo("4 .. Start voice chat with on contact");
        LogInfo("5 .. Send a friend request to jabber account");
        LogInfo("6 .. Send a text message to jabber chat room");
        LogInfo("7 .. Fetch friend list and show online statuses");
        LogInfo("0 .. Close jabber connection.");
        if (jabber_connection_ == 0)
            LogInfo("* Jabber connection is closed.");
        else
            LogInfo("* Jabber connection is open");
        if (opensim_connection_ == 0)
            LogInfo("* Opensim connection is closed.");
        else
            LogInfo("* Opensim connection is open");
    }

    void Test::RunTest1()
    {
        Communication::Credentials jabber_credentials;
        jabber_credentials.SetProtocol("jabber");
        jabber_credentials.SetUserID("jonne.nauha@gmail.com");
        jabber_credentials.SetServer("talk.google.com");
        jabber_credentials.SetPort(5222);
        jabber_credentials.SetPassword("");

        OpenConnection(jabber_credentials);
    }

    void Test::RunTest2()
    {
        Communication::Credentials opensim_credentials;
        opensim_credentials.SetProtocol("opensim_udp");

        try
        {
            Communication::CommunicationServiceInterface* communication_service = Communication::CommunicationService::GetInstance();
            opensim_connection_ = communication_service->OpenConnection(opensim_credentials);
            switch (opensim_connection_->GetState())
            {
            case Communication::ConnectionInterface::STATE_INITIALIZING:
                connect(opensim_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionReady JabberConnectionReady(Communication::ConnectionInterface&) ));
                connect(opensim_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionReady(Communication::ConnectionInterface&) ));
                break;
            case Communication::ConnectionInterface::STATE_OPEN:
                OnOpensimUdpConnectionReady(*opensim_connection_);
                break;
            case Communication::ConnectionInterface::STATE_ERROR:
                OnOpensimUdpConnectionError(*opensim_connection_);
                break;
            }

        }
        catch(Exception &e)
        {
            QString message = QString("Cannot create opensim IM connction object: ").append(e.what());
            LogError(message.toStdString());
        }
    }

    void Test::RunTest3()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }

        try
        {
            Communication::ContactVector contacts = jabber_connection_->GetContacts().GetContacts();
            for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
            {
                Communication::ChatSessionInterface* chat = jabber_connection_->OpenPrivateChatSession(**i);
                chat->SendChatMessage("Hello world");
            }
        }
        catch(Exception &e)
        {
            LogError(e.what());
        }
    }

    void Test::RunTest4()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }

        try
        {
            Communication::ContactVector contacts = jabber_connection_->GetContacts().GetContacts();
            if (contacts.size() == 0)
            {
                LogError("The jabber contact list is empty!");
                return;
            }
            Communication::VoiceSessionInterface* voice = jabber_connection_->OpenVoiceSession(*(contacts[0]));
            voice->Close();
        }
        catch(Exception &e)
        {
            LogError(e.what());
        }
    }

    void Test::RunTest5()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }
        jabber_connection_->SendFriendRequest("rex_user_1@jabber.org", "Test 5");
    }

    void Test::RunTest6()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }
        try
        {
            Communication::ChatSessionInterface* chat = jabber_connection_->OpenChatSession("my_test_123@conference.jabber.org");
            chat->SendChatMessage("Hello world!");
        }
        catch(Exception &e)
        {
            LogError(e.what());
        }
    }

    void Test::RunTest7()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }
        try
        {
            switch (jabber_connection_->GetState())
            {
            case Communication::ConnectionInterface::STATE_INITIALIZING:
                connect(jabber_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( ShowContacts(Communication::ConnectionInterface&) ));
                connect(jabber_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( ShowContacts(Communication::ConnectionInterface&) ));
                break;
            case Communication::ConnectionInterface::STATE_OPEN:
                ShowContacts(*jabber_connection_);
                break;
            case Communication::ConnectionInterface::STATE_ERROR:
                OnJabberConnectionError(*jabber_connection_);
                break;
            }
        }
        catch(Exception &e)
        {
            LogError(e.what());
        }
    }

    void Test::RunTest0()
    {
        if (jabber_connection_ == 0)
        {
            LogError("Jabber connection is not open!");
            return;
        }
        try
        {
            jabber_connection_->Close();
        }
        catch(Exception &e)
        {
            LogError(e.what());
        }
    }

    void Test::OpenConnection(Communication::CredentialsInterface& credentials)
    {
        try
        {
            Communication::CommunicationServiceInterface* communication_service = Communication::CommunicationService::GetInstance();
            jabber_connection_ = communication_service->OpenConnection(credentials);

            switch (jabber_connection_->GetState())
            {
            case Communication::ConnectionInterface::STATE_INITIALIZING:
                connect(jabber_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnJabberConnectionReady(Communication::ConnectionInterface&) ));
                connect(jabber_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnJabberConnectionReady(Communication::ConnectionInterface&) ));
                break;
            case Communication::ConnectionInterface::STATE_OPEN:
                OnJabberConnectionReady(*jabber_connection_);
                break;
            case Communication::ConnectionInterface::STATE_ERROR:
                OnJabberConnectionError(*jabber_connection_);
                break;
            }
        }
        catch(Exception &e)
        {
            QString message = QString("Cannot create jabber connction object: ").append(e.what());
            LogError(message.toStdString());
        }
    }

    void Test::ShowContacts(Communication::ConnectionInterface& connection)
    {
        try
        {
            Communication::ContactGroupInterface& friend_list = jabber_connection_->GetContacts();
            Communication::ContactVector contacts = friend_list.GetContacts();
            for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
            {
                Communication::ContactInterface* contact = *i;
                QString name = contact->GetName();
                QString id = contact->GetID();
                QString status = contact->GetPresenceStatus();
                QString message = QString("Friend: ").append(name).append(" (").append(id).append(") ").append(status);
                LogInfo(message.toStdString());
            }
        }
        catch(Exception &e)
        {
            QString message = QString("Error: ").append(e.what());
            LogDebug(message.toStdString());
        }
    }

    void Test::OnJabberConnectionReady(Communication::ConnectionInterface& connection)
    {
        try
        {
            connect(&connection, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface&) ), SLOT(OnChatSessionReceived( Communication::ChatSessionInterface&) ));
            connect(&connection, SIGNAL( VoiceSessionReceived(Communication::VoiceSessionInterface&) ), SLOT(OnVoiceSessionReceived( Communication::VoiceSessionInterface&) ));
        }
        catch(Exception &e)
        {
            QString message = QString("Test for TelepathyIM failed: ").append(e.what());
            LogDebug(message.toStdString());
        }
    }
    
    void Test::OnChatSessionReceived( Communication::ChatSessionInterface& chat)
    {
        ;
        QString message = QString("Chat session from: ").append( chat.GetParticipants()[0]->GetID() );
        LogInfo(message.toStdString());

        Communication::ChatMessageVector history = chat.GetMessageHistory();
        for (Communication::ChatMessageVector::iterator i = history.begin(); i != history.end(); ++i)
        {
            LogInfo( (*i)->GetText().toStdString() );
        }
        chat.SendChatMessage("Hello world");
    }

    void Test::OnVoiceSessionReceived( Communication::VoiceSessionInterface& session)
    {
        QString message = QString("Voice session request reseived from: ???");
        LogInfo(message.toStdString());
        session.Reject();
//        session.Accept();
    }

    void Test::OnJabberConnectionError(Communication::ConnectionInterface& connection)
    {
        QString message = QString("Test failed: ").append(connection.GetReason());
        LogError(message.toStdString());
    }

    void Test::OnOpensimUdpConnectionReady(Communication::ConnectionInterface& connection)
    {
        try
        {
            Communication::ChatSessionInterface* public_chat = connection.OpenChatSession("0"); // public chat channel 0
            public_chat->SendChatMessage("Hello world");
            public_chat->Close();
        }
        catch (Exception /*&e*/)
        {
            QString message = QString("Test failed (Cannot send a message): ").append(connection.GetReason());
            LogError(message.toStdString());
        }
    }

    void Test::OnOpensimUdpConnectionError(Communication::ConnectionInterface& connection)
    {
        QString message = QString("Test failed (Cannot create connection): ").append(connection.GetReason());
        LogError(message.toStdString());
    }


} // end of namespace: CommunicationTest
