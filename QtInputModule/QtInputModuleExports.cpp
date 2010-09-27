// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QtInputModule.h"
#include "Profiler.h"

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(QtInputModule)
POCO_END_MANIFEST

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}
