#include "StableHeaders.h"
#include "QtScriptModule.h"
#include <QtScript>

namespace RexQtScript
{
	RexQtScriptModule::RexQtScriptModule() : ModuleInterfaceImpl(type_static_)
    {
    }

    RexQtScriptModule::~RexQtScriptModule()
    {
    }

	void RexQtScriptModule::Load()
	{
        LogInfo("Module " + Name() + " loaded.");
	}

    void RexQtScriptModule::Unload()
    {
        LogInfo("Module " + Name() + " unloaded.");
    }

    void RexQtScriptModule::Initialize()
    {
		QScriptValue res = engine.evaluate("1 + 1;");
		LogInfo("Javascript thinks 1 + 1 = " + res.toString().toStdString());
	}

    void RexQtScriptModule::Update(Core::f64 frametime)
    {
	}

	void RexQtScriptModule::Uninitialize()
    {
	}

    void RexQtScriptModule::PostInitialize()
    {
	}
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace RexQtScript;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
   POCO_EXPORT_CLASS(RexQtScriptModule)
POCO_END_MANIFEST

    