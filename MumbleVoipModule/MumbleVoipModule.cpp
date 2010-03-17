// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MumbleVoipModule.h"
#include "MemoryLeakCheck.h"
#include "LinkPlugin.h"

namespace MumbleVoip
{
	std::string MumbleVoipModule::module_name_ = "MumbleVoipModule";

	MumbleVoipModule::MumbleVoipModule()
        : ModuleInterfaceImpl(module_name_), link_plugin_(new LinkPlugin())
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(link_plugin_);
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
        // Testing...
        link_plugin_->SetAvatarName("Test User");
        link_plugin_->SetAvatarIdentity("test_user_1231412389483478");
        link_plugin_->SetGroupId("naali-test-users");
        link_plugin_->SetDescription("Naali viewer");
    }

    void MumbleVoipModule::Uninitialize()
    {
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
        link_plugin_->SendData();
    }

	bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
	{
		return false;
	}

	void MumbleVoipModule::InitializeLinkPlugin()
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
