#include "Test.h"
#include "Credentials.h"
#include "CommunicationService.h"

namespace CommunicationTest
{
	Test::Test(): jabber_connection_(0), opensim_connection_(0)
	{
		
	}

	void Test::RunTest1()
	{
		Communication::Credentials jabber_credentials;
		jabber_credentials.SetProtocol("jabber");
		jabber_credentials.SetUserID("realxtend@jabber.org");
		jabber_credentials.SetServer("jabber.org");
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
				connect(opensim_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionReady(Communication::ConnectionInterface&) ));
				connect(opensim_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnOpensimUdpConnectionError(Communication::ConnectionInterface&) ));
				break;
			case Communication::ConnectionInterface::STATE_OPEN:
				OnOpensimUdpConnectionReady(*opensim_connection_);
				break;
			case Communication::ConnectionInterface::STATE_ERROR:
				OnOpensimUdpConnectionError(*opensim_connection_);
				break;
			}
		}
		catch(Core::Exception &e)
		{
			QString message = QString("Cannot create opensim IM connction object: ").append(e.what());
			LogError(message.toStdString());
		}
	}

	void Test::OpenConnection(Communication::CredentialsInterface& credentials)
	{
		try
		{
			Communication::CommunicationServiceInterface* communication_service = Communication::CommunicationService::GetInstance();
			jabber_connection_ = communication_service->OpenConnection(credentials);
			connect(jabber_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnJabberConnectionReady(Communication::ConnectionInterface&) ));
			connect(jabber_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnJabberConnectionError(Communication::ConnectionInterface&) ));
			// The test continues on OnConnectionReady function
		}
		catch(Core::Exception &e)
		{
			QString message = QString("Cannot create jabber connction object: ").append(e.what());
			LogError(message.toStdString());
		}
	}

	void Test::OnJabberConnectionReady(Communication::ConnectionInterface& connection)
	{
		try
		{
			connect(&connection, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface&) ), SLOT(OnChatSessionReceived( Communication::ChatSessionInterface&) ));
			Communication::ContactGroupInterface& friend_list = jabber_connection_->GetContacts();
			Communication::ContactVector contacts = friend_list.GetContacts();
			for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
			{
				QString name = (*i)->GetName();
				QString message = QString("Friend: ").append(name);
				LogInfo(message.toStdString());
				if (name.compare("kuonanoja") != 0)
					continue;

				//Communication::ChatSessionInterface* chat = jabber_connection_->OpenPrivateChatSession(**i);
				//chat->SendMessage("Hello world!");
			}
//			jabber_connection_->Close();
		}
		catch(Core::Exception &e)
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
		chat.SendMessage("Hello world");
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
			public_chat->SendMessage("Hello world");
			public_chat->Close();
		}
		catch (Core::Exception &e)
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
