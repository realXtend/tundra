// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraLogicModule.h"
#include "ConsoleCommandServiceInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "KristalliProtocolModuleEvents.h"
#include "CoreStringUtils.h"
#include "TundraMessages.h"
#include "TundraEvents.h"
#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

namespace TundraLogic
{

std::string TundraLogicModule::type_name_static_ = "TundraLogic";

static const unsigned short cDefaultPort = 2345;

std::string BufferToString(const std::vector<s8>& buffer)
{
    if (buffer.size())
        return std::string((const char*)&buffer[0], buffer.size());
    else
        return std::string();
}

std::vector<s8> StringToBuffer(const std::string& str)
{
    std::vector<s8> ret;
    ret.resize(str.size());
    if (str.size())
        memcpy(&ret[0], &str[0], str.size());
    return ret;
}

TundraLogicModule::TundraLogicModule() : ModuleInterface(type_name_static_),
    loginstate_(NotConnected),
    reconnect_(false)
{
}

TundraLogicModule::~TundraLogicModule()
{
}

void TundraLogicModule::PreInitialize()
{
}

void TundraLogicModule::Initialize()
{
    tundraEventCategory_ = framework_->GetEventManager()->RegisterEventCategory("Tundra");
}

void TundraLogicModule::PostInitialize()
{
    kristalliEventCategory_ = framework_->GetEventManager()->QueryEventCategory("Kristalli");

    RegisterConsoleCommand(Console::CreateCommand("startserver", 
        "Starts a server. Usage: \"startserver(port,tcp|udp)\"",
        Console::Bind(this, &TundraLogicModule::ConsoleStartServer)));
    RegisterConsoleCommand(Console::CreateCommand("stopserver", 
        "Stops the server",
        Console::Bind(this, &TundraLogicModule::ConsoleStopServer)));
    RegisterConsoleCommand(Console::CreateCommand("connect", 
        "Connects to a server. Usage: \"connect( address,[port],[tcp|udp],[username],[password])\"",
        Console::Bind(this, &TundraLogicModule::ConsoleConnect)));
    RegisterConsoleCommand(Console::CreateCommand("disconnect", 
        "Disconnects from a server.",
        Console::Bind(this, &TundraLogicModule::ConsoleDisconnect)));
    
    // Take a pointer to KristalliProtocolModule so that we don't have to take/check it every time
    kristalliModule_ = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock();
    if (!kristalliModule_.get())
        LogFatal("Could not get KristalliProtocolModule");
}

void TundraLogicModule::Uninitialize()
{
    kristalliModule_.reset();
}

void TundraLogicModule::Update(f64 frametime)
{
    // Handle pending login if we are a client
    HandleLogin();
    
    RESETPROFILER;
}

void TundraLogicModule::Login(const std::string& address, unsigned short port, bool use_udp, const std::string& username, const std::string& password)
{
    if (kristalliModule_->IsServer())
    {
        LogError("Already running a server, cannot login to a world as a client");
        return;
    }
    
    username_ = username;
    password_ = password;
    reconnect_ = false;
    
    kristalliModule_->Connect(address.c_str(), port, use_udp ? SocketOverUDP : SocketOverTCP);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
}

void TundraLogicModule::Logout(bool fail)
{
    if (loginstate_ != NotConnected)
    {
        if (GetClientConnection())
        {
            kristalliModule_->Disconnect();
            LogInfo("Disconnected");
        }
        
        loginstate_ = NotConnected;
        client_id_ = 0;
        
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_DISCONNECTED, 0);
        framework_->RemoveScene("TundraClient");
    }
    
    if (fail)
        framework_->GetEventManager()->SendEvent(tundraEventCategory_, Events::EVENT_TUNDRA_LOGIN_FAILED, 0);
}

void TundraLogicModule::HandleLogin()
{
    MessageConnection* connection = GetClientConnection();
    
    switch (loginstate_)
    {
    case ConnectionPending:
        if ((connection) && (connection->GetConnectionState() == ConnectionOK))
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
        if ((!connection) || (connection->GetConnectionState() != ConnectionOK))
        {
            loginstate_ = ConnectionPending;
        }
        break;
    }
}

void TundraLogicModule::StartServer(unsigned short port, bool use_udp)
{
    if (!kristalliModule_->IsServer())
    {
        if (!kristalliModule_->StartServer(port, use_udp ? SocketOverUDP : SocketOverTCP))
            return;
        Scene::ScenePtr scene = framework_->CreateScene("TundraServer");
        framework_->SetDefaultWorldScene(scene);
    }
}

void TundraLogicModule::StopServer()
{
    if (kristalliModule_->IsServer())
    {
        kristalliModule_->StopServer();
        framework_->RemoveScene("TundraServer");
    }
}

MessageConnection* TundraLogicModule::GetClientConnection()
{
    return kristalliModule_->GetMessageConnection();
}

