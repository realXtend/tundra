/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ScriptModule.h"

#include "MemoryLeakCheck.h"

std::string ScriptModule::type_name_static_ = "Script";

ScriptModule::ScriptModule() :
    ModuleInterface(type_name_static_)
{
}

ScriptModule::~ScriptModule()
{
}

void ScriptModule::PreInitialize()
{
}

void ScriptModule::Initialize()
{
}

void ScriptModule::PostInitialize()
{
}

void ScriptModule::Uninitialize()
{
}

void ScriptModule::Update(f64 frametime)
{
    RESETPROFILER;
}

bool ScriptModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
{
    return false;
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(ScriptModule)
POCO_END_MANIFEST
