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
		//! @todo IMPLEMENT
	}

	void Test::OpenConnection(Communication::CredentialsInterface& crederntials)
	{
		try
		{
			Communication::CommunicationServiceInterface* communication_service = Communication::CommunicationService::GetInstance();
			jabber_connection_ = communication_service->OpenConnection(crederntials);
			connect(jabber_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), SLOT( OnConnectionReady(Communication::ConnectionInterface&) ));
			connect(jabber_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), SLOT( OnConnectionError(Communication::ConnectionInterface&) ));
			// The test continues on OnConnectionReady function
		}
		catch(Core::Exception &e)
		{
			QString message = QString("Cannot create connction object: ").append(e.what());
			LogError(message.toStdString());
		}
	}

	void Test::OnConnectionReady(Communication::ConnectionInterface& connection)
	{
		try
		{
			Communication::ContactGroupInterface& friend_list = jabber_connection_->GetContacts();
			Communication::ContactVector contacts = friend_list.GetContacts();
			for (Communication::ContactVector::iterator i = contacts.begin(); i != contacts.end(); ++i)
			{
				QString name = (*i)->GetName();
				QString message = QString("Friend: ").append(name);
				LogInfo(message.toStdString());
			}
			if (contacts.size() > 0)
			{
				Communication::ChatSessionInterface* chat = jabber_connection_->OpenPrivateChatSession(*(contacts[0]));
				chat->SendMessage("Hello world!");
				chat->Close();
			}
			jabber_connection_->Close();
		}
		catch(Core::Exception &e)
		{
			QString message = QString("Test for TelepathyIM failed: ").append(e.what());
			LogDebug(message.toStdString());
		}
	}

	void Test::OnConnectionError(Communication::ConnectionInterface& connection)
	{
		QString message = QString("Test failed: ").append(connection.GetReason());
		LogError(message.toStdString());
	}

} // end of namespace: CommunicationTest
