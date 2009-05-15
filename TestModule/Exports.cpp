// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TestModule.h"
#include "TestModuleB.h"

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Framework *framework);
void SetProfiler(Foundation::Framework *framework)
{
    Foundation::ProfilerSection::SetProfiler(&framework->GetProfiler());
}

using namespace Test;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(TestModule)
   POCO_EXPORT_CLASS(TestModuleB)
POCO_END_MANIFEST