
#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Client.h"
#include "TundraLogicModule.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "CoreStringUtils.h"
#include "RexNetworkUtils.h"
#include "SyncManager.h"
#include "TundraMessages.h"
#include "TundraEvents.h"

#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

using namespace RexTypes;
using namespace kNet;

namespace TundraLogic
{

Client::Client(TundraLogicModule* owner, Foundation::Framework* fw) :
    owner_(owner),
    framework_(fw),
    loginstate_(NotConnected),
    reconnect_(false)
{
    tundraEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Tundra");
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");
}

Client::~Client()
{
}

void Client::Update(f64 frametime)
{
    // If we aren't a server, check pending login
    if (!owner_->IsServer())
        CheckLogin();
}

void Client::Login(const std::string& address, unsigned short port, const std::string& username, const std::string& password)
{
    if (owner_->IsServer())
    {
        TundraLogicModule::LogError("Already running a server, cannot login to a world as a client");
        return;
    }
    
    username_ = username;
    password_ = password;
    reconnect_ = false;
    
    owner_->GetKristalliModule()->Connect(address.c_str(), port, kNet::SocketOverTCP);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
}

void Client::Logout(bool fail)
{
    if (loginstate_ != NotConnected)
    {
        if (GetConnection())
        {
            owner_->GetKristalliModule()->Disconnect();
            TundraLogicModule::LogInfo("Disconnected");
        }
        
        loginstate_ = NotConnected;
        client_id_ = 0;
        
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_DISCONNECTED, 0);
        framework_->RemoveScene("TundraClient");
    }
    
    if (fail)
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_LOGIN_FAILED, 0);
}

void Client::CheckLogin()
{
    kNet::MessageConnection* connection = GetConnection();
    
    switch (loginstate_)
    {
    case ConnectionPending:
        if ((connection) && (connection->GetConnectionState() == kNet::ConnectionOK))
        {
            loginstate_ = Connected;
            MsgLogin msg;
            msg.userName = StringToBuffer(username_);
            msg.password = StringToBuffer(password_);
            connection->Send(msg);
        }
        break;
    
    case LoggedIn:
        // If we have logged in, but connection dropped, prepare to resend login
        if ((!connection) || (connection->GetConnectionState() != kNet::ConnectionOK))
        {
            loginstate_ = ConnectionPending;
        }
        break;
    }
}

kNet::MessageConnection* Client::GetConnection()
{
    return owner_->GetKristalliModule()->GetMessageConnection();
}

void Client::HandleKristalliEvent(event_id_t event_id, IEventData* data)
{
    if (event_id == KristalliProtocol::Events::NETMESSAGE_IN)
    {
        if (!owner_->IsServer())
        {
            KristalliProtocol::Events::KristalliNetMessageIn* eventData = checked_static_cast<KristalliProtocol::Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
        }
    }
    if (event_id == KristalliProtocol::Events::CONNECTION_FAILED)
    {
        Logout(true);
    }
}

void Client::HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes)
{
    if (source != GetConnection())
    {
        TundraLogicModule::LogWarning("Client: dropping message " + ToString(id) + " from unknown source");
        return;
    }
    
    switch (id)
    {
    case cLoginReplyMessage:
        {
            MsgLoginReply msg(data, numBytes);
            HandleLoginReply(source, msg);
        }
        break;
    case cClientJoinedMessage:
        {
            MsgClientJoined msg(data, numBytes);
            HandleClientJoined(source, msg);
        }
        break;
    case cClientLeftMessage:
        {
            MsgClientLeft msg(data, numBytes);
            HandleClientLeft(source, msg);
        }
        break;
    }
}

void Client::HandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    if (msg.success)
    {
        loginstate_ = LoggedIn;
        client_id_ = msg.userID;
        TundraLogicModule::LogInfo("Logged in successfully");
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnect_)
        {
            Scene::ScenePtr scene = framework_->CreateScene("TundraClient");
            framework_->SetDefaultWorldScene(scene);
            owner_->GetSyncManager()->RegisterToScene(scene);
            
            Events::TundraConnectedEventData event_data;
            event_data.user_id_ = msg.userID;
            framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_CONNECTED, &event_data);
        }
        reconnect_ = true;
    }
    else
    {
        Logout(true);
    }
}

void Client::HandleClientJoined(MessageConnection* source, const MsgClientJoined& msg)
{
    TundraLogicModule::LogInfo("User " + BufferToString(msg.userName) + " is inworld");
}

void Client::HandleClientLeft(MessageConnection* source, const MsgClientLeft& msg)
{
    TundraLogicModule::LogInfo("User " + BufferToString(msg.userName) + " left the world");
}

}

