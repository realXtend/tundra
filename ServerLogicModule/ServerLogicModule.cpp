// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ServerLogicModule.h"

#include "MemoryLeakCheck.h"

std::string ServerLogicModule::type_name_static_ = "ServerLogic";

ServerLogicModule::ServerLogicModule() : ModuleInterface(type_name_static_)
{
}

ServerLogicModule::~ServerLogicModule()
{
}

void ServerLogicModule::PreInitialize()
{
}

void ServerLogicModule::Initialize()
{
}

void ServerLogicModule::PostInitialize()
{
}

void ServerLogicModule::Uninitialize()
{
}

void ServerLogicModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool ServerLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(ServerLogicModule)
POCO_END_MANIFEST
