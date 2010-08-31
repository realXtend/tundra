// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ViewerLogicModule.h"

#include "MemoryLeakCheck.h"

std::string ViewerLogicModule::type_name_static_ = "ViewerLogic";

ViewerLogicModule::ViewerLogicModule() : ModuleInterface(type_name_static_)
{
}

ViewerLogicModule::~ViewerLogicModule()
{
}

void ViewerLogicModule::PreInitialize()
{
}

void ViewerLogicModule::Initialize()
{
}

void ViewerLogicModule::PostInitialize()
{
}

void ViewerLogicModule::Uninitialize()
{
}

void ViewerLogicModule::Update(f64 frametime)
{
    RESETPROFILER;
}

// virtual
bool ViewerLogicModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(ViewerLogicModule)
POCO_END_MANIFEST
