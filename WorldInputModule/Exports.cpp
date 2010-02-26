// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "WorldInputModule.h"
#include "Profiler.h"

using namespace Input;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(WorldInputModule)
POCO_END_MANIFEST

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}
