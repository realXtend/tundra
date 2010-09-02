// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraLogicModule.h"
#include "ConsoleCommandServiceInterface.h"
#include "ModuleManager.h"
#include "KristalliProtocolModule.h"
#include "CoreStringUtils.h"

#include "MemoryLeakCheck.h"

std::string TundraLogicModule::type_name_static_ = "TundraLogic";

TundraLogicModule::TundraLogicModule() : ModuleInterface(type_name_static_)
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
    RegisterConsoleCommand(Console::CreateCommand("startserver", 
        "Starts a server. Usage: \"startserver(port,tcp|udp)\"",
        Console::Bind(this, &TundraLogicModule::ConsoleStartServer)));
    RegisterConsoleCommand(Console::CreateCommand("stopserver", 
        "Stops the server",
        Console::Bind(this, &TundraLogicModule::ConsoleStopServer)));
    RegisterConsoleCommand(Console::CreateCommand("connect", 
        "Connects to a server. Usage: \"connect(address,port,tcp|udp)\"",
        Console::Bind(this, &TundraLogicModule::ConsoleConnect)));
    RegisterConsoleCommand(Console::CreateCommand("disconnect", 
        "Disconnects from a server.",
        Console::Bind(this, &TundraLogicModule::ConsoleDisconnect)));
}

void TundraLogicModule::Uninitialize()
{
}

void TundraLogicModule::Update(f64 frametime)
{
    RESETPROFILER;
}

Console::CommandResult TundraLogicModule::ConsoleStartServer(const StringVector& params)
{
    KristalliProtocol::KristalliProtocolModule *module = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock().get();
    if(!module)
        return Console::ResultFailure("No Kristalli module");
    
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
    
    module->StartServer(port, transport);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleStopServer(const StringVector& params)
{
    KristalliProtocol::KristalliProtocolModule *module = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock().get();
    if(!module)
        return Console::ResultFailure("No Kristalli module");
    
    module->StopServer();
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleConnect(const StringVector& params)
{
    KristalliProtocol::KristalliProtocolModule *module = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock().get();
    if(!module)
        return Console::ResultFailure("No Kristalli module");

    if (params.size() < 1)
        return Console::ResultFailure("No address specified");
    
    unsigned short port = 2345;
    SocketTransportLayer transport = SocketOverUDP;
    
    try
    {
        if (params.size() > 1)
            port = ParseString<int>(params[1]);
        if (params.size() > 2)
            if (params[2] == "tcp")
                transport = SocketOverTCP;
    }
    catch (...) {}
    
    module->Connect(params[0].c_str(), port, transport);
    
    return Console::ResultSuccess();
}

Console::CommandResult TundraLogicModule::ConsoleDisconnect(const StringVector& params)
{
    KristalliProtocol::KristalliProtocolModule *module = framework_->GetModuleManager()->GetModule<KristalliProtocol::KristalliProtocolModule>().lock().get();
    if(!module)
        return Console::ResultFailure("No Kristalli module");
    
    module->Disconnect();
    
    return Console::ResultSuccess();
}

// virtual
bool TundraLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TundraLogicModule)
POCO_END_MANIFEST
