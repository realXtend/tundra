// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MumbleVoipModule.h"
#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
	std::string MumbleVoipModule::module_name_ = "MumbleVoipModule";

	MumbleVoipModule::MumbleVoipModule()
        : ModuleInterfaceImpl(module_name_)
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
    }

    void MumbleVoipModule::Load()
    {
    }

    void MumbleVoipModule::Unload()
    {
    }

    void MumbleVoipModule::Initialize() 
    {
		InitializeLinkPlugin();
    }

    void MumbleVoipModule::PostInitialize()
    {
    }

    void MumbleVoipModule::Uninitialize()
    {
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
		UpdateLinkPlugin();
    }

	bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
	{
		return false;
	}

	void MumbleVoipModule::InitializeLinkPlugin()
	{
		//! todo: IMPLEMENT
	}

	void MumbleVoipModule::UpdateLinkPlugin()
	{
		//! todo: IMPLEMENT
	}

} // end of namespace: MumbleVoip

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace MumbleVoip;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(MumbleVoipModule)
POCO_END_MANIFEST
