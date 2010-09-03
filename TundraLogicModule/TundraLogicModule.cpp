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
#include "MsgLogin.h"
#include "MsgLoginReply.h"
#include "MsgClientJoined.h"
#include "MsgClientLeft.h"

#include "MemoryLeakCheck.h"

using namespace KristalliProtocol;

std::string TundraLogicModule::type_name_static_ = "TundraLogic";

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
    loginstate_(NotConnected)
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
    kristalliModule_ = framework_->GetModuleManager()->GetModule<KristalliProtocolModule>().lock();
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
    username_ = username;
    password_ = password;
    
    kristalliModule_->Connect(address.c_str(), port, use_udp ? SocketOverUDP : SocketOverTCP);
    loginstate_ = ConnectionPending;
    client_id_ = 0;
}

void TundraLogicModule::Logout()
{
    if (loginstate_ != NotConnected)
    {
        LogInfo("Disconnected");
        kristalliModule_->Disconnect();
        loginstate_ = NotConnected;
        client_id_ = 0;
    }
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

MessageConnection* TundraLogicModule::GetClientConnection()
{
    return kristalliModule_->GetMessageConnection();
}

Console::CommandResult TundraLogicModule::ConsoleStartServer(const StringVector& params)
{
    unsigned short port = 2345;
    SocketTransportLayer transport = SocketOverUDP;
    
    try
    {
        if (params.size() > 0)
            port = ParseString<int>(params[0]);
        if (params.size() > 1)
            if (params[1] == "tcp")
                transport = SocketOverTCP;
    }
    catch (...) {}
    
    kristalliModule_->StartServer(port, transport);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleStopServer(const StringVector& params)
{
    kristalliModule_->StopServer();
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleConnect(const StringVector& params)
{
    if (params.size() < 1)
        return Console::ResultFailure("No address specified");
    
    unsigned short port = 2345;
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
    Logout();
    
    return Console::ResultSuccess();
}

// virtual
bool TundraLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (category_id == kristalliEventCategory_)
    {
        if (event_id == Events::NETMESSAGE_IN)
        {
            Events::KristalliNetMessageIn* eventData = checked_static_cast<Events::KristalliNetMessageIn*>(data);
            HandleKristalliMessage(eventData->source, eventData->id, eventData->data, eventData->numBytes);
        }
        if (event_id == Events::USER_DISCONNECTED)
        {
            Events::KristalliUserDisconnected* eventData = checked_static_cast<Events::KristalliUserDisconnected*>(data);
            UserConnection* user = eventData->connection;
            if (user)
                ServerHandleUserDisconnected(user);
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
            UserConnection* user = kristalliModule_->GetUserConnection(source);
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
    UserConnection* user = kristalliModule_->GetUserConnection(source);
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
    UserConnectionList users = kristalliModule_->GetAuthenticatedUsers();
    MsgClientJoined joined;
    joined.userID = user->userID;
    joined.userName = msg.userName;
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
        iter->connection->Send(joined);
    
    // Advertise the users who already are in the world, to the new user
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
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

void TundraLogicModule::ServerHandleUserDisconnected(UserConnection* user)
{
    // Tell everyone of the client leaving
    MsgClientLeft left;
    left.userID = user->userID;
    left.userName = StringToBuffer(user->userName);
    UserConnectionList users = kristalliModule_->GetAuthenticatedUsers();
    for (UserConnectionList::const_iterator iter = users.begin(); iter != users.end(); ++iter)
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

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TundraLogicModule)
POCO_END_MANIFEST
