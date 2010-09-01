// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraServerModule.h"

#include "MemoryLeakCheck.h"

std::string TundraServerModule::type_name_static_ = "TundraServer";

TundraServerModule::TundraServerModule() : ModuleInterface(type_name_static_)
{
}

TundraServerModule::~TundraServerModule()
{
}

void TundraServerModule::PreInitialize()
{
}

void TundraServerModule::Initialize()
{
}

void TundraServerModule::PostInitialize()
{
}

void TundraServerModule::Uninitialize()
{
}

void TundraServerModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool TundraServerModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TundraServerModule)
POCO_END_MANIFEST
