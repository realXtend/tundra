// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "LegacyAvatarModule.h"
#include <Poco/ClassLibrary.h>

#include <iomanip>
#include <limits>
#include <boost/math/special_functions/fpclassify.hpp>

namespace LegacyAvatar
{
    LegacyAvatarModule::LegacyAvatarModule() 
    : ModuleInterfaceImpl("LegacyAvatarModule")
    {
    }
    
    // virtual
    LegacyAvatarModule::~LegacyAvatarModule()
    {

    }

    void LegacyAvatarModule::Load()
    {
        LogInfo("Module " + Name() + " loaded.");
    }

    void LegacyAvatarModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void LegacyAvatarModule::Initialize()
    {        
        LogInfo("Module " + Name() + " initialized.");
    }

    void LegacyAvatarModule::PostInitialize()
    {
    }

    void LegacyAvatarModule::Uninitialize()
    {
        LogInfo("Module " + Name() + " uninitialized.");
    }

    void LegacyAvatarModule::Update(f64 frametime)
    {
        RESETPROFILER;
    }

    bool LegacyAvatarModule::HandleEvent(
        event_category_id_t category_id,
        event_id_t event_id, 
        Foundation::EventDataInterface* data)
    {
        PROFILE(LegacyAvatarModule_HandleEvent);

        return false;
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace LegacyAvatar;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(LegacyAvatarModule)
POCO_END_MANIFEST
