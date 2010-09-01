// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "TundraClientModule.h"

#include "MemoryLeakCheck.h"

std::string TundraClientModule::type_name_static_ = "TundraClient";

TundraClientModule::TundraClientModule() : ModuleInterface(type_name_static_)
{
}

TundraClientModule::~TundraClientModule()
{
}

void TundraClientModule::PreInitialize()
{
}

void TundraClientModule::Initialize()
{
}

void TundraClientModule::PostInitialize()
{
}

void TundraClientModule::Uninitialize()
{
}

void TundraClientModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool TundraClientModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TundraClientModule)
POCO_END_MANIFEST