Console::CommandResult TundraLogicModule::ConsoleStartServer(const StringVector& params)
{
    unsigned short port = cDefaultPort;
    bool use_udp = true;
    
    try
    {
        if (params.size() > 0)
            port = ParseString<int>(params[0]);
        if (params.size() > 1)
            if (params[1] == "tcp")
                use_udp = false;
    }
    catch (...) {}
    
    StartServer(port, use_udp);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleStopServer(const StringVector& params)
{
    StopServer();
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleConnect(const StringVector& params)
{
    if (params.size() < 1)
        return Console::ResultFailure("No address specified");
    
    unsigned short port = cDefaultPort;
    bool use_udp = true;
    std::string username = "test";
    std::string password = "test";
    
    try
    {
        if (params.size() > 1)
            port = ParseString<int>(params[1]);
        if (params.size() > 2)
            if (params[2] == "tcp")
                use_udp = false;
        if (params.size() > 3)
            username = params[3];
        if (params.size() > 4)
            password = params[4];
    }
    catch (...) {}
    
    Login(params[0], port, use_udp, username, password);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleDisconnect(const StringVector& params)
{
    Logout(false);
    
    return Console::ResultSuccess();
}

// virtual
bool TundraLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (category_id == tundraEventCategory_)
    {
        if (event_id == Events::EVENT_TUNDRA_LOGIN)
        {
            // Note: here UDP is chosen in a hardcoded way
            bool use_udp = true;
            Events::TundraLoginEventData* event_data = checked_static_cast<Events::TundraLoginEventData*>(data);
            Login(event_data->address_, event_data->port_ ? event_data->port_ : cDefaultPort, use_udp, event_data->username_, event_data->password_);
        }
    }
    
    if (category_id == kristalliEventCategory_)
    {
        if (event_id == KristalliProtocol::Events::NETMESSAGE_IN)
        {
            KristalliProtocol::Events::KristalliNetMessageIn* eventData = checked_static_cast<KristalliProtocol::Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
        }
        if (event_id == KristalliProtocol::Events::USER_DISCONNECTED)
        {
            KristalliProtocol::Events::KristalliUserDisconnected* eventData = checked_static_cast<KristalliProtocol::Events::KristalliUserDisconnected*>(data);
            KristalliProtocol::UserConnection* user = eventData->connection;
            if (user)
                ServerHandleUserDisconnected(user);
        }
        if (event_id == KristalliProtocol::Events::CONNECTION_FAILED)
        {
            Logout(true);
        }
    }
    
    return false;
}

void TundraLogicModule::HandleKristalliMessage(MessageConnection* source, message_id_t id, const char* data, size_t numBytes)
{
    // If we are client, verify that the message comes from the server we're connected to
    if (!kristalliModule_->IsServer())
    {
        if (source != GetClientConnection())
        {
            LogWarning("Client: dropping message " + ToString(id) + " from unknown source");
            return;
        }
    }
    else
    {
        // If we are server, only allow the login message from an unauthenticated user
        if (id != cLoginMessage)
        {
            KristalliProtocol::UserConnection* user = kristalliModule_->GetUserConnection(source);
            if ((!user) || (!user->authenticated))
            {
                LogWarning("Server: dropping message " + ToString(id) + " from unauthenticated user");
                //! \todo something more severe, like disconnecting the user
                return;
            }
        }
    }
    
    switch (id)
    {
        // Server
    case cLoginMessage:
        {
            MsgLogin msg(data, numBytes);
            ServerHandleLogin(source, msg);
        }
        break;
        
        // Client
    case cLoginReplyMessage:
        {
            MsgLoginReply msg(data, numBytes);
            ClientHandleLoginReply(source, msg);
        }
        break;
    case cClientJoinedMessage:
        {
            MsgClientJoined msg(data, numBytes);
            ClientHandleClientJoined(source, msg);
        }
        break;
    case cClientLeftMessage:
        {
            MsgClientLeft msg(data, numBytes);
            ClientHandleClientLeft(source, msg);
        }
        break;
    }
}

void TundraLogicModule::ServerHandleLogin(MessageConnection* source, const MsgLogin& msg)
{
    // For now, automatically accept the connection if it's from a known user
    KristalliProtocol::UserConnection* user = kristalliModule_->GetUserConnection(source);
    if (!user)
    {
        LogWarning("Login message from unknown user");
        return;
    }
    
    user->userName = BufferToString(msg.userName);
    user->authenticated = true;
    LogInfo("User " + user->userName + " logging in");
    
    MsgLoginReply reply;
    reply.success = 1;
    reply.userID = user->userID;
    user->connection->Send(reply);
    
    // Tell everyone of the client joining (also the user who joined)
    KristalliProtocol::UserConnectionList users = kristalliModule_->GetAuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    joined.userName = msg.userName;
    for (KristalliProtocol::UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        iter->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    for (KristalliProtocol::UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if (iter->userID != user->userID)
        {
            MsgClientJoined joined;
            joined.userID = iter->userID;
            joined.userName = StringToBuffer(iter->userName);
            user->connection->Send(joined);
        }
    }
}

void TundraLogicModule::ServerHandleUserDisconnected(KristalliProtocol::UserConnection* user)
{
    // Tell everyone of the client leaving
    MsgClientLeft left;
    left.userID = user->userID;
    left.userName = StringToBuffer(user->userName);
    KristalliProtocol::UserConnectionList users = kristalliModule_->GetAuthenticatedUsers();
    for (KristalliProtocol::UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
    {
        if (iter->userID != user->userID)
            iter->connection->Send(left);
    }
}

void TundraLogicModule::ClientHandleLoginReply(MessageConnection* source, const MsgLoginReply& msg)
{
    if (msg.success)
    {
        loginstate_ = LoggedIn;
        client_id_ = msg.userID;
        LogInfo("Logged in successfully");
        
        // Note: create scene & send info of login success only on first connection, not on reconnect
        if (!reconnect_)
        {
            Scene::ScenePtr scene = framework_->CreateScene("TundraClient");
            framework_->SetDefaultWorldScene(scene);
            
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

void TundraLogicModule::ClientHandleClientJoined(MessageConnection* source, const MsgClientJoined& msg)
{
    LogInfo("User " + BufferToString(msg.userName) + " is inworld");
}

void TundraLogicModule::ClientHandleClientLeft(MessageConnection* source, const MsgClientLeft& msg)
{
     LogInfo("User " + BufferToString(msg.userName) + " left the world");
}

}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace TundraLogic;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TundraLogicModule)
POCO_END_MANIFEST
